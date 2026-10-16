#include "havit_app.h"

#include "havit_display.h"
#include "havit_touch.h"
#include "havit_ui.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

namespace {
constexpr char TAG[] = "havit_app";

void lvgl_task(void *)
{
    int64_t last_health_us = esp_timer_get_time();
    for(;;) {
        const uint32_t delay_ms = lv_timer_handler();
        const int64_t now = esp_timer_get_time();
        if(now - last_health_us >= 10000000) {
            configASSERT(heap_caps_check_integrity_all(true));
            ESP_LOGI(TAG, "LVGL alive: uptime=%lld s, internal heap=%u bytes",
                     now / 1000000, (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
            last_health_us = now;
        }
        vTaskDelay(pdMS_TO_TICKS(delay_ms == LV_NO_TIMER_READY || delay_ms > 10 ? 10 : delay_ms));
    }
}

void lvgl_tick(void *)
{
    lv_tick_inc(2);
}
} // namespace

void havit_app_start(void)
{
    ESP_LOGI(TAG, "Starting HAVIT firmware");
    ESP_ERROR_CHECK(havit_display_init());
    ESP_ERROR_CHECK(havit_touch_init());
    havit_ui_show_eyes_intro();

    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer = nullptr;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 2000));
    ESP_ERROR_CHECK(xTaskCreate(lvgl_task, "havit_lvgl", 8192, nullptr, 5, nullptr) == pdPASS
                        ? ESP_OK : ESP_ERR_NO_MEM);
    ESP_LOGI(TAG, "Eyes intro and Login 1 ready");
}
