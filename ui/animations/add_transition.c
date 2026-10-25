#include "havit_add_transition.h"

enum { ADD_TRANSITION_DURATION_MS = 300 };

static struct {
    havit_add_scene_t *source;
    havit_add_scene_t *destination;
    lv_anim_completed_cb_t completed;
} s_motion;

static void opacity(lv_obj_t *object, int32_t value)
{
    lv_obj_set_style_opa(object, (lv_opa_t)value, 0);
}

static void restore_children_opacity(lv_obj_t *view)
{
    for(uint32_t i = 0; i < lv_obj_get_child_count(view); i++)
        opacity(lv_obj_get_child(view, i), LV_OPA_COVER);
}

static void animate_components(void *context, int32_t progress)
{
    LV_UNUSED(context);
    /* The source holds the matched gradient, rings, text, and mint circles.
     * Only changed layers animate. Blending the destination color over the
     * opaque source circle interpolates its color without a black dip. */
    for(unsigned i = 0; i < 2; i++)
        opacity(s_motion.destination->color_buttons[i], progress);
    opacity(s_motion.source->date, LV_OPA_COVER - progress);
    opacity(s_motion.destination->date, progress);
    for(unsigned i = 0; i < s_motion.source->control_count; i++)
        opacity(s_motion.source->controls[i], LV_OPA_COVER - progress);
    for(unsigned i = 0; i < s_motion.destination->control_count; i++)
        opacity(s_motion.destination->controls[i], progress);
}

static void finish(lv_anim_t *animation)
{
    lv_obj_add_flag(s_motion.source->view, LV_OBJ_FLAG_HIDDEN);
    for(unsigned i = 0; i < 7; i++)
        lv_obj_clear_flag(s_motion.destination->fixed[i], LV_OBJ_FLAG_HIDDEN);
    opacity(s_motion.source->date, LV_OPA_COVER);
    opacity(s_motion.destination->date, LV_OPA_COVER);
    for(unsigned i = 0; i < 2; i++)
        opacity(s_motion.destination->color_buttons[i], LV_OPA_COVER);
    for(unsigned i = 0; i < s_motion.source->control_count; i++)
        opacity(s_motion.source->controls[i], LV_OPA_COVER);
    for(unsigned i = 0; i < s_motion.destination->control_count; i++)
        opacity(s_motion.destination->controls[i], LV_OPA_COVER);
    if(s_motion.completed != NULL) s_motion.completed(animation);
}

void havit_add_transition_start(havit_add_scene_t *source,
                                havit_add_scene_t *destination,
                                lv_anim_completed_cb_t completed)
{
    s_motion.source = source;
    s_motion.destination = destination;
    s_motion.completed = completed;
    lv_obj_set_style_opa(source->view, LV_OPA_COVER, 0);
    lv_obj_set_style_opa(destination->view, LV_OPA_COVER, 0);
    /* Quick Task fades every Add child before hiding the view. Restore the
     * complete destination first so a later Today -> Add entry also revives
     * fixed layers that are not part of this transition's animated arrays. */
    restore_children_opacity(destination->view);
    lv_obj_move_foreground(destination->view);
    for(unsigned i = 0; i < 7; i++)
        lv_obj_add_flag(destination->fixed[i], LV_OBJ_FLAG_HIDDEN);
    animate_components(&s_motion, 0);
    lv_obj_clear_flag(destination->view, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, &s_motion);
    lv_anim_set_exec_cb(&animation, animate_components);
    lv_anim_set_values(&animation, 0, LV_OPA_COVER);
    lv_anim_set_duration(&animation, ADD_TRANSITION_DURATION_MS);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_set_completed_cb(&animation, finish);
    lv_anim_start(&animation);
}
