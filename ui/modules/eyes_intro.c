#include "havit_ui.h"

#include "havit_eyes_intro_animation.h"
#include "havit_device_claim.h"
#include "havit_default_background.h"
#include "havit_touch.h"
#include "esp_log.h"
#include "lvgl.h"
#include <stdbool.h>

static lv_obj_t *s_left_eye;
static lv_obj_t *s_right_eye;
static lv_obj_t *s_default_1_background;
static lv_obj_t *s_default_2_background;
static lv_obj_t *s_default_3_background;
static lv_obj_t *s_default_4_background;
static bool s_intro_finished;
static bool s_login_visible;
static const char *TAG = "havit_intro";

static lv_obj_t *create_eye(lv_obj_t *parent, int32_t x)
{
    lv_obj_t *eye = lv_obj_create(parent);
    lv_obj_remove_style_all(eye);
    lv_obj_set_size(eye, 36, 4);
    lv_obj_set_pos(eye, x, 209);
    lv_obj_set_style_bg_color(eye, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(eye, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(eye, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(eye, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(eye, (lv_opa_t)64, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(eye, 21, LV_PART_MAIN);
    lv_obj_set_style_shadow_offset_y(eye, 2, LV_PART_MAIN);
    lv_obj_clear_flag(eye, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return eye;
}

static void reveal_login1_after_eyes(lv_anim_t *animation)
{
    LV_UNUSED(animation);
    /* Default 1 belongs to the eyes; the app uses the thinner Default 2 asset. */
    if(s_default_1_background != NULL)
        lv_obj_add_flag(s_default_1_background, LV_OBJ_FLAG_HIDDEN);
    if(s_default_2_background != NULL)
        lv_obj_clear_flag(s_default_2_background, LV_OBJ_FLAG_HIDDEN);
    ESP_LOGI(TAG, "Eye blink complete; revealing Login 1");
    havit_device_claim_snapshot_t claim;
    havit_device_claim_snapshot(&claim);
    if(claim.binding_id[0]) havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY);
    else havit_ui_reveal_login1();
    s_login_visible = true;
}

static void set_eye_opa(void *object, int32_t opa)
{
    lv_obj_set_style_opa((lv_obj_t *)object, (lv_opa_t)opa, LV_PART_MAIN);
}

static void eyes_intro_completed(void)
{
    s_intro_finished = true;
    ESP_LOGI(TAG, "Intro exit requested; fading eyes to Login 1");
    lv_anim_t fade;
    lv_anim_init(&fade);
    lv_anim_set_var(&fade, s_left_eye);
    lv_anim_set_exec_cb(&fade, set_eye_opa);
    lv_anim_set_values(&fade, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&fade, 300);
    lv_anim_set_path_cb(&fade, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&fade, reveal_login1_after_eyes);
    lv_anim_start(&fade);

    lv_anim_t right_fade;
    lv_anim_init(&right_fade);
    lv_anim_set_var(&right_fade, s_right_eye);
    lv_anim_set_exec_cb(&right_fade, set_eye_opa);
    lv_anim_set_values(&right_fade, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&right_fade, 300);
    lv_anim_set_path_cb(&right_fade, lv_anim_path_ease_in_out);
    lv_anim_start(&right_fade);
}

static void intro_touched(int32_t x, int32_t y)
{
    if(!s_intro_finished) {
        ESP_LOGI(TAG, "Touch at (%d,%d) on eyes; starting final blink",
                 (int)x, (int)y);
        havit_eyes_intro_animation_request_exit();
    }
    else if(s_login_visible) {
        ESP_LOGI(TAG, "Touch at (%d,%d) after intro; routing to Login view",
                 (int)x, (int)y);
        havit_ui_handle_touch(x, y);
    }
}

static void resume_known_device(lv_timer_t *timer)
{
    if(s_intro_finished) { lv_timer_delete(timer); return; }
    havit_device_claim_snapshot_t claim;
    havit_device_claim_snapshot(&claim);
    if(claim.binding_id[0]) {
        havit_eyes_intro_animation_request_exit();
        lv_timer_delete(timer);
    }
}

void havit_ui_show_eyes_intro(void)
{
    s_intro_finished = false;
    s_login_visible = false;
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(screen);
    s_default_3_background = havit_default_background_create(
        screen, HAVIT_DEFAULT_BACKGROUND_3_SCROLL_BASE);
    s_default_1_background = havit_default_background_create(
        screen, HAVIT_DEFAULT_BACKGROUND_1_EYES);
    s_default_2_background = havit_default_background_create(
        screen, HAVIT_DEFAULT_BACKGROUND_2_APP);
    s_default_4_background = havit_default_background_create(
        screen, HAVIT_DEFAULT_BACKGROUND_4_SCROLL_RING);
    if(s_default_2_background != NULL)
        lv_obj_add_flag(s_default_2_background, LV_OBJ_FLAG_HIDDEN);
    if(s_default_3_background != NULL)
        lv_obj_move_background(s_default_3_background);
    if(s_default_4_background != NULL)
        lv_obj_add_flag(s_default_4_background, LV_OBJ_FLAG_HIDDEN);
    havit_ui_prepare_login_flow(screen);

    s_left_eye = create_eye(screen, 112);
    s_right_eye = create_eye(screen, 211);
    havit_touch_set_callback(intro_touched);
    lv_screen_load(screen);

    havit_eyes_intro_animation_start(s_left_eye, s_right_eye, eyes_intro_completed);
    lv_timer_create(resume_known_device, 1000, NULL);
}
