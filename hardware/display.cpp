#include "havit_display.h"

#include "havit_board.h"
#include "esp_display_panel.hpp"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "lvgl.h"
#include "draw/sw/lv_draw_sw_utils.h"

using esp_panel::drivers::LCD;

namespace {
LCD *s_lcd = nullptr;

bool lcd_draw_complete(void *user_data)
{
    lv_display_flush_ready(static_cast<lv_display_t *>(user_data));
    return false;
}

void flush_display(lv_display_t *display, const lv_area_t *area, uint8_t *pixels)
{
    const int width = area->x2 - area->x1 + 1;
    const int height = area->y2 - area->y1 + 1;
    lv_draw_sw_rgb565_swap(pixels, static_cast<uint32_t>(width * height));
    if(!s_lcd->drawBitmap(area->x1, area->y1, width, height, pixels, 0)) {
        lv_display_flush_ready(display);
    }
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

    constexpr uint32_t BUFFER_LINES = 16;
    const uint32_t buffer_bytes = s_lcd->getFrameWidth() * BUFFER_LINES * sizeof(lv_color_t);
    void *buffer_1 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    void *buffer_2 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if(buffer_1 == nullptr || buffer_2 == nullptr) {
        heap_caps_free(buffer_1);
        heap_caps_free(buffer_2);
        return ESP_ERR_NO_MEM;
    }

    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, buffer_1, buffer_2, buffer_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(display, s_lcd);
    lv_display_set_flush_cb(display, flush_display);
    if(!s_lcd->attachDrawBitmapFinishCallback(lcd_draw_complete, display)) return ESP_FAIL;
    return ESP_OK;
}
