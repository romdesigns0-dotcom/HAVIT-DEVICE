#pragma once

#include "esp_err.h"
#include <stdint.h>

typedef void (*havit_touch_callback_t)(int32_t x, int32_t y);

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t havit_touch_init(void);
void havit_touch_set_callback(havit_touch_callback_t callback);

#ifdef __cplusplus
}
#endif
