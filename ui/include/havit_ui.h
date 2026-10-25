#pragma once

#include "lvgl.h"
#include "havit_connectivity.h"

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
const char *havit_ui_device_setup_name(void);
void havit_ui_device_setup_set_pin(const char *pin);
/* Invoke only on the LVGL task. Network events publish snapshots instead. */
void havit_ui_device_setup_update(const havit_net_snapshot_t *status);

typedef enum {
    HAVIT_UI_SCREEN_LOGIN1,
    HAVIT_UI_SCREEN_LOGIN2,
    HAVIT_UI_SCREEN_WIFI,
    HAVIT_UI_SCREEN_PIN,
    HAVIT_UI_SCREEN_SETUP_BLE_READY,
    HAVIT_UI_SCREEN_SETUP_BLE_CONNECTED,
    HAVIT_UI_SCREEN_SETUP_WIFI_SCAN,
    HAVIT_UI_SCREEN_SETUP_WIFI_CONNECT,
    HAVIT_UI_SCREEN_SETUP_LINK_TOUCH,
    HAVIT_UI_SCREEN_SETUP_LINK_PIN,
    HAVIT_UI_SCREEN_SETUP_SYNC,
    HAVIT_UI_SCREEN_SETUP_RESET,
    HAVIT_UI_SCREEN_WELCOME,
    HAVIT_UI_SCREEN_TODAY,
    HAVIT_UI_SCREEN_TODAY_TASKS,
    HAVIT_UI_SCREEN_TODAY_TASKS_SELECT,
    HAVIT_UI_SCREEN_TODAY_TASK_DETAIL,
    HAVIT_UI_SCREEN_ADD,
    HAVIT_UI_SCREEN_CALENDAR,
    HAVIT_UI_SCREEN_CALENDAR_DAY,
    HAVIT_UI_SCREEN_TASK_QUICK,
    HAVIT_UI_SCREEN_TASK_QUICK_RUNNING,
    HAVIT_UI_SCREEN_TASK_QUICK_DONE,
    HAVIT_UI_SCREEN_TEAM_CREATE,
    HAVIT_UI_SCREEN_TEAM_GENERATING,
    HAVIT_UI_SCREEN_TEAM_TIMELINE,
    HAVIT_UI_SCREEN_TEAM_BREAKDOWN,
    HAVIT_UI_SCREEN_TEAM_SEND,
    HAVIT_UI_SCREEN_TASK_LIST,
    HAVIT_UI_SCREEN_TASK_DETAIL,
    HAVIT_UI_SCREEN_TASK_EDIT,
    HAVIT_UI_SCREEN_COUNT,
} havit_ui_screen_t;

void havit_ui_build_device_setup_views(lv_obj_t *ble_ready,
                                       lv_obj_t *ble_connected,
                                       lv_obj_t *wifi_scan,
                                       lv_obj_t *wifi_connect,
                                       lv_obj_t *link_touch,
                                       lv_obj_t *link_pin,
                                       lv_obj_t *sync);

void havit_ui_build_today_views(lv_obj_t *today, lv_obj_t *add,
                                lv_obj_t *calendar, lv_obj_t *calendar_day);
void havit_ui_build_today_task_views(lv_obj_t *list, lv_obj_t *select, lv_obj_t *detail);
bool havit_ui_today_task_handle_touch(havit_ui_screen_t screen, int32_t x, int32_t y);
void havit_ui_build_task_views(lv_obj_t *list, lv_obj_t *detail, lv_obj_t *editor);
void havit_ui_tasks_tick(void);
void havit_ui_tasks_open_list(const char *date);
void havit_ui_tasks_new(void);
void havit_ui_today_refresh(void);
void havit_ui_build_quick_task_views(lv_obj_t *quick, lv_obj_t *running,
                                     lv_obj_t *done);
void havit_ui_quick_task_reset(void);
void havit_ui_build_team_views(lv_obj_t *create, lv_obj_t *generating,
                               lv_obj_t *timeline, lv_obj_t *breakdown,
                               lv_obj_t *send);
void havit_ui_team_screen_changed(havit_ui_screen_t screen);
bool havit_ui_team_handle_touch(havit_ui_screen_t screen, int32_t x, int32_t y);
void havit_ui_today_handle_touch(havit_ui_screen_t screen, int32_t x, int32_t y);
bool havit_ui_quick_task_handle_touch(havit_ui_screen_t screen, int32_t x, int32_t y);
bool havit_ui_start_add_transition(havit_ui_screen_t source,
                                    havit_ui_screen_t destination,
                                    lv_anim_completed_cb_t completed);

void havit_ui_navigate_to(havit_ui_screen_t screen);
/* Read only on the LVGL task. */
havit_ui_screen_t havit_ui_current_screen(void);
void havit_ui_handle_tap(lv_obj_t *target);
void havit_ui_handle_touch(int32_t x, int32_t y);

#ifdef __cplusplus
}
#endif
