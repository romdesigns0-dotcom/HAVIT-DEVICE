#include "havit_ui.h"

#include "esp_log.h"
#include "esp_mac.h"
#include "lvgl.h"
#include <stdio.h>

extern const lv_font_t havit_instrument_sans_15;
extern const lv_font_t havit_instrument_sans_17;
extern const lv_font_t havit_instrument_sans_21;
extern const lv_font_t havit_instrument_sans_22;
extern const lv_font_t havit_instrument_sans_20;
extern const lv_font_t havit_instrument_sans_12;

static const char *TAG = "havit_setup_ui";
static char s_device_name[16];
static lv_obj_t *s_pin_label;
static lv_obj_t *s_titles[7], *s_status_labels[7], *s_bodies[7], *s_actions[7];
static unsigned s_view_index;

static lv_obj_t *centered_label(lv_obj_t *parent, const char *value,
                                int32_t x, int32_t y, int32_t width,
                                int32_t height, const lv_font_t *font,
                                lv_opa_t opacity)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, value);
    lv_obj_set_width(label, width);
    lv_obj_set_height(label, height);
    lv_obj_set_pos(label, x, y);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, opacity, LV_PART_MAIN);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return label;
}

static void setup_screen(lv_obj_t *view, const char *title, const char *status,
                         const char *body, const char *action)
{
    if(view == NULL) return;

    /* Keep this view transparent so the shared black Default 2 + ring remains visible. */
    lv_obj_remove_style_all(view);
    lv_obj_set_size(view, 360, 360);
    lv_obj_set_pos(view, 0, 0);
    lv_obj_clear_flag(view, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(view, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_flag(view, LV_OBJ_FLAG_HIDDEN);

    const unsigned index = s_view_index++;
    s_titles[index] = centered_label(view, title, 30, 70, 300, 34,
                                     &havit_instrument_sans_22, (lv_opa_t)219);
    s_status_labels[index] = centered_label(view, status, 35, 104, 290, 24,
                                             &havit_instrument_sans_15, (lv_opa_t)140);
    s_bodies[index] = centered_label(view, body, 32, 157, 296, 74,
                                      &havit_instrument_sans_21, (lv_opa_t)230);

    if(action != NULL && action[0] != '\0') {
        lv_obj_t *pill = lv_obj_create(view);
        lv_obj_remove_style_all(pill);
        lv_obj_set_size(pill, 200, 48);
        lv_obj_set_pos(pill, 80, 270);
        lv_obj_set_style_radius(pill, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_color(pill, lv_color_hex(0x73979A), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(pill, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t *action_label = lv_label_create(pill);
        lv_label_set_text(action_label, action);
        lv_obj_set_size(action_label, 180, 32);
        lv_label_set_long_mode(action_label, LV_LABEL_LONG_WRAP);
        lv_obj_center(action_label);
        lv_obj_set_style_text_align(action_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_font(action_label, &havit_instrument_sans_17, LV_PART_MAIN);
        lv_obj_set_style_text_color(action_label, lv_color_white(), LV_PART_MAIN);
        lv_obj_clear_flag(action_label, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        s_actions[index] = action_label;
    }
}

static void update_view(unsigned index, const char *title, const char *status,
                         const char *body, const char *action)
{
    lv_label_set_text(s_titles[index], title);
    lv_label_set_text(s_status_labels[index], status);
    lv_label_set_text(s_bodies[index], body);
    if(s_actions[index] != NULL && action != NULL) lv_label_set_text(s_actions[index], action);
    /* Fixed bands prevent one label from drawing underneath another. Size
     * text against the actual generated Instrument Sans glyph metrics. */
    lv_point_t size;
    lv_text_get_size(&size, body, &havit_instrument_sans_21, 0, 0, 296, LV_TEXT_FLAG_NONE);
    lv_obj_set_style_text_font(s_bodies[index], size.y > 62 ?
                              &havit_instrument_sans_15 : &havit_instrument_sans_21, 0);
    lv_text_get_size(&size, status, &havit_instrument_sans_15, 0, 0, 290, LV_TEXT_FLAG_NONE);
    lv_obj_set_style_text_font(s_status_labels[index], size.y > 24 ?
                              &havit_instrument_sans_12 : &havit_instrument_sans_15, 0);
}

void havit_ui_device_setup_update(const havit_net_snapshot_t *status)
{
    char text[128];
    snprintf(text, sizeof(text), "%s\nBusca este nombre en la app", status->name);
    update_view(0, "Bluetooth disponible", "Configura HAVIT desde tu móvil", text, "ESPERANDO APP");
    update_view(1, status->state == HAVIT_NET_BLE_CONNECTED ? "Bluetooth conectado" : "Bluetooth seguro",
                status->state == HAVIT_NET_BLE_CONNECTED ? "Falta verificar el código" : "Canal cifrado establecido",
                status->state == HAVIT_NET_BLE_CONNECTED ? text :
                "Busca las redes Wi-Fi desde la app.\nElige una red de 2.4 GHz.", "CONTINÚA EN LA APP");
    update_view(2, "Buscando redes Wi-Fi", "Escaneo solicitado desde la app",
                "Las redes aparecerán en el móvil.\nElige una e introduce su contraseña.", "CONTINÚA EN LA APP");
    update_view(3, status->state == HAVIT_NET_ERROR ? "Revisa la conexión" : "Conectando a Wi-Fi",
                status->state == HAVIT_NET_ERROR ? "Configuración pendiente" : "Esperando dirección IP",
                status->message, status->state == HAVIT_NET_ERROR ? "REINTENTAR" : "CONECTANDO");
    if(!status->identity_provisioned) {
        snprintf(text, sizeof(text), "IP: %s\nEsta unidad requiere el alta USB segura.", status->ip);
        update_view(4, "Wi-Fi conectado", "Unidad no registrada en backend", text, "ALTA PENDIENTE");
    } else if(status->claim_expired) {
        snprintf(text, sizeof(text), "IP: %s\n%s", status->ip, status->message);
        update_view(4, "Sesión vencida", "Inicia una nueva vinculación", text, "NUEVA VINCULACIÓN");
    } else if(status->state == HAVIT_NET_BACKEND_ERROR) {
        snprintf(text, sizeof(text), "IP: %s\n%s", status->ip, status->message);
        update_view(4, "Vinculación pendiente", "No se confirmó el backend", text, "REINTENTAR");
    } else if(status->backend_waiting) {
        snprintf(text, sizeof(text), "IP: %s\n%s", status->ip, status->message);
        update_view(4, "Esperando al servidor", "La vinculación sigue pendiente", text, "ESPERANDO");
    } else {
        snprintf(text, sizeof(text), "IP: %s\nAbre HAVIT móvil e inicia sesión\npara vincular este dispositivo.", status->ip);
        update_view(4, "Vinculación segura", "Confirma desde tu cuenta HAVIT",
                    "Toca la pantalla del dispositivo\npara confirmar que estás aquí.", "");
    }
    if(status->account_linked) {
        update_view(6, "Dispositivo vinculado", "Cuenta HAVIT conectada",
                    "Preparando tareas y calendario…", "SINCRONIZANDO");
    } else if(status->claim_expired) {
        update_view(6, "Sesión vencida", "La prueba de vinculación expiró",
                    "Vuelve a iniciar la vinculación\ndesde HAVIT móvil.", "NUEVA VINCULACIÓN");
    } else if(status->device_credential_rejected) {
        update_view(6, "Alta USB requerida", "Credencial de unidad no válida",
                    "Solicita al operador revisar\nel registro de este dispositivo.", "ALTA PENDIENTE");
    } else if(!status->identity_provisioned) {
        update_view(6, "Alta del dispositivo", "Vinculación no disponible",
                    "Falta registrar esta unidad\ncon una credencial individual.", "ALTA PENDIENTE");
    } else {
        update_view(6, "Vinculación pendiente", "Esperando a HAVIT móvil",
                    "Inicia sesión en la app para\nconfirmar esta unidad.", "ESPERANDO APP");
    }
    if(status->state == HAVIT_NET_STARTING)
        update_view(0, "Preparando conexión", "Inicializando Wi-Fi y Bluetooth",
                    "Espera unos segundos…", "PREPARANDO");
    if(status->fatal_error)
        update_view(3, "Error de conexión", "NVS conservada", status->message, "REVISA USB");
}

const char *havit_ui_device_setup_name(void)
{
    if(s_device_name[0] == '\0') {
        uint8_t address[6] = {0};
        if(esp_read_mac(address, ESP_MAC_BT) == ESP_OK) {
            snprintf(s_device_name, sizeof(s_device_name), "HAVIT-%02X%02X",
                     address[4], address[5]);
        } else {
            snprintf(s_device_name, sizeof(s_device_name), "HAVIT");
            ESP_LOGW(TAG, "Could not read Bluetooth MAC; using fallback name HAVIT");
        }
    }
    return s_device_name;
}

void havit_ui_device_setup_set_pin(const char *pin)
{
    if(s_pin_label == NULL || pin == NULL || pin[0] == '\0') return;
    lv_label_set_text(s_pin_label, pin);
}

void havit_ui_build_device_setup_views(lv_obj_t *ble_ready,
                                       lv_obj_t *ble_connected,
                                       lv_obj_t *wifi_scan,
                                       lv_obj_t *wifi_connect,
                                       lv_obj_t *link_touch,
                                       lv_obj_t *link_pin,
                                       lv_obj_t *sync)
{
    s_view_index = 0;
    char ready_body[64];
    snprintf(ready_body, sizeof(ready_body), "%s\nBusca este nombre en la app",
             havit_ui_device_setup_name());
    setup_screen(ble_ready, "Bluetooth disponible", "Esperando conexión segura",
                 ready_body, "LISTO PARA VINCULAR");
    setup_screen(ble_connected, "Conexión Bluetooth", "Canal seguro establecido",
                 "La app ya está conectada.\nAhora puedes configurar Wi-Fi.",
                 "BLUETOOTH ACTIVO");
    setup_screen(wifi_scan, "Buscando redes Wi-Fi", "Solicitud recibida desde HAVIT móvil",
                 "Escaneando redes cercanas…\nEsto puede tardar unos segundos.",
                 "BUSCANDO REDES");
    setup_screen(wifi_connect, "Conectando a Wi-Fi",
                 "Credenciales enviadas de forma segura por Bluetooth",
                 "HAVIT intenta conectarse\na la red Wi-Fi", "CONECTANDO");
    setup_screen(link_touch, "Vinculación segura", "Confirma desde tu cuenta HAVIT",
                 "Toca la pantalla del dispositivo\npara confirmar que estás aquí", NULL);
    setup_screen(link_pin, "Confirmar con PIN", "Introduce el código en HAVIT móvil",
                 "PIN temporal", NULL);

    s_pin_label = lv_label_create(link_pin);
    lv_label_set_text(s_pin_label, "······");
    lv_obj_set_width(s_pin_label, 296);
    lv_obj_set_pos(s_pin_label, 32, 203);
    lv_obj_set_height(s_pin_label, 28);
    lv_obj_set_style_text_align(s_pin_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(s_pin_label, &havit_instrument_sans_21, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_pin_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_clear_flag(s_pin_label, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    setup_screen(sync, "Vinculación pendiente", "Esperando confirmación del servidor",
                 "Las actividades aún no se han\nsincronizado con este dispositivo.",
                 "ESPERANDO");
    ESP_LOGI(TAG, "Device setup views built; BLE name=%s", havit_ui_device_setup_name());
}
