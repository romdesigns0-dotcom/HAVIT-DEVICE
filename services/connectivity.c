#include "havit_connectivity.h"
#include "havit_device_claim.h"
#include "havit_tasks.h"
#include "havit_ble_relay.h"
#include "esp_heap_caps.h"
#include "mbedtls/platform.h"
#include "esp_event.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"
#include "protocomm_security.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "havit_net";
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
static havit_net_snapshot_t s_status = {.state = HAVIT_NET_STARTING};
static QueueHandle_t s_commands;
static SemaphoreHandle_t s_startup;
static SemaphoreHandle_t s_factory_reset_done;
static esp_err_t s_factory_reset_result=ESP_FAIL;
static bool s_manager;
static char s_pop[7]; /* Lifetime covers the complete protocomm session. */
enum { CMD_BEGIN, CMD_RECONFIGURE, CMD_RETRY, CMD_END, CMD_RESET_WIFI, CMD_API_REFRESH, CMD_BRIDGE_CONFIRMED, CMD_FACTORY_RESET, CMD_FINISH_FACTORY_RESET };

void havit_connectivity_snapshot(havit_net_snapshot_t *out)
{
    if(!out) return;
    portENTER_CRITICAL(&s_lock);
    *out = s_status;
    portEXIT_CRITICAL(&s_lock);
}

static void set_state(havit_net_state_t state, const char *message)
{
    portENTER_CRITICAL(&s_lock);
    s_status.state = state;
    snprintf(s_status.message, sizeof(s_status.message), "%s", message);
    ++s_status.revision;
    portEXIT_CRITICAL(&s_lock);
    ESP_LOGI(TAG, "State=%d: %s", state, message);
}

static esp_err_t post_command(int command)
{
    if(!s_commands) return ESP_ERR_INVALID_STATE;
    return xQueueSend(s_commands, &command, 0) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t havit_connectivity_begin(bool reconfigure)
{
    return post_command(reconfigure ? CMD_RECONFIGURE : CMD_BEGIN);
}
esp_err_t havit_connectivity_retry(void) { return post_command(CMD_RETRY); }
esp_err_t havit_connectivity_reset_wifi(void) { return post_command(CMD_RESET_WIFI); }
esp_err_t havit_connectivity_refresh_backend(void) { return post_command(CMD_API_REFRESH); }
esp_err_t havit_connectivity_bridge_confirmed(void) { return post_command(CMD_BRIDGE_CONFIRMED); }
esp_err_t havit_connectivity_factory_reset(void)
{
    if(!s_factory_reset_done)return ESP_ERR_INVALID_STATE;
    while(xSemaphoreTake(s_factory_reset_done,0)==pdTRUE){}
    esp_err_t err=post_command(CMD_FACTORY_RESET);
    if(err!=ESP_OK)return err;
    if(xSemaphoreTake(s_factory_reset_done,pdMS_TO_TICKS(15000))!=pdTRUE)return ESP_ERR_TIMEOUT;
    return s_factory_reset_result;
}
esp_err_t havit_connectivity_finish_factory_reset(void) { return post_command(CMD_FINISH_FACTORY_RESET); }

/* Identity/status only. This endpoint never claims a Firebase account. */
static esp_err_t info_endpoint(uint32_t session_id, const uint8_t *in, ssize_t len,
                                uint8_t **out, ssize_t *out_len, void *context)
{
    (void)session_id; (void)in; (void)len; (void)context;
    havit_net_snapshot_t status;
    havit_connectivity_snapshot(&status);
    cJSON *json = cJSON_CreateObject();
    if(!json) return ESP_ERR_NO_MEM;
    cJSON_AddNumberToObject(json, "protocol", 1);
    cJSON_AddStringToObject(json, "device_id", status.device_id);
    cJSON_AddStringToObject(json, "name", status.name);
    cJSON_AddBoolToObject(json, "wifi_connected", status.wifi_connected);
    cJSON_AddStringToObject(json, "ip", status.ip);
    cJSON_AddBoolToObject(json, "presence_confirmed", status.presence_confirmed);
    cJSON_AddBoolToObject(json, "account_linked", status.account_linked);
    cJSON_AddBoolToObject(json, "identity_provisioned", status.identity_provisioned);
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if(!response) return ESP_ERR_NO_MEM;
    *out = (uint8_t *)response;
    *out_len = strlen(response);
    return ESP_OK;
}

static void network_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    havit_net_snapshot_t status;
    havit_connectivity_snapshot(&status);
    if(base == WIFI_PROV_EVENT) {
        switch(id) {
        case WIFI_PROV_CRED_RECV:
            portENTER_CRITICAL(&s_lock);
            s_status.has_saved_network = true;
            portEXIT_CRITICAL(&s_lock);
            /* Never log SSID/password, PoP, or provisioning payloads. */
            set_state(HAVIT_NET_CONNECTING, "Credenciales recibidas; conectando a Wi-Fi");
            break;
        case WIFI_PROV_CRED_FAIL: {
            wifi_prov_sta_fail_reason_t reason = *(wifi_prov_sta_fail_reason_t *)data;
            set_state(HAVIT_NET_ERROR, reason == WIFI_PROV_STA_AUTH_ERROR ?
                      "Contraseña incorrecta. Reintenta desde la app." :
                      "No se encontró la red. Reintenta desde la app.");
            break;
        }
        case WIFI_PROV_END:
            portENTER_CRITICAL(&s_lock);
            s_status.provisioning = false;
            memset(s_status.pop, 0, sizeof(s_status.pop));
            ++s_status.revision;
            portEXIT_CRITICAL(&s_lock);
            post_command(CMD_END);
            break;
        default: break;
        }
    } else if(base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *event = data;
        portENTER_CRITICAL(&s_lock);
        s_status.wifi_connected = true;
        snprintf(s_status.ip, sizeof(s_status.ip), IPSTR, IP2STR(&event->ip_info.ip));
        portEXIT_CRITICAL(&s_lock);
        set_state(HAVIT_NET_ONLINE, "Wi-Fi conectado. La vinculación de cuenta está pendiente.");
        post_command(CMD_API_REFRESH);
    } else if(base == WIFI_EVENT) {
        if(id == WIFI_EVENT_STA_DISCONNECTED) {
            portENTER_CRITICAL(&s_lock);
            s_status.wifi_connected = false;
            s_status.ip[0] = 0;
            s_status.presence_confirmed = false;
            portEXIT_CRITICAL(&s_lock);
            /* The provisioning manager owns retries while provisioning. */
            if(!status.provisioning)
                set_state(HAVIT_NET_CONNECTING, "Reconectando a la red guardada…");
        } else if(id == WIFI_EVENT_SCAN_DONE && status.provisioning &&
                  !status.wifi_connected && status.state != HAVIT_NET_CONNECTING) {
            set_state(HAVIT_NET_SCAN, "Redes disponibles. Elige una en la app.");
        }
    } else if(base == PROTOCOMM_TRANSPORT_BLE_EVENT) {
        if(id == PROTOCOMM_TRANSPORT_BLE_DISCONNECTED && havit_ble_relay_active()) havit_ble_relay_disconnected();
        if(id == PROTOCOMM_TRANSPORT_BLE_CONNECTED && status.provisioning && !status.wifi_connected)
            set_state(HAVIT_NET_BLE_CONNECTED, "Introduce el código mostrado en HAVIT móvil.");
        else if(id == PROTOCOMM_TRANSPORT_BLE_DISCONNECTED && status.provisioning &&
                !status.wifi_connected && status.state != HAVIT_NET_CONNECTING &&
                status.state != HAVIT_NET_ERROR)
            set_state(HAVIT_NET_BLE_READY, "Esperando conexión desde HAVIT móvil");
    } else if(base == PROTOCOMM_SECURITY_SESSION_EVENT && status.provisioning && !status.wifi_connected) {
        if(id == PROTOCOMM_SECURITY_SESSION_SETUP_OK)
            set_state(HAVIT_NET_SECURE, "Canal cifrado listo. Busca redes desde la app.");
        else if(id == PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH ||
                id == PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS)
            set_state(HAVIT_NET_ERROR, "Código incorrecto. Reconecta e introduce el código de HAVIT.");
    }
}

static esp_err_t start_provisioning(void)
{
    havit_ble_relay_stop();
    esp_err_t err;
    err = havit_device_claim_begin_window();
    if(err != ESP_OK) return err;
    if(s_manager) {
        wifi_prov_mgr_deinit();
        s_manager = false;
    }
    const wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        /* Keep BLE memory available for a later explicit change of network. */
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
        .app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
        .wifi_prov_conn_cfg = {.wifi_conn_attempts = 3},
    };
    err = wifi_prov_mgr_init(config);
    if(err != ESP_OK) return err;
    s_manager = true;
    err = wifi_prov_mgr_endpoint_create("havit-info");
    if(err != ESP_OK) return err;
    err = wifi_prov_mgr_endpoint_create("havit-claim");
    if(err != ESP_OK) return err;
    /* Rejection sampling avoids biased six-digit codes. */
    uint32_t random;
    do { random = esp_random(); } while(random >= 4294000000U);
    snprintf(s_pop, sizeof(s_pop), "%06lu", (unsigned long)(random % 1000000));
    portENTER_CRITICAL(&s_lock);
    snprintf(s_status.pop, sizeof(s_status.pop), "%s", s_pop);
    s_status.provisioning = true;
    s_status.presence_confirmed = false;
    s_status.wifi_connected = false;
    s_status.ip[0] = 0;
    portEXIT_CRITICAL(&s_lock);
    havit_net_snapshot_t status;
    havit_connectivity_snapshot(&status);
    set_state(HAVIT_NET_BLE_READY, "Busca este nombre en HAVIT móvil");
    err = wifi_prov_mgr_start_provisioning(WIFI_PROV_SECURITY_1, s_pop, status.name, NULL);
    if(err != ESP_OK) return err;
    err = wifi_prov_mgr_endpoint_register("havit-info", info_endpoint, NULL);
    if(err != ESP_OK) return err;
    err = havit_device_claim_register_endpoint();
    if(err != ESP_OK) return err;
    return ESP_OK;
}

static void refresh_backend_status(void)
{
    esp_err_t err = havit_device_claim_refresh();
    havit_device_claim_snapshot_t claim;
    havit_device_claim_snapshot(&claim);
    portENTER_CRITICAL(&s_lock);
    s_status.identity_provisioned = claim.identity_provisioned;
    s_status.claim_acknowledged = claim.claim_acknowledged;
    s_status.claim_published = claim.claim_published;
    s_status.claim_expired = claim.claim_expired;
    s_status.device_credential_rejected = claim.device_credential_rejected;
    s_status.backend_waiting = claim.backend_waiting;
    s_status.retry_after_seconds = claim.retry_after_seconds;
    s_status.account_linked = claim.account_linked;
    snprintf(s_status.binding_id, sizeof(s_status.binding_id), "%s", claim.binding_id);
    snprintf(s_status.account_name, sizeof(s_status.account_name), "%s", claim.account_name);
    portEXIT_CRITICAL(&s_lock);
    if(!claim.identity_provisioned) {
        set_state(HAVIT_NET_ONLINE, "Wi-Fi conectado. La unidad requiere alta USB.");
    } else if(claim.device_credential_rejected ||
              (err != ESP_OK && claim.retry_after_seconds == 0)) {
        set_state(HAVIT_NET_BACKEND_ERROR, claim.message[0] ? claim.message :
                  "No se pudo consultar el backend. Reintenta.");
    } else if(claim.retry_after_seconds > 0 && !claim.backend_waiting) {
        set_state(HAVIT_NET_BACKEND_ERROR, claim.message[0] ? claim.message :
                  "El servidor pidió esperar antes de reintentar.");
    } else if(claim.claim_expired) {
        set_state(HAVIT_NET_LINK_PENDING, claim.message[0] ? claim.message :
                  "La sesión de vinculación venció. Inicia una nueva desde HAVIT móvil.");
    } else if(claim.backend_waiting || claim.retry_after_seconds > 0) {
        set_state(HAVIT_NET_LINK_PENDING, claim.message[0] ? claim.message :
                  "Esperando al servidor antes de reintentar.");
    } else if(claim.account_linked) {
        set_state(HAVIT_NET_LINKED, "Dispositivo vinculado a la cuenta.");
    } else {
        set_state(HAVIT_NET_LINK_PENDING, claim.message[0] ? claim.message :
                  "Esperando que HAVIT móvil vincule la cuenta.");
    }
}

static void copy_claim_status(void)
{
    havit_device_claim_snapshot_t claim;
    havit_device_claim_snapshot(&claim);
    portENTER_CRITICAL(&s_lock);
    s_status.identity_provisioned = claim.identity_provisioned;
    s_status.claim_acknowledged = claim.claim_acknowledged;
    s_status.claim_published = claim.claim_published;
    s_status.claim_expired = claim.claim_expired;
    s_status.device_credential_rejected = claim.device_credential_rejected;
    s_status.backend_waiting = claim.backend_waiting;
    s_status.retry_after_seconds = claim.retry_after_seconds;
    s_status.account_linked = claim.account_linked;
    snprintf(s_status.binding_id, sizeof(s_status.binding_id), "%s", claim.binding_id);
    snprintf(s_status.account_name, sizeof(s_status.account_name), "%s", claim.account_name);
    portEXIT_CRITICAL(&s_lock);
}

static esp_err_t initialize_network(void)
{
    /* Do not use the example's nvs_flash_erase recovery: preserve user data. */
    esp_err_t err = nvs_flash_init();
    if(err != ESP_OK) return err;
    err = esp_netif_init();
    if(err != ESP_OK) return err;
    err = esp_event_loop_create_default();
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;
    if(!esp_netif_create_default_wifi_sta()) return ESP_ERR_NO_MEM;
    havit_net_snapshot_t identity_status;
    havit_connectivity_snapshot(&identity_status);
    ESP_LOGI(TAG,"Radio RAM before BLE: free=%u largest=%u",(unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT),(unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
    ESP_RETURN_ON_ERROR(havit_device_claim_init(identity_status.device_id, identity_status.name),
                        TAG, "device identity init");
    copy_claim_status();
    /* Reserve the controller's contiguous internal buffers before Wi-Fi opens
     * its RX/TX pools. Starting BLE later can assert inside the S3 controller
     * even when plenty of external PSRAM remains free. */
    havit_connectivity_snapshot(&identity_status);
    if(identity_status.binding_id[0]) {
        ESP_RETURN_ON_ERROR(havit_ble_relay_start(), TAG, "initial BLE relay");
    }
    wifi_init_config_t wifi = WIFI_INIT_CONFIG_DEFAULT();
    ESP_LOGI(TAG,"Radio RAM before Wi-Fi: free=%u largest=%u",(unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT),(unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
    err = esp_wifi_init(&wifi);
    if(err != ESP_OK) return err;
    ESP_RETURN_ON_ERROR(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, network_event, NULL), TAG, "prov events");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, network_event, NULL), TAG, "wifi events");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, network_event, NULL), TAG, "ip events");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, network_event, NULL), TAG, "ble events");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, network_event, NULL), TAG, "security events");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "station mode");
    /* Active Wi-Fi supplies hardware entropy before creating the session PoP. */
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "wifi start");
    wifi_config_t saved = {0};
    ESP_RETURN_ON_ERROR(esp_wifi_get_config(WIFI_IF_STA, &saved), TAG, "saved config");
    if(saved.sta.ssid[0]) {
        portENTER_CRITICAL(&s_lock);
        s_status.has_saved_network = true;
        portEXIT_CRITICAL(&s_lock);
        set_state(HAVIT_NET_CONNECTING, "Conectando a la red guardada…");
        return esp_wifi_connect();
    }
    return start_provisioning();
}

static void *tls_calloc(size_t count, size_t size)
{
    return heap_caps_calloc(count, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static void connectivity_task(void *unused)
{
    (void)unused;
    /* LVGL and Wi-Fi need internal RAM. Keep certificate/TLS allocations in
     * this board's 8 MiB PSRAM using mbedTLS' supported allocator hook. Set
     * before provisioning/network starts; certificate verification is intact. */
    mbedtls_platform_set_calloc_free(tls_calloc, heap_caps_free);
    (void)havit_tasks_init();
    esp_err_t err = initialize_network();
    if(s_startup) xSemaphoreGive(s_startup);
    if(err != ESP_OK) {
        portENTER_CRITICAL(&s_lock);
        s_status.fatal_error = true;
        portEXIT_CRITICAL(&s_lock);
        set_state(HAVIT_NET_ERROR, "Error al iniciar la red. Revisa el registro USB; NVS conservada.");
        ESP_LOGE(TAG, "Network init: %s", esp_err_to_name(err));
        vTaskDelete(NULL);
        return;
    }
    int64_t session_start = esp_timer_get_time();
    int64_t last_retry = 0;
    int64_t last_backend_refresh = 0;
    for(;;) {
        int command = -1;
        if(xQueueReceive(s_commands, &command, pdMS_TO_TICKS(1000)) == pdTRUE) {
            havit_net_snapshot_t status;
            havit_connectivity_snapshot(&status);
            err = ESP_OK;
            if(command == CMD_END) {
                /* A stale END from a restarted session must not stop the new one. */
                if(s_manager && !status.provisioning) {
                    wifi_prov_mgr_deinit();
                    s_manager = false;
                    memset(s_pop, 0, sizeof(s_pop));
                }
            } else if(command == CMD_BRIDGE_CONFIRMED) {
                copy_claim_status();
                havit_connectivity_snapshot(&status);
                if(status.account_linked) set_state(HAVIT_NET_LINKED, "Dispositivo vinculado. Actividades por Bluetooth.");
            } else if(command == CMD_API_REFRESH) {
                if(status.wifi_connected) {
                    refresh_backend_status();
                    last_backend_refresh = esp_timer_get_time();
                }
            } else if(command == CMD_RESET_WIFI) {
                set_state(HAVIT_NET_STARTING, "Restableciendo solo la configuración Wi-Fi");
                if(s_manager) {
                    wifi_prov_mgr_deinit();
                    s_manager = false;
                }
                esp_wifi_disconnect();
                wifi_config_t empty = {0};
                err = esp_wifi_set_storage(WIFI_STORAGE_FLASH);
                if(err == ESP_OK) err = esp_wifi_set_config(WIFI_IF_STA, &empty);
                if(err == ESP_OK) {
                    portENTER_CRITICAL(&s_lock);
                    s_status.has_saved_network = false;
                    s_status.wifi_connected = false;
                    s_status.presence_confirmed = false;
                    s_status.ip[0] = 0;
                    portEXIT_CRITICAL(&s_lock);
                    err = start_provisioning();
                    session_start = esp_timer_get_time();
                }
            } else if(command == CMD_FACTORY_RESET) {
                set_state(HAVIT_NET_STARTING,"Restableciendo HAVIT de fábrica…");
                if(s_manager){wifi_prov_mgr_deinit();s_manager=false;}
                (void)esp_wifi_disconnect();
                wifi_config_t empty={0};
                err=esp_wifi_set_storage(WIFI_STORAGE_FLASH);
                if(err==ESP_OK)err=esp_wifi_set_config(WIFI_IF_STA,&empty);
                if(err==ESP_OK)err=havit_tasks_factory_reset();
                if(err==ESP_OK)err=havit_device_claim_factory_reset();
                if(err==ESP_OK){
                    portENTER_CRITICAL(&s_lock);
                    s_status.has_saved_network=false;s_status.wifi_connected=false;s_status.presence_confirmed=false;
                    s_status.account_linked=false;s_status.binding_id[0]=0;s_status.account_name[0]=0;s_status.ip[0]=0;
                    portEXIT_CRITICAL(&s_lock);
                }
                s_factory_reset_result=err;
                if(s_factory_reset_done)xSemaphoreGive(s_factory_reset_done);
                if(err==ESP_OK){
                    /* Keep BLE alive briefly so iOS can return the device proof to Firebase. */
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    havit_ble_relay_stop();
                    err=start_provisioning();session_start=esp_timer_get_time();
                }
            } else if(command == CMD_FINISH_FACTORY_RESET) {
                set_state(HAVIT_NET_STARTING,"Borrando la red Wi-Fi guardada");
                if(s_manager){wifi_prov_mgr_deinit();s_manager=false;}
                (void)esp_wifi_disconnect();
                wifi_config_t empty={0};
                err=esp_wifi_set_storage(WIFI_STORAGE_FLASH);
                if(err==ESP_OK)err=esp_wifi_set_config(WIFI_IF_STA,&empty);
                if(err==ESP_OK){
                    portENTER_CRITICAL(&s_lock);
                    s_status.has_saved_network=false;s_status.wifi_connected=false;s_status.presence_confirmed=false;
                    s_status.account_linked=false;s_status.binding_id[0]=0;s_status.account_name[0]=0;s_status.ip[0]=0;
                    portEXIT_CRITICAL(&s_lock);
                    havit_ble_relay_stop();
                    err=start_provisioning();session_start=esp_timer_get_time();
                }
            } else if(command == CMD_RETRY && status.provisioning && s_manager) {
                err = wifi_prov_mgr_reset_sm_state_on_failure();
                if(err == ESP_OK)
                    set_state(HAVIT_NET_SECURE, "Reintenta la red y contraseña desde la app.");
            } else if(command == CMD_RETRY && !status.provisioning && status.has_saved_network) {
                set_state(HAVIT_NET_CONNECTING, "Reintentando la red guardada");
                err = esp_wifi_connect();
            } else if(command == CMD_RECONFIGURE ||
                      (command == CMD_BEGIN && !status.provisioning &&
                       !status.wifi_connected && !status.has_saved_network)) {
                err = start_provisioning();
                session_start = esp_timer_get_time();
            }
            if(err != ESP_OK) {
                ESP_LOGE(TAG, "Provisioning action: %s", esp_err_to_name(err));
                set_state(HAVIT_NET_ERROR, "No se pudo configurar. Vuelve y restablece Wi-Fi.");
                err = ESP_OK;
            }
        }
        havit_net_snapshot_t status;
        havit_connectivity_snapshot(&status);
        int64_t now = esp_timer_get_time();
        if(!s_manager && status.binding_id[0] && !havit_ble_relay_active()) {
            esp_err_t relay_error = havit_ble_relay_start();
            if(relay_error != ESP_OK) ESP_LOGW(TAG,"BLE relay start: %s",esp_err_to_name(relay_error));
        } else if(!status.binding_id[0] && havit_ble_relay_active()) havit_ble_relay_stop();
        if(status.wifi_connected && now - last_backend_refresh >= 15000000LL) {
            refresh_backend_status();
            last_backend_refresh = esp_timer_get_time();
        }
        havit_tasks_tick(status.wifi_connected);
        if(status.provisioning && now - session_start >= 600000000LL) {
            wifi_prov_mgr_stop_provisioning();
            if(!status.wifi_connected)
                set_state(HAVIT_NET_ERROR, "Sesión caducada. Pulsa REINTENTAR para abrir Bluetooth.");
            session_start = now;
        }
        if(!status.provisioning && !status.wifi_connected && status.has_saved_network &&
           (status.state == HAVIT_NET_CONNECTING || status.state == HAVIT_NET_LINKED) && now - last_retry >= 5000000) {
            esp_wifi_connect();
            last_retry = now;
        }
    }
}

esp_err_t havit_connectivity_init(void)
{
    if(s_commands) return ESP_OK;
    uint8_t mac[6];
    esp_err_t err = esp_read_mac(mac, ESP_MAC_BT);
    if(err != ESP_OK) return err;
    snprintf(s_status.name, sizeof(s_status.name), "HAVIT-%02X%02X", mac[4], mac[5]);
    snprintf(s_status.device_id, sizeof(s_status.device_id), "havit-%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "Device identity: %s (%s)", s_status.device_id, s_status.name);
    s_commands = xQueueCreate(8, sizeof(int));
    if(!s_commands) return ESP_ERR_NO_MEM;
    s_startup = xSemaphoreCreateBinary();
    if(!s_startup) { vQueueDelete(s_commands); s_commands=NULL; return ESP_ERR_NO_MEM; }
    s_factory_reset_done=xSemaphoreCreateBinary();
    if(!s_factory_reset_done){vQueueDelete(s_commands);vSemaphoreDelete(s_startup);s_startup=NULL;s_commands=NULL;return ESP_ERR_NO_MEM;}
    /* HTTPS certificate validation and the BLE/Wi-Fi manager share this worker. */
    if(xTaskCreate(connectivity_task, "havit_network", 10240, NULL, 4, NULL) != pdPASS) {
        vQueueDelete(s_commands);
        vSemaphoreDelete(s_startup); s_startup=NULL;
        vSemaphoreDelete(s_factory_reset_done);s_factory_reset_done=NULL;
        s_commands = NULL;
        return ESP_ERR_NO_MEM;
    }
    /* Radio pools need contiguous internal memory. Finish driver allocation
     * before the caller starts LVGL/display objects. Wi-Fi association itself
     * remains asynchronous and does not block the screen on Internet access. */
    if(xSemaphoreTake(s_startup,pdMS_TO_TICKS(5000))!=pdTRUE)
        ESP_LOGW(TAG,"Radio startup still in progress; continuing UI startup");
    return ESP_OK;
}
