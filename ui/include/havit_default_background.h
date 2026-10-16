#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Apply the shared black 360x360 background and gradient ring to a screen. */
void havit_default_background_create(lv_obj_t *parent);

#ifdef __cplusplus
}
#endif
