#include "havit_touch.h"

#include "havit_board.h"
#include "esp_display_panel.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "sdkconfig.h"

using esp_panel::drivers::Touch;
using esp_panel::drivers::TouchPoint;

namespace {
constexpr char TAG[] = "havit_touch";
Touch *s_touch = nullptr;
volatile bool s_touch_pending = false;
havit_touch_callback_t s_touch_callback = nullptr;
havit_touch_activity_callback_t s_activity_callback = nullptr;
#if CONFIG_HAVIT_DEBUG_USB_CONTROL
struct DebugGesture {
    bool active;
    int x1, y1, x2, y2;
    int64_t started_us, duration_us;
} s_debug{};
#endif

void read_touch(lv_indev_t *, lv_indev_data_t *data)
{
    static TouchPoint last_point{};
    static int64_t last_press_us = 0;
    static bool was_pressed = false;
    static int64_t released_since_us = 0;
    static bool press_armed = true;
    static bool wake_press_consumed = false;
    const int64_t now = esp_timer_get_time();
#if CONFIG_HAVIT_DEBUG_USB_CONTROL
    if(s_debug.active) {
        const int64_t elapsed = now - s_debug.started_us;
        if(elapsed < 0) {
            data->point.x = last_point.x;
            data->point.y = last_point.y;
            data->state = LV_INDEV_STATE_RELEASED;
            was_pressed = false;
            last_press_us = 0;
            return;
        }
        const bool down = elapsed < s_debug.duration_us;
        const int64_t position = down ? elapsed : s_debug.duration_us;
        last_point.x = s_debug.x1 + (s_debug.x2 - s_debug.x1) * position / s_debug.duration_us;
        last_point.y = s_debug.y1 + (s_debug.y2 - s_debug.y1) * position / s_debug.duration_us;
        data->point.x = last_point.x;
        data->point.y = last_point.y;
        data->state = down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        if(down && s_activity_callback && s_activity_callback(last_point.x, last_point.y))
            wake_press_consumed = true;
        if(down && !was_pressed && !wake_press_consumed && s_touch_callback)
            s_touch_callback(last_point.x, last_point.y);
        if(wake_press_consumed) data->state = LV_INDEV_STATE_RELEASED;
        was_pressed = down;
        last_press_us = 0;
        if(!down && elapsed >= s_debug.duration_us + 200000) {
            s_debug.active = false;
            press_armed = true;
            wake_press_consumed = false;
            released_since_us = now;
            ESP_LOGI(TAG, "USB gesture finished at x=%d y=%d", last_point.x, last_point.y);
        }
        return;
    }
#endif
    /* Some CST816S firmware revisions interrupt on touch-down but do not
     * generate a fresh edge for every movement. Poll while a touch is active
     * so LVGL receives changing coordinates needed for scrolling. */
    if(s_touch != nullptr && (!s_touch->isInterruptEnabled() || s_touch_pending ||
                              last_press_us != 0)) {
        s_touch_pending = false;
        TouchPoint point;
        if(s_touch->readPoints(&point, 1, 0) > 0) {
            last_point = point;
            last_press_us = now;
        } else {
            last_press_us = 0;
        }
    }
    data->point.x = last_point.x;
    data->point.y = last_point.y;
    const bool pressed = last_press_us != 0 && now - last_press_us < 120000;
    if(pressed && s_activity_callback && s_activity_callback(last_point.x, last_point.y))
        wake_press_consumed = true;
    /* CST816S can briefly report no point during a held press. Require a
     * stable release before dispatching another application action. LVGL
     * still receives every sample for dragging and scrolling. */
    if(!pressed) {
        if(released_since_us == 0) released_since_us = now;
        if(now - released_since_us >= 150000) press_armed = true;
        if(now - released_since_us >= 150000) wake_press_consumed = false;
    } else {
        released_since_us = 0;
    }
    if(pressed && !was_pressed && press_armed && !wake_press_consumed) {
        press_armed = false;
        ESP_LOGI(TAG, "Touch detected at x=%d y=%d", last_point.x, last_point.y);
        if(s_touch_callback != nullptr) s_touch_callback(last_point.x, last_point.y);
    }
    was_pressed = pressed;
    data->state = pressed && !wake_press_consumed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
} // namespace

esp_err_t havit_touch_debug_gesture(int x1, int y1, int x2, int y2, unsigned duration_ms)
{
#if CONFIG_HAVIT_DEBUG_USB_CONTROL
    if(x1 < 0 || x1 >= 360 || y1 < 0 || y1 >= 360 ||
       x2 < 0 || x2 >= 360 || y2 < 0 || y2 >= 360 ||
       duration_ms < 50 || duration_ms > 5000) return ESP_ERR_INVALID_ARG;
    if(s_debug.active) return ESP_ERR_INVALID_STATE;
    /* Start after the physical pointer has delivered a stable release. */
    s_debug = {true, x1, y1, x2, y2, esp_timer_get_time() + 200000,
               static_cast<int64_t>(duration_ms) * 1000};
    return ESP_OK;
#else
    (void)x1; (void)y1; (void)x2; (void)y2; (void)duration_ms;
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

void havit_touch_set_callback(havit_touch_callback_t callback)
{
    s_touch_callback = callback;
}

void havit_touch_set_activity_callback(havit_touch_activity_callback_t callback)
{
    s_activity_callback = callback;
}

esp_err_t havit_touch_init(void)
{
    s_touch = havit_board_touch();
    if(s_touch == nullptr) return ESP_ERR_NOT_FOUND;
    if(s_touch->isInterruptEnabled()) {
        s_touch->attachInterruptCallback([](void *) -> bool {
            s_touch_pending = true;
            return false;
        });
    }
    lv_indev_t *pointer = lv_indev_create();
    if(pointer == nullptr) return ESP_ERR_NO_MEM;
    lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(pointer, read_touch);
    lv_timer_set_period(lv_indev_get_read_timer(pointer), 10);
    return ESP_OK;
}
