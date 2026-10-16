#include "havit_ui.h"

#include "esp_log.h"
#include "lvgl.h"

extern const lv_font_t havit_instrument_sans_15;
extern const lv_font_t havit_instrument_sans_22;
extern const lv_font_t havit_instrument_sans_40;
extern const lv_image_dsc_t havit_google_g;

static lv_obj_t *s_views[5];
static havit_ui_screen_t s_current_screen = HAVIT_UI_SCREEN_LOGIN1;
static bool s_transitioning;
static lv_obj_t *s_login2_google_circle;
static lv_obj_t *s_login2_google_g;
static const char *TAG = "havit_ui";

static lv_obj_t *create_view(lv_obj_t *parent, havit_ui_screen_t screen)
{
    lv_obj_t *view = lv_obj_create(parent);
    lv_obj_remove_style_all(view);
    lv_obj_set_size(view, 360, 360);
    lv_obj_set_pos(view, 0, 0);
    lv_obj_clear_flag(view, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(view, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_flag(view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_opa(view, LV_OPA_COVER, LV_PART_MAIN);
    s_views[screen] = view;
    return view;
}

static lv_obj_t *create_label(lv_obj_t *parent, const char *text, int32_t y,
                              const lv_font_t *font, lv_color_t color, lv_opa_t opa)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, 360);
    lv_obj_set_pos(label, 0, y);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, opa, LV_PART_MAIN);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return label;
}

static void set_view_opa(void *object, int32_t opa)
{
    lv_obj_set_style_opa((lv_obj_t *)object, (lv_opa_t)opa, LV_PART_MAIN);
}

static lv_obj_t *s_transition_source;
static lv_obj_t *s_transition_destination;

static void source_fade_finished(lv_anim_t *animation)
{
    LV_UNUSED(animation);
    if(s_transition_source != NULL) {
        lv_obj_add_flag(s_transition_source, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(s_transition_source, LV_OPA_COVER, LV_PART_MAIN);
    }
    if(s_transition_destination != NULL) {
        lv_obj_set_style_opa(s_transition_destination, LV_OPA_COVER, LV_PART_MAIN);
    }
    s_transition_source = NULL;
    s_transition_destination = NULL;
    s_transitioning = false;
}

static void animate_transition(lv_obj_t *source, lv_obj_t *destination)
{
    s_transition_source = source;
    s_transition_destination = destination;
    lv_obj_clear_flag(destination, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_opa(destination, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_anim_t outgoing;
    lv_anim_init(&outgoing);
    lv_anim_set_var(&outgoing, source);
    lv_anim_set_exec_cb(&outgoing, set_view_opa);
    lv_anim_set_values(&outgoing, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&outgoing, 300);
    lv_anim_set_path_cb(&outgoing, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&outgoing, source_fade_finished);
    lv_anim_start(&outgoing);

    lv_anim_t incoming;
    lv_anim_init(&incoming);
    lv_anim_set_var(&incoming, destination);
    lv_anim_set_exec_cb(&incoming, set_view_opa);
    lv_anim_set_values(&incoming, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&incoming, 300);
    lv_anim_set_path_cb(&incoming, lv_anim_path_ease_in_out);
    lv_anim_start(&incoming);
}

void havit_ui_prepare_login_flow(lv_obj_t *parent)
{
    if(parent == NULL) return;

    s_current_screen = HAVIT_UI_SCREEN_LOGIN1;
    s_transitioning = false;
    s_transition_source = NULL;
    s_transition_destination = NULL;

    havit_ui_prepare_login1(parent);
    s_views[HAVIT_UI_SCREEN_LOGIN1] = havit_ui_login1_view();
    s_views[HAVIT_UI_SCREEN_LOGIN2] = create_view(parent, HAVIT_UI_SCREEN_LOGIN2);
    s_views[HAVIT_UI_SCREEN_WIFI] = create_view(parent, HAVIT_UI_SCREEN_WIFI);
    s_views[HAVIT_UI_SCREEN_PIN] = create_view(parent, HAVIT_UI_SCREEN_PIN);
    s_views[HAVIT_UI_SCREEN_WELCOME] = create_view(parent, HAVIT_UI_SCREEN_WELCOME);

    const lv_color_t white = lv_color_white();
    create_label(s_views[HAVIT_UI_SCREEN_LOGIN2], "Inicia Sesión", 144,
                 &havit_instrument_sans_22, white, (lv_opa_t)217);
    create_label(s_views[HAVIT_UI_SCREEN_LOGIN2], "Con tu Dispositivo", 172,
                 &havit_instrument_sans_15, white, (lv_opa_t)128);
    s_login2_google_circle = lv_obj_create(s_views[HAVIT_UI_SCREEN_LOGIN2]);
    lv_obj_remove_style_all(s_login2_google_circle);
    lv_obj_set_size(s_login2_google_circle, 54, 54);
    lv_obj_set_pos(s_login2_google_circle, 153, 211);
    lv_obj_set_style_radius(s_login2_google_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_login2_google_circle, lv_color_hex(0x77B7DB), 0);
    lv_obj_set_style_bg_opa(s_login2_google_circle, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_login2_google_circle, LV_OBJ_FLAG_SCROLLABLE);
    s_login2_google_g = lv_image_create(s_login2_google_circle);
    lv_image_set_src(s_login2_google_g, &havit_google_g);
    lv_obj_center(s_login2_google_g);
    lv_obj_clear_flag(s_login2_google_g, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    create_label(s_views[HAVIT_UI_SCREEN_WIFI], "Configuración de Wifi", 157,
                 &havit_instrument_sans_22, white, (lv_opa_t)217);
    create_label(s_views[HAVIT_UI_SCREEN_WIFI], "Buscando red", 185,
                 &havit_instrument_sans_15, white, (lv_opa_t)128);
    create_label(s_views[HAVIT_UI_SCREEN_PIN], "Vinculación", 157,
                 &havit_instrument_sans_22, white, (lv_opa_t)217);
    create_label(s_views[HAVIT_UI_SCREEN_PIN], "Mostrar Pin", 185,
                 &havit_instrument_sans_15, white, (lv_opa_t)128);
    create_label(s_views[HAVIT_UI_SCREEN_WELCOME], "Bienvenido de nuevo", 144,
                 &havit_instrument_sans_22, white, (lv_opa_t)217);
    create_label(s_views[HAVIT_UI_SCREEN_WELCOME], "Rom!", 169,
                 &havit_instrument_sans_40, lv_color_hex(0x77B7DB), LV_OPA_COVER);
}

void havit_ui_navigate_to(havit_ui_screen_t screen)
{
    if(s_transitioning || screen < HAVIT_UI_SCREEN_LOGIN1 || screen > HAVIT_UI_SCREEN_WELCOME ||
       screen == s_current_screen) return;

    lv_obj_t *source = s_views[s_current_screen];
    lv_obj_t *destination = s_views[screen];
    if(source == NULL || destination == NULL) return;

    ESP_LOGI(TAG, "Navigate %d -> %d", (int)s_current_screen, (int)screen);
    s_transitioning = true;
    s_current_screen = screen;
    animate_transition(source, destination);
}

void havit_ui_handle_tap(lv_obj_t *target)
{
    if(havit_ui_login1_is_button(target)) {
        ESP_LOGI(TAG, "Login 1 continue button tapped; opening Login 2");
        havit_ui_navigate_to(HAVIT_UI_SCREEN_LOGIN2);
        return;
    }
    if(target == s_login2_google_circle || target == s_login2_google_g) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_WIFI);
        return;
    }
    if(s_current_screen == HAVIT_UI_SCREEN_WIFI) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_PIN);
    }
    else if(s_current_screen == HAVIT_UI_SCREEN_PIN) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_WELCOME);
    }
}

void havit_ui_handle_touch(int32_t x, int32_t y)
{
    if(s_transitioning) return;
    if(s_current_screen == HAVIT_UI_SCREEN_LOGIN1 &&
       x >= 143 && x <= 217 && y >= 201 && y <= 275) {
        ESP_LOGI(TAG, "Touch at (%ld,%ld) inside Login 1 continue button", (long)x, (long)y);
        havit_ui_navigate_to(HAVIT_UI_SCREEN_LOGIN2);
    }
    else if(s_current_screen == HAVIT_UI_SCREEN_LOGIN2 &&
            x >= 143 && x <= 217 && y >= 201 && y <= 275) {
        ESP_LOGI(TAG, "Google button tapped; opening WiFi prototype view");
        havit_ui_navigate_to(HAVIT_UI_SCREEN_WIFI);
    }
    else if(s_current_screen == HAVIT_UI_SCREEN_WIFI) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_PIN);
    }
    else if(s_current_screen == HAVIT_UI_SCREEN_PIN) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_WELCOME);
    }
}
