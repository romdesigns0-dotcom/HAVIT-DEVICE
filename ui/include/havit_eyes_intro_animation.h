#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*havit_eyes_intro_completed_cb_t)(void);

void havit_eyes_intro_animation_start(lv_obj_t *left_eye, lv_obj_t *right_eye,
                                      havit_eyes_intro_completed_cb_t completed_cb);
void havit_eyes_intro_animation_request_exit(void);

#ifdef __cplusplus
}
#endif
