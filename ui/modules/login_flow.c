#include "havit_ui.h"
#include "havit_default_background.h"

#include "esp_log.h"
#include "lvgl.h"

extern const lv_font_t havit_instrument_sans_15;
extern const lv_font_t havit_instrument_sans_22;
extern const lv_font_t havit_instrument_sans_28;
extern const lv_font_t havit_instrument_sans_12;

static lv_obj_t *s_views[HAVIT_UI_SCREEN_COUNT];
static havit_ui_screen_t s_current_screen = HAVIT_UI_SCREEN_LOGIN1;
static bool s_transitioning;
static lv_obj_t *s_connection_summary;
static lv_obj_t *s_welcome_name;
static bool s_reset_requested;
static lv_timer_t *s_welcome_timer;
static lv_timer_t *s_network_timer;
static uint32_t s_network_revision = UINT32_MAX;
static bool s_network_press_pending;
static uint32_t s_network_release_start;
static havit_ui_screen_t s_network_press_screen;
static void setup_button_tapped(void);
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

static void connection_button(lv_obj_t *parent, const char *text, int y)
{
    lv_obj_t *button = lv_obj_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_set_size(button, 216, 40);
    lv_obj_set_pos(button, 72, y);
    lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x6BB8D1), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *label = lv_label_create(button);
    lv_obj_set_style_text_font(label, &havit_instrument_sans_12, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t *s_transition_source;
static lv_obj_t *s_transition_destination;
static uint8_t s_dissolve_timeline;
static uint8_t s_welcome_timeline;
static uint8_t s_quick_task_timeline;
static uint8_t s_today_task_timeline;
static void source_fade_finished(lv_anim_t *animation);
static void children_opacity(lv_obj_t *view, lv_opa_t opacity);

static bool is_today_task_screen(havit_ui_screen_t screen)
{
    return screen >= HAVIT_UI_SCREEN_TODAY_TASKS &&
           screen <= HAVIT_UI_SCREEN_TODAY_TASK_DETAIL;
}

static void today_task_transition_progress(void *context, int32_t progress)
{
    LV_UNUSED(context);
    children_opacity(s_transition_source, (lv_opa_t)(LV_OPA_COVER - progress));
    children_opacity(s_transition_destination, (lv_opa_t)progress);
}

static void animate_today_task_transition(void)
{
    lv_obj_move_foreground(s_transition_destination);
    lv_obj_set_style_opa(s_transition_destination, LV_OPA_COVER, 0);
    today_task_transition_progress(NULL, 0);
    lv_obj_clear_flag(s_transition_destination, LV_OBJ_FLAG_HIDDEN);
    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, &s_today_task_timeline);
    lv_anim_set_exec_cb(&animation, today_task_transition_progress);
    lv_anim_set_values(&animation, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&animation, 300);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_set_completed_cb(&animation, source_fade_finished);
    lv_anim_start(&animation);
    ESP_LOGI(TAG, "Today task Smart Animate: Ease Out, 300 ms");
}

static void welcome_delay_elapsed(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    s_welcome_timer = NULL; /* The one-shot timer deletes itself after this callback. */
    if(s_current_screen == HAVIT_UI_SCREEN_WELCOME && !s_transitioning) {
        ESP_LOGI(TAG, "Welcome visible for 800 ms; opening Today automatically");
        havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY);
    }
}

static void dissolve_progress(void *context, int32_t progress)
{
    LV_UNUSED(context);
    lv_obj_set_style_opa(s_transition_source, (lv_opa_t)(LV_OPA_COVER - progress), 0);
    lv_obj_set_style_opa(s_transition_destination, (lv_opa_t)progress, 0);
}

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
    if(s_current_screen == HAVIT_UI_SCREEN_WELCOME && s_welcome_timer == NULL) {
        /* Count the delay from the end of the incoming transition, once the
         * welcome message is fully visible, rather than from the PIN tap. */
        s_welcome_timer = lv_timer_create(welcome_delay_elapsed, 800, NULL);
        if(s_welcome_timer != NULL) lv_timer_set_repeat_count(s_welcome_timer, 1);
        else ESP_LOGE(TAG, "Unable to create Welcome delay timer");
    }
}

static void children_opacity(lv_obj_t *view, lv_opa_t opacity)
{
    for(uint32_t i = 0; i < lv_obj_get_child_count(view); i++)
        lv_obj_set_style_opa(lv_obj_get_child(view, i), opacity, 0);
}

static void welcome_to_today_progress(void *context, int32_t progress)
{
    LV_UNUSED(context);
    /* The default background is the matched Figma layer and remains fixed.
     * Welcome text and Today components have no matching geometry: Smart
     * Animate fades these unmatched layers, without invented translations. */
    children_opacity(s_transition_source, (lv_opa_t)(LV_OPA_COVER - progress));
    children_opacity(s_transition_destination, (lv_opa_t)progress);
}

static void welcome_to_today_finished(lv_anim_t *animation)
{
    lv_obj_add_flag(s_transition_source, LV_OBJ_FLAG_HIDDEN);
    children_opacity(s_transition_source, LV_OPA_COVER);
    children_opacity(s_transition_destination, LV_OPA_COVER);
    source_fade_finished(animation);
}

static void quick_task_progress(void *context, int32_t progress)
{
    LV_UNUSED(context);
    children_opacity(s_transition_source, (lv_opa_t)(LV_OPA_COVER - progress));
    children_opacity(s_transition_destination, (lv_opa_t)progress);
}

static void quick_task_transition_finished(lv_anim_t *animation)
{
    /* The Add page is reused after entering Quick Task. Normalize every
     * component before exposing it again so it cannot remain as only the
     * shared background ring after a return transition. */
    if(s_current_screen == HAVIT_UI_SCREEN_ADD &&
       s_transition_destination == s_views[HAVIT_UI_SCREEN_ADD]) {
        lv_obj_clear_flag(s_transition_destination, LV_OBJ_FLAG_HIDDEN);
        children_opacity(s_transition_destination, LV_OPA_COVER);
        ESP_LOGI(TAG, "Quick Task returned to Add; restored its components");
    }
    source_fade_finished(animation);
}

static void animate_quick_task(void)
{
    lv_obj_move_foreground(s_transition_destination);
    lv_obj_set_style_opa(s_transition_destination, LV_OPA_COVER, 0);
    quick_task_progress(NULL, 0);
    lv_obj_clear_flag(s_transition_destination, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, &s_quick_task_timeline);
    lv_anim_set_exec_cb(&animation, quick_task_progress);
    lv_anim_set_values(&animation, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&animation, 300);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_set_completed_cb(&animation, quick_task_transition_finished);
    lv_anim_start(&animation);
    ESP_LOGI(TAG, "Quick Task: Smart Animate, 300 ms, Ease Out");
}

static void animate_welcome_to_today(void)
{
    lv_obj_move_foreground(s_transition_destination);
    lv_obj_set_style_opa(s_transition_destination, LV_OPA_COVER, 0);
    welcome_to_today_progress(NULL, 0);
    lv_obj_clear_flag(s_transition_destination, LV_OBJ_FLAG_HIDDEN);
    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, &s_welcome_timeline);
    lv_anim_set_exec_cb(&animation, welcome_to_today_progress);
    lv_anim_set_values(&animation, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&animation, 300);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&animation, welcome_to_today_finished);
    lv_anim_start(&animation);
    ESP_LOGI(TAG, "Welcome -> Today: Smart Animate, 300 ms, Ease In Out");
}

/* Different page layouts (including calendar) use Dissolve, not object motion. */
static void animate_dissolve(lv_obj_t *source, lv_obj_t *destination)
{
    s_transition_source = source;
    s_transition_destination = destination;
    lv_obj_move_foreground(destination);
    lv_obj_set_style_opa(destination, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(destination, LV_OBJ_FLAG_HIDDEN);

    /* One clock keeps the outgoing/incoming opacity and input lock in sync. */
    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, &s_dissolve_timeline);
    lv_anim_set_exec_cb(&animation, dissolve_progress);
    lv_anim_set_values(&animation, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&animation, 300);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_set_completed_cb(&animation, source_fade_finished);
    lv_anim_start(&animation);
    ESP_LOGI(TAG, "Dissolve: 300 ms, Ease Out");
}

static bool is_setup_screen(void)
{
    return s_current_screen >= HAVIT_UI_SCREEN_SETUP_BLE_READY &&
           s_current_screen <= HAVIT_UI_SCREEN_SETUP_RESET;
}

static void network_poll(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if(!is_setup_screen() && s_current_screen != HAVIT_UI_SCREEN_LOGIN2) return;
    havit_net_snapshot_t status;
    havit_connectivity_snapshot(&status);
    if(s_current_screen == HAVIT_UI_SCREEN_LOGIN2) {
        lv_label_set_text(s_connection_summary, status.has_saved_network ?
            "Hay una red Wi-Fi guardada.\nPuedes verla o restablecerla." :
            "Configura Wi-Fi desde tu móvil\nmediante Bluetooth seguro.");
        return;
    }
    if(s_current_screen == HAVIT_UI_SCREEN_SETUP_RESET) {
        if(s_reset_requested && !s_transitioning &&
           ((status.state == HAVIT_NET_BLE_READY && !status.wifi_connected) ||
            status.state == HAVIT_NET_ERROR)) {
            s_reset_requested = false;
            s_network_revision = UINT32_MAX;
            havit_ui_navigate_to(status.state == HAVIT_NET_ERROR ?
                HAVIT_UI_SCREEN_SETUP_WIFI_CONNECT : HAVIT_UI_SCREEN_SETUP_BLE_READY);
        }
        return;
    }
    lv_indev_t *input = lv_indev_get_next(NULL);
    if(s_network_press_pending && input &&
       lv_indev_get_state(input) == LV_INDEV_STATE_RELEASED) {
        if(!s_network_release_start) s_network_release_start = lv_tick_get();
        if(lv_tick_elaps(s_network_release_start) >= 150) {
            s_network_press_pending = false;
            s_network_release_start = 0;
            if(!s_transitioning && s_network_press_screen == s_current_screen)
                setup_button_tapped();
        }
    } else s_network_release_start = 0;
    if(s_transitioning) return;
    if(status.revision != s_network_revision) {
        havit_ui_device_setup_update(&status);
        s_network_revision = status.revision;
    }
    if(s_welcome_name != NULL && status.account_linked)
        lv_label_set_text(s_welcome_name, status.account_name[0] ? status.account_name : "¡Hola!");
    havit_ui_screen_t destination = HAVIT_UI_SCREEN_SETUP_BLE_READY;
    switch(status.state) {
    case HAVIT_NET_BLE_CONNECTED:
    case HAVIT_NET_SECURE: destination = HAVIT_UI_SCREEN_SETUP_BLE_CONNECTED; break;
    case HAVIT_NET_SCAN: destination = HAVIT_UI_SCREEN_SETUP_WIFI_SCAN; break;
    case HAVIT_NET_CONNECTING:
    case HAVIT_NET_ERROR: destination = HAVIT_UI_SCREEN_SETUP_WIFI_CONNECT; break;
    case HAVIT_NET_BACKEND_ERROR:
    case HAVIT_NET_LINK_PENDING:
    case HAVIT_NET_ONLINE: destination = HAVIT_UI_SCREEN_SETUP_LINK_TOUCH; break;
    case HAVIT_NET_LINKED: destination = HAVIT_UI_SCREEN_SETUP_SYNC; break;
    default: break;
    }
    havit_ui_navigate_to(destination);
}

havit_ui_screen_t havit_ui_current_screen(void)
{
    return s_current_screen;
}

static void setup_button_tapped(void)
{
    havit_net_snapshot_t status;
    havit_connectivity_snapshot(&status);
    if(s_current_screen == HAVIT_UI_SCREEN_SETUP_LINK_TOUCH) {
        havit_connectivity_refresh_backend();
    } else if(status.state == HAVIT_NET_ERROR && !status.fatal_error) {
        if(status.provisioning || status.has_saved_network) havit_connectivity_retry();
        else havit_connectivity_begin(false);
    } else if(status.claim_expired) {
        havit_connectivity_begin(true);
    } else if(status.state == HAVIT_NET_BACKEND_ERROR || status.state == HAVIT_NET_LINK_PENDING) {
        havit_connectivity_refresh_backend();
    } else if(status.state == HAVIT_NET_LINKED && status.account_linked) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_WELCOME);
    }
}

void havit_ui_prepare_login_flow(lv_obj_t *parent)
{
    if(parent == NULL) return;
    if(s_welcome_timer != NULL) {
        lv_timer_delete(s_welcome_timer);
        s_welcome_timer = NULL;
    }

    s_current_screen = HAVIT_UI_SCREEN_LOGIN1;
    s_transitioning = false;
    s_transition_source = NULL;
    s_transition_destination = NULL;

    havit_ui_prepare_login1(parent);
    s_views[HAVIT_UI_SCREEN_LOGIN1] = havit_ui_login1_view();
    s_views[HAVIT_UI_SCREEN_LOGIN2] = create_view(parent, HAVIT_UI_SCREEN_LOGIN2);
    s_views[HAVIT_UI_SCREEN_WIFI] = create_view(parent, HAVIT_UI_SCREEN_WIFI);
    s_views[HAVIT_UI_SCREEN_PIN] = create_view(parent, HAVIT_UI_SCREEN_PIN);
    s_views[HAVIT_UI_SCREEN_SETUP_BLE_READY] =
        create_view(parent, HAVIT_UI_SCREEN_SETUP_BLE_READY);
    s_views[HAVIT_UI_SCREEN_SETUP_BLE_CONNECTED] =
        create_view(parent, HAVIT_UI_SCREEN_SETUP_BLE_CONNECTED);
    s_views[HAVIT_UI_SCREEN_SETUP_WIFI_SCAN] =
        create_view(parent, HAVIT_UI_SCREEN_SETUP_WIFI_SCAN);
    s_views[HAVIT_UI_SCREEN_SETUP_WIFI_CONNECT] =
        create_view(parent, HAVIT_UI_SCREEN_SETUP_WIFI_CONNECT);
    s_views[HAVIT_UI_SCREEN_SETUP_LINK_TOUCH] =
        create_view(parent, HAVIT_UI_SCREEN_SETUP_LINK_TOUCH);
    s_views[HAVIT_UI_SCREEN_SETUP_LINK_PIN] =
        create_view(parent, HAVIT_UI_SCREEN_SETUP_LINK_PIN);
    s_views[HAVIT_UI_SCREEN_SETUP_SYNC] =
        create_view(parent, HAVIT_UI_SCREEN_SETUP_SYNC);
    s_views[HAVIT_UI_SCREEN_WELCOME] = create_view(parent, HAVIT_UI_SCREEN_WELCOME);
    s_views[HAVIT_UI_SCREEN_TODAY] = create_view(parent, HAVIT_UI_SCREEN_TODAY);
    s_views[HAVIT_UI_SCREEN_TODAY_TASKS] = create_view(parent, HAVIT_UI_SCREEN_TODAY_TASKS);
    s_views[HAVIT_UI_SCREEN_TODAY_TASKS_SELECT] =
        create_view(parent, HAVIT_UI_SCREEN_TODAY_TASKS_SELECT);
    s_views[HAVIT_UI_SCREEN_TODAY_TASK_DETAIL] =
        create_view(parent, HAVIT_UI_SCREEN_TODAY_TASK_DETAIL);
    s_views[HAVIT_UI_SCREEN_ADD] = create_view(parent, HAVIT_UI_SCREEN_ADD);
    s_views[HAVIT_UI_SCREEN_CALENDAR] = create_view(parent, HAVIT_UI_SCREEN_CALENDAR);
    s_views[HAVIT_UI_SCREEN_CALENDAR_DAY] = create_view(parent, HAVIT_UI_SCREEN_CALENDAR_DAY);
    s_views[HAVIT_UI_SCREEN_TASK_QUICK] = create_view(parent, HAVIT_UI_SCREEN_TASK_QUICK);
    s_views[HAVIT_UI_SCREEN_TASK_QUICK_RUNNING] =
        create_view(parent, HAVIT_UI_SCREEN_TASK_QUICK_RUNNING);
    s_views[HAVIT_UI_SCREEN_TASK_QUICK_DONE] =
        create_view(parent, HAVIT_UI_SCREEN_TASK_QUICK_DONE);
    s_views[HAVIT_UI_SCREEN_TEAM_CREATE] = create_view(parent, HAVIT_UI_SCREEN_TEAM_CREATE);
    s_views[HAVIT_UI_SCREEN_TEAM_GENERATING] =
        create_view(parent, HAVIT_UI_SCREEN_TEAM_GENERATING);
    s_views[HAVIT_UI_SCREEN_TEAM_TIMELINE] =
        create_view(parent, HAVIT_UI_SCREEN_TEAM_TIMELINE);
    s_views[HAVIT_UI_SCREEN_TEAM_BREAKDOWN] =
        create_view(parent, HAVIT_UI_SCREEN_TEAM_BREAKDOWN);
    s_views[HAVIT_UI_SCREEN_TEAM_SEND] = create_view(parent, HAVIT_UI_SCREEN_TEAM_SEND);
    havit_ui_build_task_views(create_view(parent, HAVIT_UI_SCREEN_TASK_LIST),
                             create_view(parent, HAVIT_UI_SCREEN_TASK_DETAIL),
                             create_view(parent, HAVIT_UI_SCREEN_TASK_EDIT));

    const lv_color_t white = lv_color_white();
    create_label(s_views[HAVIT_UI_SCREEN_LOGIN2], "Conecta tu HAVIT", 88,
                 &havit_instrument_sans_22, white, LV_OPA_COVER);
    s_connection_summary = create_label(s_views[HAVIT_UI_SCREEN_LOGIN2], "Preparando conexión…", 142,
                 &havit_instrument_sans_15, white, (lv_opa_t)217);
    connection_button(s_views[HAVIT_UI_SCREEN_LOGIN2], "VER / INICIAR CONEXIÓN", 225);
    connection_button(s_views[HAVIT_UI_SCREEN_LOGIN2], "RESTABLECER WI-FI", 279);
    s_views[HAVIT_UI_SCREEN_SETUP_RESET] = create_view(parent, HAVIT_UI_SCREEN_SETUP_RESET);
    create_label(s_views[HAVIT_UI_SCREEN_SETUP_RESET], "Restablecer Wi-Fi", 92,
                 &havit_instrument_sans_22, white, LV_OPA_COVER);
    create_label(s_views[HAVIT_UI_SCREEN_SETUP_RESET],
                 "Se borrarán la red y contraseña.\nLos demás datos se conservan.", 152,
                 &havit_instrument_sans_15, white, LV_OPA_COVER);
    connection_button(s_views[HAVIT_UI_SCREEN_SETUP_RESET], "BORRAR Y CONFIGURAR", 225);
    connection_button(s_views[HAVIT_UI_SCREEN_SETUP_RESET], "CANCELAR", 279);
    create_label(s_views[HAVIT_UI_SCREEN_WIFI], "Configuración de Wifi", 157,
                 &havit_instrument_sans_22, white, (lv_opa_t)217);
    create_label(s_views[HAVIT_UI_SCREEN_WIFI], "Buscando red", 185,
                 &havit_instrument_sans_15, white, (lv_opa_t)128);
    create_label(s_views[HAVIT_UI_SCREEN_PIN], "Vinculación", 157,
                 &havit_instrument_sans_22, white, (lv_opa_t)217);
    create_label(s_views[HAVIT_UI_SCREEN_PIN], "Mostrar Pin", 185,
                 &havit_instrument_sans_15, white, (lv_opa_t)128);
    havit_ui_build_device_setup_views(
        s_views[HAVIT_UI_SCREEN_SETUP_BLE_READY],
        s_views[HAVIT_UI_SCREEN_SETUP_BLE_CONNECTED],
        s_views[HAVIT_UI_SCREEN_SETUP_WIFI_SCAN],
        s_views[HAVIT_UI_SCREEN_SETUP_WIFI_CONNECT],
        s_views[HAVIT_UI_SCREEN_SETUP_LINK_TOUCH],
        s_views[HAVIT_UI_SCREEN_SETUP_LINK_PIN],
        s_views[HAVIT_UI_SCREEN_SETUP_SYNC]);
    if(s_network_timer) lv_timer_delete(s_network_timer);
    s_network_revision = UINT32_MAX;
    s_network_timer = lv_timer_create(network_poll, 100, NULL);
    create_label(s_views[HAVIT_UI_SCREEN_WELCOME], "Bienvenido de nuevo", 144,
                 &havit_instrument_sans_22, white, (lv_opa_t)217);
    s_welcome_name = create_label(s_views[HAVIT_UI_SCREEN_WELCOME], "¡Hola!", 169,
                                  &havit_instrument_sans_28,
                                  lv_color_hex(0x77B7DB), LV_OPA_COVER);
    havit_ui_build_today_views(s_views[HAVIT_UI_SCREEN_TODAY], s_views[HAVIT_UI_SCREEN_ADD],
                               s_views[HAVIT_UI_SCREEN_CALENDAR],
                               s_views[HAVIT_UI_SCREEN_CALENDAR_DAY]);
    havit_ui_build_today_task_views(s_views[HAVIT_UI_SCREEN_TODAY_TASKS],
                                    s_views[HAVIT_UI_SCREEN_TODAY_TASKS_SELECT],
                                    s_views[HAVIT_UI_SCREEN_TODAY_TASK_DETAIL]);
    havit_ui_build_quick_task_views(s_views[HAVIT_UI_SCREEN_TASK_QUICK],
                                    s_views[HAVIT_UI_SCREEN_TASK_QUICK_RUNNING],
                                    s_views[HAVIT_UI_SCREEN_TASK_QUICK_DONE]);
    havit_ui_build_team_views(s_views[HAVIT_UI_SCREEN_TEAM_CREATE],
                              s_views[HAVIT_UI_SCREEN_TEAM_GENERATING],
                              s_views[HAVIT_UI_SCREEN_TEAM_TIMELINE],
                              s_views[HAVIT_UI_SCREEN_TEAM_BREAKDOWN],
                              s_views[HAVIT_UI_SCREEN_TEAM_SEND]);
}

void havit_ui_navigate_to(havit_ui_screen_t screen)
{
    if(s_transitioning || screen < HAVIT_UI_SCREEN_LOGIN1 || screen >= HAVIT_UI_SCREEN_COUNT ||
       screen == s_current_screen) return;

    lv_obj_t *source = s_views[s_current_screen];
    lv_obj_t *destination = s_views[screen];
    if(source == NULL || destination == NULL) return;
    if(screen == HAVIT_UI_SCREEN_TASK_QUICK)
        havit_ui_quick_task_reset();
    if(s_welcome_timer != NULL) {
        lv_timer_delete(s_welcome_timer);
        s_welcome_timer = NULL;
    }

    ESP_LOGI(TAG, "Navigate %d -> %d", (int)s_current_screen, (int)screen);
    havit_ui_screen_t previous_screen = s_current_screen;
    s_transitioning = true;
    s_current_screen = screen;
    /* Select the background for every route before choosing its transition.
     * Default 4 stays fixed above the scrolling Team form; all other screens
     * use the app ring (Default 2). */
    havit_default_background_set_scroll_active(
        screen == HAVIT_UI_SCREEN_TEAM_CREATE);
    havit_ui_team_screen_changed(screen);
    s_transition_source = source;
    s_transition_destination = destination;
    if(previous_screen == HAVIT_UI_SCREEN_WELCOME && screen == HAVIT_UI_SCREEN_TODAY) {
        animate_welcome_to_today();
        return;
    }
    if(is_today_task_screen(previous_screen) && is_today_task_screen(screen)) {
        animate_today_task_transition();
        return;
    }
    bool previous_is_quick = previous_screen >= HAVIT_UI_SCREEN_TASK_QUICK &&
                             previous_screen <= HAVIT_UI_SCREEN_TASK_QUICK_DONE;
    bool destination_is_quick = screen >= HAVIT_UI_SCREEN_TASK_QUICK &&
                                screen <= HAVIT_UI_SCREEN_TASK_QUICK_DONE;
    if((previous_screen == HAVIT_UI_SCREEN_ADD &&
        screen == HAVIT_UI_SCREEN_TASK_QUICK) ||
       (previous_is_quick && destination_is_quick) ||
       (previous_is_quick && screen == HAVIT_UI_SCREEN_ADD)) {
        animate_quick_task();
        return;
    }
    if(havit_ui_start_add_transition(previous_screen, screen, source_fade_finished)) return;
    animate_dissolve(source, destination);
}

void havit_ui_handle_tap(lv_obj_t *target)
{
    if(s_transitioning) return;
    if(havit_ui_login1_is_button(target)) {
        ESP_LOGI(TAG, "Login 1 continue button tapped; opening Login 2");
        havit_ui_navigate_to(HAVIT_UI_SCREEN_LOGIN2);
        return;
    }
    /* Setup input has one owner: the coordinate handler below. */
    if(s_current_screen >= HAVIT_UI_SCREEN_SETUP_BLE_READY &&
       s_current_screen <= HAVIT_UI_SCREEN_SETUP_RESET) return;
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
    else if(s_current_screen == HAVIT_UI_SCREEN_LOGIN2) {
        if(x >= 72 && x <= 288 && y >= 225 && y <= 265) {
            havit_connectivity_begin(false);
            havit_net_snapshot_t status;
            havit_connectivity_snapshot(&status);
            s_network_revision = UINT32_MAX;
            havit_ui_navigate_to(status.state == HAVIT_NET_ONLINE ?
                HAVIT_UI_SCREEN_SETUP_LINK_TOUCH : status.state == HAVIT_NET_CONNECTING ?
                HAVIT_UI_SCREEN_SETUP_WIFI_CONNECT : HAVIT_UI_SCREEN_SETUP_BLE_READY);
        } else if(x >= 72 && x <= 288 && y >= 279 && y <= 319) {
            s_reset_requested = false;
            havit_ui_navigate_to(HAVIT_UI_SCREEN_SETUP_RESET);
        }
    }
    else if(s_current_screen == HAVIT_UI_SCREEN_SETUP_RESET) {
        if(s_reset_requested) return;
        if(x >= 72 && x <= 288 && y >= 225 && y <= 265) {
            if(havit_connectivity_reset_wifi() == ESP_OK) {
                s_reset_requested = true;
            }
        } else if(x >= 72 && x <= 288 && y >= 279 && y <= 319) {
            havit_ui_navigate_to(HAVIT_UI_SCREEN_LOGIN2);
        }
    }
    else if(s_current_screen == HAVIT_UI_SCREEN_WIFI) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_PIN);
    }
    else if(s_current_screen == HAVIT_UI_SCREEN_PIN) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_WELCOME);
    }
    else if(s_current_screen >= HAVIT_UI_SCREEN_SETUP_BLE_READY &&
            s_current_screen <= HAVIT_UI_SCREEN_SETUP_RESET) {
        /* Waiting states advance only on genuine network events. */
        if(s_current_screen == HAVIT_UI_SCREEN_SETUP_LINK_TOUCH ||
           (x >= 80 && x <= 280 && y >= 260 && y <= 318)) {
            s_network_press_pending = true;
            s_network_press_screen = s_current_screen;
            s_network_release_start = 0;
        }
        return;
    }
    else if(is_today_task_screen(s_current_screen)) {
        havit_ui_today_task_handle_touch(s_current_screen, x, y);
    }
    else if(s_current_screen >= HAVIT_UI_SCREEN_TEAM_CREATE) {
        havit_ui_team_handle_touch(s_current_screen, x, y);
    }
    else if(s_current_screen >= HAVIT_UI_SCREEN_TASK_QUICK) {
        havit_ui_quick_task_handle_touch(s_current_screen, x, y);
    }
    else if(s_current_screen >= HAVIT_UI_SCREEN_TODAY) {
        havit_ui_today_handle_touch(s_current_screen, x, y);
    }
}
