#include "havit_eyes_intro_animation.h"

enum {
    EYE_BOTTOM = 213,
    EYE_CLOSED_HEIGHT = 4,
    EYE_REST_HEIGHT = 98,
    EYE_OPEN_HEIGHT = 110,
    INTRO_OPEN_MS = 460,
    INTRO_SETTLE_MS = 180,
    BLINK_CLOSE_MS = 120,
    BLINK_OPEN_MS = 160,
    BLINK_INTERVAL_MS = 3400,
};

typedef struct {
    lv_obj_t *left_eye;
    lv_obj_t *right_eye;
} eyes_animation_state_t;

static eyes_animation_state_t s_state;
static lv_timer_t *s_blink_timer;
static uint32_t s_completed_blinks;
static uint32_t s_exit_after_blink;
static havit_eyes_intro_completed_cb_t s_completed_cb;

static void set_height(void *var, int32_t height)
{
    eyes_animation_state_t *state = (eyes_animation_state_t *)var;
    if(state->left_eye == NULL || state->right_eye == NULL) return;

    const int32_t y = EYE_BOTTOM - height;
    lv_obj_set_size(state->left_eye, 36, height);
    lv_obj_set_pos(state->left_eye, 112, y);
    lv_obj_set_size(state->right_eye, 36, height);
    lv_obj_set_pos(state->right_eye, 211, y);
}

static void animate_height(int32_t from, int32_t to, uint32_t duration,
                           lv_anim_completed_cb_t completed_cb)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, &s_state);
    lv_anim_set_exec_cb(&anim, set_height);
    lv_anim_set_values(&anim, from, to);
    lv_anim_set_duration(&anim, duration);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&anim, completed_cb);
    lv_anim_start(&anim);
}

static void schedule_blink(void);
static void blink_closed(lv_anim_t *anim);

static void start_blink_now(void)
{
    animate_height(EYE_REST_HEIGHT, EYE_CLOSED_HEIGHT, BLINK_CLOSE_MS, blink_closed);
}

static void blink_opened(lv_anim_t *anim)
{
    LV_UNUSED(anim);

    s_completed_blinks++;
    if(s_exit_after_blink != 0 && s_completed_blinks >= s_exit_after_blink) {
        havit_eyes_intro_completed_cb_t completed_cb = s_completed_cb;
        s_completed_cb = NULL;
        s_state.left_eye = NULL;
        s_state.right_eye = NULL;
        if(completed_cb != NULL) completed_cb();
        return;
    }

    schedule_blink();
}

static void blink_closed(lv_anim_t *anim)
{
    LV_UNUSED(anim);
    animate_height(EYE_CLOSED_HEIGHT, EYE_REST_HEIGHT, BLINK_OPEN_MS, blink_opened);
}

static void blink_timer_cb(lv_timer_t *timer)
{
    lv_timer_del(timer);
    s_blink_timer = NULL;
    start_blink_now();
}

static void schedule_blink(void)
{
    if(s_blink_timer != NULL) lv_timer_del(s_blink_timer);
    s_blink_timer = lv_timer_create(blink_timer_cb, BLINK_INTERVAL_MS, NULL);
    if(s_blink_timer != NULL) lv_timer_set_repeat_count(s_blink_timer, 1);
}

static void intro_settled(lv_anim_t *anim)
{
    LV_UNUSED(anim);
    if(s_exit_after_blink != 0) start_blink_now();
    else schedule_blink();
}

static void intro_opened(lv_anim_t *anim)
{
    LV_UNUSED(anim);
    animate_height(EYE_OPEN_HEIGHT, EYE_REST_HEIGHT, INTRO_SETTLE_MS, intro_settled);
}

void havit_eyes_intro_animation_start(lv_obj_t *left_eye, lv_obj_t *right_eye,
                                      havit_eyes_intro_completed_cb_t completed_cb)
{
    if(left_eye == NULL || right_eye == NULL) return;

    if(s_blink_timer != NULL) {
        lv_timer_del(s_blink_timer);
        s_blink_timer = NULL;
    }

    lv_anim_delete(&s_state, set_height);
    s_state.left_eye = left_eye;
    s_state.right_eye = right_eye;
    s_completed_blinks = 0;
    s_exit_after_blink = 0;
    s_completed_cb = completed_cb;
    set_height(&s_state, EYE_CLOSED_HEIGHT);
    animate_height(EYE_CLOSED_HEIGHT, EYE_OPEN_HEIGHT, INTRO_OPEN_MS, intro_opened);
}

void havit_eyes_intro_animation_request_exit(void)
{
    if(s_exit_after_blink != 0) return;

    s_exit_after_blink = s_completed_blinks + 1;
    if(s_blink_timer != NULL) {
        lv_timer_del(s_blink_timer);
        s_blink_timer = NULL;
        start_blink_now();
    }
}
