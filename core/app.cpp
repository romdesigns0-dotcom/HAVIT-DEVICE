#include "havit_app.h"

#include "havit_display.h"
#include "havit_board.h"
#include "havit_touch.h"
#include "havit_ui.h"
#include "havit_connectivity.h"
#include "havit_tasks.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "sdkconfig.h"
#if CONFIG_HAVIT_DEBUG_USB_CONTROL
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

namespace {
constexpr char TAG[] = "havit_app";
constexpr int64_t DISPLAY_DIM_TIMEOUT_US = 30LL * 1000000LL;
constexpr int64_t DISPLAY_OFF_TIMEOUT_US = 60LL * 1000000LL;
constexpr uint8_t DISPLAY_DIM_BRIGHTNESS_PERCENT = 20;
int64_t s_last_display_activity_us = 0;
bool s_backlight_awake = true;
bool s_backlight_dimmed = false;

bool display_activity(int32_t, int32_t)
{
    s_last_display_activity_us = esp_timer_get_time();
    if(!s_backlight_awake) {
        havit_board_set_brightness(100);
        havit_board_set_backlight(true);
        s_backlight_awake = true;
        s_backlight_dimmed = false;
        ESP_LOGI(TAG, "Display woke on touch");
        return true;
    }
    if(s_backlight_dimmed) {
        havit_board_set_brightness(100);
        s_backlight_dimmed = false;
        ESP_LOGI(TAG, "Display restored to full brightness on touch");
    }
    return false;
}

#if CONFIG_HAVIT_DEBUG_USB_CONTROL
/* Default USB Serial/JTAG VFS reads are nonblocking. Parse a bounded number
 * of bytes each frame; commands and LVGL run on the same task. */
void poll_usb_control()
{
    static char line[96];
    static size_t used;
    static bool overflow;
    char ch;
    for(unsigned budget = 0; budget < 96 && read(STDIN_FILENO, &ch, 1) == 1; ++budget) {
        if(ch == '\r' || ch == '\n') {
            if(overflow) ESP_LOGW(TAG, "USB command rejected: too long");
            else if(used) {
                line[used] = 0;
                int x1, y1, x2, y2, duration;
                char extra;
                esp_err_t result = ESP_ERR_INVALID_ARG;
                if(strcmp(line, "havit status") == 0) {
                    havit_net_snapshot_t net;
                    havit_connectivity_snapshot(&net);
                    ESP_LOGI(TAG, "USB status screen=%d network=%d identity=%d wifi=%d linked=%d",
                             (int)havit_ui_current_screen(), (int)net.state,
                             net.identity_provisioned, net.wifi_connected, net.account_linked);
                    result = ESP_OK;
                } else if(strcmp(line, "havit tasks") == 0) {
                    havit_tasks_status_t tasks;
                    havit_tasks_status(&tasks);
                    ESP_LOGI(TAG, "USB tasks count=%d pending=%d conflicts=%d revision=%u online=%d clock=%d zone=%s",
                             tasks.count, tasks.pending, tasks.conflicts, (unsigned)tasks.revision,
                             tasks.online, tasks.clock_valid, tasks.time_zone);
                    struct tm now; char day[11]; havit_tasks_local_time(&now);
                    strftime(day, sizeof(day), "%Y-%m-%d", &now);
                    ESP_LOGI(TAG, "USB tasks localDate=%s ready=%d", day, tasks.ready);
                    static havit_task_t task;
                    for(int i = 0; havit_tasks_get(i, &task); ++i)
                        ESP_LOGI(TAG, "USB task id=%s version=%d completed=%d", task.id, task.version, task.completed);
                    result = ESP_OK;
                } else if(strcmp(line, "havit task-test-create") == 0) {
                    static havit_task_t task;
                    memset(&task, 0, sizeof(task));
                    snprintf(task.title, sizeof(task.title), "Prueba de sincronizacion");
                    snprintf(task.detail, sizeof(task.detail), "Actividad temporal de comprobacion USB");
                    result = havit_tasks_save(&task, true) ? ESP_OK : ESP_FAIL;
                } else if(strcmp(line, "havit factory-reset") == 0) {
                    result = havit_connectivity_factory_reset();
                } else if(sscanf(line, "havit tap %d %d %c", &x1, &y1, &extra) == 2) {
                    result = havit_touch_debug_gesture(x1, y1, x1, y1, 100);
                } else if(sscanf(line, "havit swipe %d %d %d %d %d %c",
                                 &x1, &y1, &x2, &y2, &duration, &extra) == 5 && duration >= 0) {
                    result = havit_touch_debug_gesture(x1, y1, x2, y2, duration);
                }
                ESP_LOGI(TAG, "USB command %s", esp_err_to_name(result));
            }
            used = 0;
            overflow = false;
        } else if(!overflow) {
            if(used + 1 < sizeof(line)) line[used++] = ch;
            else overflow = true;
        }
    }
}
#endif

void lvgl_task(void *)
{
    int64_t last_health_us = esp_timer_get_time();
    for(;;) {
#if CONFIG_HAVIT_DEBUG_USB_CONTROL
        poll_usb_control();
#endif
        const uint32_t delay_ms = lv_timer_handler();
        havit_ui_tasks_tick();
        havit_ui_today_refresh();
        const int64_t now = esp_timer_get_time();
        const int64_t idle_us = now - s_last_display_activity_us;
        if(s_backlight_awake && !s_backlight_dimmed && idle_us >= DISPLAY_DIM_TIMEOUT_US) {
            havit_board_set_brightness(DISPLAY_DIM_BRIGHTNESS_PERCENT);
            s_backlight_dimmed = true;
            ESP_LOGI(TAG, "Display dimmed to %u%% after 30 seconds of inactivity",
                     (unsigned)DISPLAY_DIM_BRIGHTNESS_PERCENT);
        } else if(s_backlight_awake && s_backlight_dimmed && idle_us >= DISPLAY_OFF_TIMEOUT_US) {
            havit_board_set_backlight(false);
            s_backlight_awake = false;
            s_backlight_dimmed = false;
            ESP_LOGI(TAG, "Display backlight off after inactivity; Wi-Fi remains active");
        }
        if(now - last_health_us >= 10000000) {
            configASSERT(heap_caps_check_integrity_all(true));
            ESP_LOGI(TAG, "LVGL alive: uptime=%lld s, internal heap=%u bytes, PSRAM free=%u bytes",
                     now / 1000000, (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                     (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
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
    /* Keep larger UI/vector allocations in PSRAM so the two radio drivers can
     * still reserve internal DMA buffers after the display has started. Small
     * control objects remain internal; explicit capability allocations and
     * FreeRTOS stacks are unaffected. */
    heap_caps_malloc_extmem_enable(4096);
    ESP_LOGI(TAG, "Starting HAVIT firmware");
    ESP_ERROR_CHECK(havit_connectivity_init());
    ESP_ERROR_CHECK(havit_display_init());
    ESP_ERROR_CHECK(havit_touch_init());
    s_last_display_activity_us = esp_timer_get_time();
    havit_touch_set_activity_callback(display_activity);
    havit_ui_show_eyes_intro();

    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer = nullptr;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 2000));
    ESP_ERROR_CHECK(xTaskCreatePinnedToCore(lvgl_task, "havit_lvgl", 8192, nullptr, 5, nullptr, 0) == pdPASS
                        ? ESP_OK : ESP_ERR_NO_MEM);
    ESP_LOGI(TAG, "Eyes intro and Login 1 ready");
}
