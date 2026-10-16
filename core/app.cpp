#include "havit_app.h"

#include "havit_display.h"
#include "havit_touch.h"
#include "havit_ui.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

namespace {
constexpr char TAG[] = "havit_app";

void lvgl_task(void *)
{
    for(;;) {
        const uint32_t delay_ms = lv_timer_handler();
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
    ESP_LOGI(TAG, "Starting minimal HAVIT firmware");
    ESP_ERROR_CHECK(havit_display_init());
    ESP_ERROR_CHECK(havit_touch_init());
    havit_ui_show_blank();

    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer = nullptr;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 2000));
    ESP_ERROR_CHECK(xTaskCreate(lvgl_task, "havit_lvgl", 8192, nullptr, 5, nullptr) == pdPASS
                        ? ESP_OK : ESP_ERR_NO_MEM);
    ESP_LOGI(TAG, "Minimal black screen ready");
}
