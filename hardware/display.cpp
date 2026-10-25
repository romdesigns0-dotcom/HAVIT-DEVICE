#include "havit_display.h"

#include "havit_board.h"
#include "esp_display_panel.hpp"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <cstring>
#include "lvgl.h"
#include "draw/sw/lv_draw_sw_utils.h"

using esp_panel::drivers::LCD;

namespace {
LCD *s_lcd = nullptr;
constexpr char TAG[] = "havit_display";
constexpr uint32_t DMA_LINES = 40;
uint8_t *s_dma_strip = nullptr;
QueueHandle_t s_frame_queue = nullptr;
SemaphoreHandle_t s_frame_done = nullptr;
int64_t s_render_started_us = 0;

struct Frame {
    lv_display_t *display;
    lv_area_t area;
    uint8_t *pixels;
    uint32_t stride;
    uint32_t render_us;
};

void transfer_frames(void *)
{
    Frame frame;
    uint32_t frames = 0, samples = 0, max_render_us = 0, max_transfer_us = 0;
    uint64_t total_render_us = 0, total_transfer_us = 0;
    for(;;) {
        if(xQueueReceive(s_frame_queue, &frame, portMAX_DELAY) != pdTRUE) continue;
        const int64_t started = esp_timer_get_time();
        const int width = frame.area.x2 - frame.area.x1 + 1;
        const int height = frame.area.y2 - frame.area.y1 + 1;
        const size_t stride = width * sizeof(uint16_t);
        /* LVGL has finished composing this update (the entire viewport during
         * scroll). Stream it through DMA-safe memory; no drawing between strips.
         * The bus has a 32768-byte transaction limit, so each strip is <=28800. */
        for(int row = 0; row < height; row += DMA_LINES) {
            const int rows = height - row < (int)DMA_LINES ? height - row : DMA_LINES;
            if(frame.stride == stride) {
                memcpy(s_dma_strip, frame.pixels + row * stride, rows * stride);
            }
            else {
                for(int i = 0; i < rows; ++i)
                    memcpy(s_dma_strip + i * stride,
                           frame.pixels + (row + i) * frame.stride, stride);
            }
            lv_draw_sw_rgb565_swap(s_dma_strip, width * rows);
            if(!s_lcd->drawBitmap(frame.area.x1, frame.area.y1 + row,
                                  width, rows, s_dma_strip, -1)) {
                ESP_LOGE(TAG, "Completed-frame transfer failed at row %d", row);
                break;
            }
        }
        const uint32_t transfer_us = (uint32_t)(esp_timer_get_time() - started);
        total_render_us += frame.render_us;
        total_transfer_us += transfer_us;
        if(frame.render_us > max_render_us) max_render_us = frame.render_us;
        if(transfer_us > max_transfer_us) max_transfer_us = transfer_us;
        ++frames;
        ++samples;
        /* Release only after the entire frame reached the panel. The second
         * PSRAM buffer lets LVGL compose while this task transmits the first. */
        xSemaphoreGive(s_frame_done);
        if(frames == 1 || samples == 30) {
            ESP_LOGI(TAG, "Updates=%lu prepare avg/max=%lu/%lu us, transfer avg/max=%lu/%lu us, region=%dx%d",
                     (unsigned long)frames, (unsigned long)(total_render_us / samples),
                     (unsigned long)max_render_us, (unsigned long)(total_transfer_us / samples),
                     (unsigned long)max_transfer_us, width, height);
            samples = 0;
            total_render_us = total_transfer_us = 0;
            max_render_us = max_transfer_us = 0;
        }
    }
}

void render_start(lv_event_t *)
{
    s_render_started_us = esp_timer_get_time();
}

void wait_for_transfer(lv_display_t *)
{
    /* LVGL is configured without OS synchronization. Its default spinning
     * flush wait is unsafe for completion from the other CPU. Block here and
     * let LVGL clear its flushing flag on its own task after the DMA finishes. */
    configASSERT(xSemaphoreTake(s_frame_done, pdMS_TO_TICKS(2000)) == pdTRUE);
}

void flush_display(lv_display_t *display, const lv_area_t *area, uint8_t *pixels)
{
    const Frame frame = {display, *area, pixels,
        lv_display_get_buf_active(display)->header.stride,
        (uint32_t)(esp_timer_get_time() - s_render_started_us)};
    configASSERT(xQueueSend(s_frame_queue, &frame, portMAX_DELAY) == pdTRUE);
    s_render_started_us = esp_timer_get_time();
}
} // namespace

esp_err_t havit_display_init(void)
{
    const esp_err_t board_result = havit_board_start();
    if(board_result != ESP_OK) return board_result;
    s_lcd = havit_board_lcd();
    if(s_lcd == nullptr) return ESP_ERR_NOT_FOUND;

    lv_init();
    lv_display_t *display = lv_display_create(s_lcd->getFrameWidth(), s_lcd->getFrameHeight());
    if(display == nullptr) return ESP_ERR_NO_MEM;

    const uint32_t buffer_bytes = s_lcd->getFrameWidth() * s_lcd->getFrameHeight() * sizeof(uint16_t);
    void *buffer_1 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    void *buffer_2 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    s_dma_strip = static_cast<uint8_t *>(heap_caps_malloc(
        s_lcd->getFrameWidth() * DMA_LINES * sizeof(uint16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA));
    s_frame_queue = xQueueCreate(1, sizeof(Frame));
    s_frame_done = xSemaphoreCreateBinary();
    if(buffer_1 == nullptr || buffer_2 == nullptr || s_dma_strip == nullptr || s_frame_queue == nullptr || s_frame_done == nullptr) {
        heap_caps_free(buffer_1);
        heap_caps_free(buffer_2);
        heap_caps_free(s_dma_strip);
        if(s_frame_queue != nullptr) vQueueDelete(s_frame_queue);
        if(s_frame_done != nullptr) vSemaphoreDelete(s_frame_done);
        lv_display_delete(display);
        return ESP_ERR_NO_MEM;
    }

    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    /* Full-viewport capacity removes the old render/flush/render bands during
     * scroll. Keep dirty-area rendering so blinking eyes and small animations
     * don't repaint 129600 unchanged pixels on every update. */
    lv_display_set_buffers(display, buffer_1, buffer_2, buffer_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(display, s_lcd);
    lv_display_set_flush_cb(display, flush_display);
    lv_display_set_flush_wait_cb(display, wait_for_transfer);
    lv_display_add_event_cb(display, render_start, LV_EVENT_RENDER_START, nullptr);
    lv_timer_set_period(lv_display_get_refr_timer(display), 16);
    if(xTaskCreatePinnedToCore(transfer_frames, "havit_lcd_tx", 4096, nullptr, 5, nullptr, 1) != pdPASS)
        return ESP_ERR_NO_MEM;
    ESP_LOGI(TAG, "Full-viewport RGB565 buffers: 2 x %lu bytes in PSRAM, %lu-line DMA staging",
             (unsigned long)buffer_bytes, (unsigned long)DMA_LINES);
    return ESP_OK;
}
