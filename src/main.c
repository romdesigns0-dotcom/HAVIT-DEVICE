/**
 * @file main.c
 *
 * Minimal LVGL PC simulator.
 */

#include "lvgl.h"
#include "hal.h"

#if HAS_UI
/* Header exported by the LVGL Editor into the ui/ folder. */
#include "ui/ui.h"

typedef lv_obj_t *(*preview_screen_create_fn)(void);
typedef struct {
    const char *name;
    preview_screen_create_fn create;
} preview_screen_t;

/* Keep this list in the same order as the current Figma flow. The archived
 * frame_1/frame_2/frame_3 designs are intentionally omitted. */
static const preview_screen_t preview_screens[] = {
    {"intro_1", intro_1_create}, {"intro_2", intro_2_create},
    {"intro_3", intro_3_create}, {"intro_5", intro_5_create},
    {"intro_4", intro_4_create}, {"log_in", log_in_create},
    {"welcome_back", welcome_back_create}, {"today", today_create},
    {"today_c", today_c_create}, {"today_b", today_b_create},
    {"home", home_create}, {"esta_semana", esta_semana_create},
    {"add_task_p1", add_task_p1_create}, {"add_task_p2", add_task_p2_create},
    {"add_task_p3", add_task_p3_create}, {"add_task_p4", add_task_p4_create},
    {"add_task_p5_d", add_task_p5_d_create}, {"add_task_p5_e", add_task_p5_e_create},
    {"add_task_p5", add_task_p5_create}, {"add_task_p5_b", add_task_p5_b_create},
    {"add_task_p5_c", add_task_p5_c_create}, {"desglose_b", desglose_b_create},
    {"desglose", desglose_create}, {"add_task_p1_b", add_task_p1_b_create},
    {"add_task_p1_c", add_task_p1_c_create}, {"add_task_p1_d", add_task_p1_d_create},
};
static size_t preview_screen_index = 0;

static void preview_key_event(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_KEY) return;
    const uint32_t key = lv_event_get_key(event);
    const size_t screen_count = sizeof(preview_screens) / sizeof(preview_screens[0]);
    if(key == LV_KEY_RIGHT || key == LV_KEY_DOWN) {
        preview_screen_index = (preview_screen_index + 1) % screen_count;
    } else if(key == LV_KEY_LEFT || key == LV_KEY_UP) {
        preview_screen_index = (preview_screen_index + screen_count - 1) % screen_count;
    } else {
        return;
    }

    lv_obj_t *screen = preview_screens[preview_screen_index].create();
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(screen, preview_key_event, LV_EVENT_KEY, NULL);
    lv_group_t *group = lv_group_get_default();
    if(group != NULL) {
        lv_group_add_obj(group, screen);
        lv_group_focus_obj(screen);
    }
    lv_screen_load_anim(screen, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);
}

static void preview_load_screen(size_t index)
{
    const size_t screen_count = sizeof(preview_screens) / sizeof(preview_screens[0]);
    preview_screen_index = index % screen_count;
    lv_obj_t *screen = preview_screens[preview_screen_index].create();
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(screen, preview_key_event, LV_EVENT_KEY, NULL);
    lv_group_t *group = lv_group_get_default();
    if(group != NULL) {
        lv_group_add_obj(group, screen);
        lv_group_focus_obj(screen);
    }
    lv_screen_load(screen);
}
#endif

#ifdef _WIN32
#include <windows.h>
static void msleep(uint32_t ms) { Sleep(ms); }
#else
#include <unistd.h>
static void msleep(uint32_t ms) { usleep(ms * 1000); }
#endif

int main(void)
{
    /*Initialize LVGL*/
    lv_init();

    /* Build everything while holding the LVGL mutex. lv_lock()/lv_unlock() are
     * no-ops when no OS is used (the SDL backend) and guard against the driver's
     * window thread on the Windows backend. */
    lv_lock();

    /*Create the display and input devices (SDL, or the Windows driver)*/
    hal_init(360, 360);

#if HAS_UI
    /* Initialize the UI exported from the LVGL Editor.
     * "A:ui" tells LVGL where the file-based assets (images, fonts) live,
     * relative to the working directory. */
    ui_init("A:ui");

    /* Load one of your screens. Replace `some_screen` with a screen name from
     * your Editor project, e.g.:
     *     lv_screen_load(home_create());
     */
    /* Arrow keys cycle through the current Figma-exported screens. */
    preview_load_screen(0);
#else
    /*No UI exported yet: show a welcome message so the project runs out of the box.*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label,
                      "LVGL is running!\n"
                      "Export your UI from the LVGL Editor\n"
                      "into the ui/ folder to see it here.");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);
#endif

    lv_unlock();

    /*Handle LVGL tasks until the window is closed*/
    while(1) {
        uint32_t idle_ms = lv_timer_handler();
        if(idle_ms == LV_NO_TIMER_READY) {
            idle_ms = LV_DEF_REFR_PERIOD;
        }
        msleep(idle_ms);
    }

    return 0;
}
