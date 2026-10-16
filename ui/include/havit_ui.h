#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void havit_ui_show_eyes_intro(void);
void havit_ui_show_blank(void);
void havit_ui_prepare_login1(lv_obj_t *parent);
lv_obj_t *havit_ui_login1_view(void);
bool havit_ui_login1_is_button(lv_obj_t *object);
void havit_ui_reveal_login1(void);
void havit_ui_prepare_login_flow(lv_obj_t *parent);

typedef enum {
    HAVIT_UI_SCREEN_LOGIN1,
    HAVIT_UI_SCREEN_LOGIN2,
    HAVIT_UI_SCREEN_WIFI,
    HAVIT_UI_SCREEN_PIN,
    HAVIT_UI_SCREEN_WELCOME,
} havit_ui_screen_t;

void havit_ui_navigate_to(havit_ui_screen_t screen);
void havit_ui_handle_tap(lv_obj_t *target);
void havit_ui_handle_touch(int32_t x, int32_t y);

#ifdef __cplusplus
}
#endif
