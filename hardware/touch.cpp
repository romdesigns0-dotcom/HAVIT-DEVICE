#include "havit_touch.h"

#include "havit_board.h"
#include "esp_display_panel.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

using esp_panel::drivers::Touch;
using esp_panel::drivers::TouchPoint;

namespace {
constexpr char TAG[] = "havit_touch";
Touch *s_touch = nullptr;
volatile bool s_touch_pending = false;

void read_touch(lv_indev_t *, lv_indev_data_t *data)
{
    static TouchPoint last_point{};
    static int64_t last_press_us = 0;
    static bool was_pressed = false;
    const int64_t now = esp_timer_get_time();
    if(s_touch != nullptr && (!s_touch->isInterruptEnabled() || s_touch_pending)) {
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
    if(pressed && !was_pressed) ESP_LOGI(TAG, "Touch detected at x=%d y=%d", last_point.x, last_point.y);
    was_pressed = pressed;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
} // namespace

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
    return ESP_OK;
}
