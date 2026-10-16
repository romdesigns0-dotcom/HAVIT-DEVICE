#include "havit_ui.h"

#include "lvgl.h"

extern const lv_font_t havit_instrument_sans_22;
extern const lv_font_t havit_instrument_sans_14;
extern const lv_image_dsc_t havit_continue_arrow;

static const lv_font_t *s_title_font;
static const lv_font_t *s_caption_font;
static lv_obj_t *s_view;
static lv_obj_t *s_title;
static lv_obj_t *s_caption;
static lv_obj_t *s_button_circle;
static lv_obj_t *s_button_arrow;

static void set_object_opa(void *object, int32_t opa)
{
    lv_obj_set_style_opa((lv_obj_t *)object, (lv_opa_t)opa, LV_PART_MAIN);
}

static void animate_appearance(lv_obj_t *object, uint32_t delay_ms)
{
    if(object == NULL) return;

    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, object);
    lv_anim_set_exec_cb(&animation, set_object_opa);
    lv_anim_set_values(&animation, 0, LV_OPA_COVER);
    lv_anim_set_duration(&animation, 300);
    lv_anim_set_delay(&animation, delay_ms);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_in_out);
    lv_anim_start(&animation);
}

static lv_obj_t *create_centered_label(lv_obj_t *parent, const char *text,
                                       int32_t y, const lv_font_t *font, lv_opa_t text_opa)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, 360);
    lv_obj_set_pos(label, 0, y);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, text_opa, LV_PART_MAIN);
    lv_obj_set_style_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return label;
}

static lv_obj_t *create_arrow_image(lv_obj_t *parent)
{
    lv_obj_t *arrow = lv_image_create(parent);
    lv_image_set_src(arrow, &havit_continue_arrow);
    lv_obj_center(arrow);
    lv_obj_set_style_opa(arrow, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(arrow, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return arrow;
}

void havit_ui_prepare_login1(lv_obj_t *parent)
{
    if(parent == NULL) return;

    s_view = lv_obj_create(parent);
    lv_obj_remove_style_all(s_view);
    lv_obj_set_size(s_view, 360, 360);
    lv_obj_set_pos(s_view, 0, 0);
    lv_obj_clear_flag(s_view, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_view, LV_OBJ_FLAG_EVENT_BUBBLE);

    s_title_font = &havit_instrument_sans_22;
    s_caption_font = &havit_instrument_sans_14;

    s_title = create_centered_label(s_view, "Hola, Soy Havit.", 144,
                                    s_title_font, (lv_opa_t)217);
    s_caption = create_centered_label(s_view, "Empecemos por aquí", 172,
                                      s_caption_font, (lv_opa_t)128);

    s_button_circle = lv_obj_create(s_view);
    lv_obj_remove_style_all(s_button_circle);
    lv_obj_set_pos(s_button_circle, 153, 211);
    lv_obj_set_size(s_button_circle, 54, 54);
    lv_obj_set_style_radius(s_button_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_button_circle, lv_color_hex(0x77B7DB), 0);
    lv_obj_set_style_bg_opa(s_button_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_opa(s_button_circle, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(s_button_circle, LV_OBJ_FLAG_SCROLLABLE);
    s_button_arrow = create_arrow_image(s_button_circle);
    lv_obj_add_flag(s_button_circle, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_flag(s_view, LV_OBJ_FLAG_HIDDEN);
}

void havit_ui_reveal_login1(void)
{
    lv_obj_clear_flag(s_view, LV_OBJ_FLAG_HIDDEN);
    /* Stagger the three Login 1 layers while keeping the screen and background alive. */
    animate_appearance(s_title, 0);
    animate_appearance(s_caption, 100);
    animate_appearance(s_button_circle, 200);
    animate_appearance(s_button_arrow, 200);
}

lv_obj_t *havit_ui_login1_view(void)
{
    return s_view;
}

bool havit_ui_login1_is_button(lv_obj_t *object)
{
    return object == s_button_circle || object == s_button_arrow;
}
