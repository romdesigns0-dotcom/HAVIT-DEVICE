#pragma once

#include "esp_err.h"
#include <stdint.h>

typedef void (*havit_touch_callback_t)(int32_t x, int32_t y);
typedef bool (*havit_touch_activity_callback_t)(int32_t x, int32_t y);

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t havit_touch_init(void);
void havit_touch_set_callback(havit_touch_callback_t callback);
void havit_touch_set_activity_callback(havit_touch_activity_callback_t callback);
/* LVGL task only. A gesture includes a release interval before completion. */
esp_err_t havit_touch_debug_gesture(int x1, int y1, int x2, int y2, unsigned duration_ms);

#ifdef __cplusplus
}
#endif
