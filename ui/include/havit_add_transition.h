#pragma once
#include "lvgl.h"

/* Pre-created objects matched across the two Figma states. */
typedef struct {
    lv_obj_t *view;
    lv_obj_t *fixed[7];
    lv_obj_t *color_buttons[2];
    lv_obj_t *date;
    lv_obj_t *controls[5];
    uint8_t control_count;
} havit_add_scene_t;

void havit_add_transition_start(havit_add_scene_t *source,
                                havit_add_scene_t *destination,
                                lv_anim_completed_cb_t completed);
