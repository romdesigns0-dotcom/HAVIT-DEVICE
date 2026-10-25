#include "havit_device_claim.h"

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_netif_sntp.h"
#include "mbedtls/sha256.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi_provisioning/manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#define IDENTITY_PARTITION "havit_identity"
#define IDENTITY_NAMESPACE "havit_identity"
#define CLAIM_NAMESPACE "havit_claim"
#define API_BASE_URL "https://us-central1-havit-c8c1d.cloudfunctions.net/havitDeviceApiV1"
#define CLAIM_LIFETIME_US (1800LL * 1000000LL)
#define HTTP_RESPONSE_CAPACITY 768

static const char *TAG = "havit_claim";
static SemaphoreHandle_t s_mutex;
static havit_device_claim_snapshot_t s_status;
static char s_device_token[96];
static char s_secret_b64[48];
static uint8_t s_secret_raw[32];
static char s_session_id[37];
static char s_secret_hash[65];
static char s_claim_expires_at[40];
static int64_t s_session_deadline_us;
static int64_t s_retry_not_before_us;
static int64_t s_retry_not_before_epoch;
static bool s_retry_is_waiting;
static bool s_credential_rejected_this_boot;
static bool s_ram_proof_ready;
static bool s_sntp_started;
bool havit_device_claim_confirm_bridge(const char *binding_id)
{
    if(!s_mutex || !binding_id || !binding_id[0]) return false;
    xSemaphoreTake(s_mutex,portMAX_DELAY);
    bool valid = s_status.identity_provisioned && strcmp(s_status.binding_id,binding_id)==0;
    if(valid) { s_status.account_linked=true; s_status.backend_waiting=false; }
    xSemaphoreGive(s_mutex);
    return valid;
}
esp_err_t havit_device_claim_factory_reset(void)
{
    if(!s_mutex || !s_status.identity_provisioned) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_mutex,portMAX_DELAY);
    nvs_handle_t handle;
    esp_err_t err=nvs_open_from_partition(IDENTITY_PARTITION,CLAIM_NAMESPACE,NVS_READWRITE,&handle);
    if(err==ESP_ERR_NVS_NOT_FOUND) err=ESP_OK;
    else if(err==ESP_OK){err=nvs_erase_all(handle);if(err==ESP_OK)err=nvs_commit(handle);nvs_close(handle);}
    if(err==ESP_OK){
        s_status.claim_acknowledged=false;s_status.claim_published=false;s_status.claim_expired=false;
        s_status.device_credential_rejected=false;s_status.backend_waiting=false;s_status.retry_after_seconds=0;
        s_status.account_linked=false;s_status.binding_id[0]=0;s_status.account_name[0]=0;s_status.message[0]=0;
        memset(s_secret_b64,0,sizeof(s_secret_b64));memset(s_secret_raw,0,sizeof(s_secret_raw));
        memset(s_session_id,0,sizeof(s_session_id));memset(s_secret_hash,0,sizeof(s_secret_hash));
        memset(s_claim_expires_at,0,sizeof(s_claim_expires_at));s_session_deadline_us=0;
        s_retry_not_before_us=0;s_retry_not_before_epoch=0;s_retry_is_waiting=false;s_ram_proof_ready=false;
    }
    xSemaphoreGive(s_mutex);
    return err;
}
esp_err_t havit_device_relay_key(uint8_t out[32])
{
    if(!s_mutex || !out) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if(!s_device_token[0]) { xSemaphoreGive(s_mutex); return ESP_ERR_INVALID_STATE; }
    uint8_t hash[32]; char text[80] = "havit-relay-v1:";
    int err = mbedtls_sha256((const unsigned char *)s_device_token, strlen(s_device_token), hash, 0);
    xSemaphoreGive(s_mutex);
    for(int i=0;i<32;i++) snprintf(text+15+i*2, 3, "%02x", hash[i]);
    if(!err) err = mbedtls_sha256((const unsigned char *)text, strlen(text), out, 0);
    memset(hash,0,sizeof(hash)); memset(text,0,sizeof(text));
    return err ? ESP_FAIL : ESP_OK;
}
static bool published_claim_expired(void);

typedef struct {
    char data[HTTP_RESPONSE_CAPACITY];
    char retry_after[16];
    size_t length;
    bool overflow;
} http_response_t;

static bool constant_time_equal(const char *a, const char *b, size_t length)
{
    uint8_t diff = 0;
    for(size_t i = 0; i < length; ++i) diff |= (uint8_t)a[i] ^ (uint8_t)b[i];
    return diff == 0;
}

static void wipe_secret(void)
{
    volatile uint8_t *bytes = s_secret_raw;
    for(size_t i = 0; i < sizeof(s_secret_raw); ++i) bytes[i] = 0;
    volatile char *encoded = s_secret_b64;
    for(size_t i = 0; i < sizeof(s_secret_b64); ++i) encoded[i] = 0;
    s_ram_proof_ready = false;
}

static esp_err_t load_nvs_string(nvs_handle_t handle, const char *key,
                                 char *out, size_t capacity)
{
    size_t length = capacity;
    esp_err_t err = nvs_get_str(handle, key, out, &length);
    if(err != ESP_OK) return err;
    if(length == 0 || length > capacity || out[length - 1] != '\0') return ESP_ERR_INVALID_SIZE;
    return ESP_OK;
}

static bool valid_device_token(const char *token)
{
    size_t length = strlen(token);
    static const char prefix[] = "hvt_dev_";
    if(length != sizeof(prefix) - 1 + 43 || length >= sizeof(s_device_token) ||
       strncmp(token, prefix, sizeof(prefix) - 1) != 0) return false;
    for(size_t i = sizeof(prefix) - 1; i < length; ++i) {
        char c = token[i];
        if(!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
             (c >= '0' && c <= '9') || c == '-' || c == '_')) return false;
    }
    return true;
}

static bool load_claim_state(void)
{
    nvs_handle_t handle;
    if(nvs_open_from_partition(IDENTITY_PARTITION, CLAIM_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
        return false;
    char session_id[sizeof(s_session_id)] = {0};
    char secret_hash[sizeof(s_secret_hash)] = {0};
    char binding[sizeof(s_status.binding_id)] = {0};
    char expires_at[sizeof(s_claim_expires_at)] = {0};
    uint8_t acknowledged = 0;
    uint8_t published = 0;
    uint8_t expired = 0;
    uint8_t retry_waiting = 0;
    uint64_t retry_until_epoch = 0;
    size_t sid_size = sizeof(session_id), hash_size = sizeof(secret_hash);
    size_t binding_size = sizeof(binding), expiry_size = sizeof(expires_at);
    (void)nvs_get_str(handle, "claim_sid", session_id, &sid_size);
    (void)nvs_get_str(handle, "claim_hash", secret_hash, &hash_size);
    (void)nvs_get_u8(handle, "claim_ack", &acknowledged);
    (void)nvs_get_u8(handle, "claim_pub", &published);
    (void)nvs_get_str(handle, "binding_id", binding, &binding_size);
    (void)nvs_get_str(handle, "claim_until", expires_at, &expiry_size);
    (void)nvs_get_u8(handle, "claim_expired", &expired);
    (void)nvs_get_u64(handle, "retry_until", &retry_until_epoch);
    (void)nvs_get_u8(handle, "retry_waiting", &retry_waiting);
    nvs_close(handle);
    if(expired == 1) {
        s_status.claim_expired = true;
    } else if(acknowledged == 1 && strlen(session_id) == 36 && strlen(secret_hash) == 64) {
        snprintf(s_session_id, sizeof(s_session_id), "%s", session_id);
        snprintf(s_secret_hash, sizeof(s_secret_hash), "%s", secret_hash);
        s_status.claim_acknowledged = true;
        s_status.claim_published = published == 1;
        if(s_status.claim_published && expires_at[0])
            snprintf(s_claim_expires_at, sizeof(s_claim_expires_at), "%s", expires_at);
    }
    if(binding[0] && strlen(binding) < sizeof(s_status.binding_id))
        snprintf(s_status.binding_id, sizeof(s_status.binding_id), "%s", binding);
    /* A cached binding is retained, but is not shown as linked until HTTPS confirms it. */
    s_status.account_linked = false;
    if(retry_until_epoch > 0) {
        s_retry_not_before_epoch = (int64_t)retry_until_epoch;
        s_retry_is_waiting = retry_waiting == 1;
    }
    return s_status.claim_acknowledged || s_status.binding_id[0] != '\0' || s_status.claim_expired;
}

static esp_err_t persist_ack(const char *session_id, const char *secret_hash)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(IDENTITY_PARTITION, CLAIM_NAMESPACE,
                                            NVS_READWRITE, &handle);
    if(err != ESP_OK) return err;
    err = nvs_set_str(handle, "claim_sid", session_id);
    if(err == ESP_OK) err = nvs_set_str(handle, "claim_hash", secret_hash);
    if(err == ESP_OK) err = nvs_set_u8(handle, "claim_pub", 0);
    if(err == ESP_OK) err = nvs_set_u8(handle, "claim_ack", 1);
    if(err == ESP_OK) err = nvs_set_u8(handle, "claim_expired", 0);
    if(err == ESP_OK) {
        esp_err_t erase_err = nvs_erase_key(handle, "claim_until");
        if(erase_err != ESP_OK && erase_err != ESP_ERR_NVS_NOT_FOUND) err = erase_err;
    }
    if(err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

static esp_err_t persist_published(const char *expires_at)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(IDENTITY_PARTITION, CLAIM_NAMESPACE,
                                            NVS_READWRITE, &handle);
    if(err != ESP_OK) return err;
    err = nvs_set_str(handle, "claim_until", expires_at);
    if(err == ESP_OK) err = nvs_set_u8(handle, "claim_pub", 1);
    if(err == ESP_OK) err = nvs_set_u8(handle, "claim_expired", 0);
    if(err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

static esp_err_t persist_link(const char *binding_id, bool linked)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(IDENTITY_PARTITION, CLAIM_NAMESPACE,
                                            NVS_READWRITE, &handle);
    if(err != ESP_OK) return err;
    if(linked) err = nvs_set_str(handle, "binding_id", binding_id);
    else err = nvs_erase_key(handle, "binding_id");
    if(err == ESP_ERR_NVS_NOT_FOUND && !linked) err = ESP_OK;
    if(err == ESP_OK) err = nvs_set_u8(handle, "linked", linked ? 1 : 0);
    if(err == ESP_OK && linked) err = nvs_set_u8(handle, "claim_expired", 0);
    if(err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

static esp_err_t persist_retry_until(uint64_t unix_seconds, bool backend_waiting)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(IDENTITY_PARTITION, CLAIM_NAMESPACE,
                                            NVS_READWRITE, &handle);
    if(err != ESP_OK) return err;
    if(unix_seconds) {
        err = nvs_set_u64(handle, "retry_until", unix_seconds);
        if(err == ESP_OK) err = nvs_set_u8(handle, "retry_waiting", backend_waiting ? 1 : 0);
    }
    else {
        err = nvs_erase_key(handle, "retry_until");
        if(err == ESP_ERR_NVS_NOT_FOUND) err = ESP_OK;
        if(err == ESP_OK) {
            esp_err_t erase_err = nvs_erase_key(handle, "retry_waiting");
            if(erase_err != ESP_OK && erase_err != ESP_ERR_NVS_NOT_FOUND) err = erase_err;
        }
    }
    if(err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

static esp_err_t clear_pending_claim(bool expired)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open_from_partition(IDENTITY_PARTITION, CLAIM_NAMESPACE,
                                            NVS_READWRITE, &handle);
    if(err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if(err != ESP_OK) return err;
    static const char *keys[] = {"claim_sid", "claim_hash", "claim_ack", "claim_pub", "claim_until"};
    for(size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        esp_err_t erase_err = nvs_erase_key(handle, keys[i]);
        if(erase_err != ESP_OK && erase_err != ESP_ERR_NVS_NOT_FOUND) {
            err = erase_err;
            break;
        }
    }
    if(err == ESP_OK) err = nvs_set_u8(handle, "claim_expired", expired ? 1 : 0);
    if(err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    if(err == ESP_OK) {
        wipe_secret();
        memset(s_session_id, 0, sizeof(s_session_id));
        memset(s_secret_hash, 0, sizeof(s_secret_hash));
        memset(s_claim_expires_at, 0, sizeof(s_claim_expires_at));
        s_session_deadline_us = 0;
        s_status.claim_acknowledged = false;
        s_status.claim_published = false;
        s_status.claim_expired = expired;
    }
    return err;
}

esp_err_t havit_device_claim_begin_window(void)
{
    if(!s_mutex || !s_status.identity_provisioned) return ESP_OK;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if(s_status.account_linked || s_status.binding_id[0] != '\0') {
        xSemaphoreGive(s_mutex);
        return ESP_OK;
    }
    esp_err_t err = clear_pending_claim(false);
    if(err == ESP_OK) {
        snprintf(s_status.message, sizeof(s_status.message), "Nueva ventana de vinculación abierta");
    }
    xSemaphoreGive(s_mutex);
    return err;
}

static void make_session_id(char out[37])
{
    uint8_t bytes[16];
    esp_fill_random(bytes, sizeof(bytes));
    bytes[6] = (uint8_t)((bytes[6] & 0x0f) | 0x40); /* UUID v4 */
    bytes[8] = (uint8_t)((bytes[8] & 0x3f) | 0x80); /* RFC 4122 variant */
    snprintf(out, 37,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7],
             bytes[8], bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
}

static void encode_base64url(const uint8_t input[32], char output[48])
{
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t in = 0, out = 0;
    while(in + 3 <= 32) {
        uint32_t value = ((uint32_t)input[in] << 16) |
                         ((uint32_t)input[in + 1] << 8) | input[in + 2];
        output[out++] = alphabet[(value >> 18) & 63];
        output[out++] = alphabet[(value >> 12) & 63];
        output[out++] = alphabet[(value >> 6) & 63];
        output[out++] = alphabet[value & 63];
        in += 3;
    }
    uint32_t value = (uint32_t)input[in] << 16 | (uint32_t)input[in + 1] << 8;
    output[out++] = alphabet[(value >> 18) & 63];
    output[out++] = alphabet[(value >> 12) & 63];
    output[out++] = alphabet[(value >> 6) & 63];
    output[out] = '\0'; /* 32 bytes encode to 43 unpadded base64url characters. */
}

static esp_err_t calculate_proof_hash(const char *device_id, const char *session_id,
                                      const uint8_t secret[32], char out[65])
{
    static const char prefix[] = "havit-claim-v1:";
    uint8_t input[sizeof(prefix) - 1 + 24 + 1 + 36 + 1 + 32];
    size_t offset = 0;
    memcpy(input + offset, prefix, sizeof(prefix) - 1); offset += sizeof(prefix) - 1;
    size_t id_length = strlen(device_id);
    size_t session_length = strlen(session_id);
    if(id_length >= 24 || session_length != 36) return ESP_ERR_INVALID_ARG;
    memcpy(input + offset, device_id, id_length); offset += id_length;
    input[offset++] = 0;
    memcpy(input + offset, session_id, session_length); offset += session_length;
    input[offset++] = 0;
    memcpy(input + offset, secret, 32); offset += 32;
    uint8_t digest[32];
    esp_err_t err = mbedtls_sha256(input, offset, digest, 0);
    if(err != 0) return ESP_FAIL;
    for(size_t i = 0; i < sizeof(digest); ++i) snprintf(out + i * 2, 3, "%02x", digest[i]);
    out[64] = '\0';
    memset(digest, 0, sizeof(digest));
    memset(input, 0, sizeof(input));
    return ESP_OK;
}

static esp_err_t send_json(uint8_t **out, ssize_t *out_len, cJSON *json)
{
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if(!response) return ESP_ERR_NO_MEM;
    *out = (uint8_t *)response;
    *out_len = (ssize_t)strlen(response);
    return ESP_OK;
}

static esp_err_t send_error(uint8_t **out, ssize_t *out_len, const char *code)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *error = cJSON_CreateObject();
    if(!root || !error) {
        cJSON_Delete(root);
        cJSON_Delete(error);
        return ESP_ERR_NO_MEM;
    }
    cJSON_AddNumberToObject(root, "protocol", 1);
    cJSON_AddStringToObject(error, "code", code);
    cJSON_AddItemToObject(root, "error", error);
    return send_json(out, out_len, root);
}

static esp_err_t claim_endpoint(uint32_t session_id, const uint8_t *in, ssize_t len,
                                uint8_t **out, ssize_t *out_len, void *context)
{
    (void)session_id;
    (void)context;
    if(!in || len <= 0) return send_error(out, out_len, "INVALID_REQUEST");
    cJSON *request = cJSON_ParseWithLength((const char *)in, (size_t)len);
    if(!request) return send_error(out, out_len, "INVALID_REQUEST");
    cJSON *protocol = cJSON_GetObjectItemCaseSensitive(request, "protocol");
    cJSON *op = cJSON_GetObjectItemCaseSensitive(request, "op");
    if(!cJSON_IsNumber(protocol) || protocol->valuedouble != 1.0 || !cJSON_IsString(op)) {
        cJSON_Delete(request);
        return send_error(out, out_len, "UNSUPPORTED_PROTOCOL");
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if(strcmp(op->valuestring, "restart_claim") == 0) {
        /*
         * Recovery for an ACK persisted by an older/lost phone session. Only a
         * caller already inside the authenticated BLE Security 1 endpoint can
         * request this; ownership remains enforced by the backend claim API.
         */
        if(!s_status.identity_provisioned) {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "DEVICE_NOT_ENROLLED");
        }
        if(s_status.account_linked || s_status.binding_id[0] != '\0') {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "DEVICE_ALREADY_LINKED");
        }
        esp_err_t reset_err = clear_pending_claim(false);
        if(reset_err != ESP_OK) {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "PERSISTENCE_FAILED");
        }
        if(!cJSON_SetValuestring(op, "prepare_claim")) {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "PERSISTENCE_FAILED");
        }
    }
    if(strcmp(op->valuestring, "prepare_claim") == 0) {
        cJSON_Delete(request);
        if(!s_status.identity_provisioned) {
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "DEVICE_NOT_ENROLLED");
        }
        if(s_status.account_linked) {
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "DEVICE_ALREADY_LINKED");
        }
        if(s_status.claim_expired ||
           (s_status.claim_published && s_claim_expires_at[0] && published_claim_expired())) {
            esp_err_t expire_err = clear_pending_claim(false);
            if(expire_err != ESP_OK) {
                xSemaphoreGive(s_mutex);
                return send_error(out, out_len, "PERSISTENCE_FAILED");
            }
        }
        if(s_status.claim_acknowledged) {
            cJSON *response = cJSON_CreateObject();
            if(!response) { xSemaphoreGive(s_mutex); return ESP_ERR_NO_MEM; }
            cJSON_AddNumberToObject(response, "protocol", 1);
            cJSON_AddStringToObject(response, "device_id", s_status.device_id);
            cJSON_AddStringToObject(response, "name", s_status.name);
            cJSON_AddStringToObject(response, "claim_session_id", s_session_id);
            cJSON_AddStringToObject(response, "state", "acknowledged");
            xSemaphoreGive(s_mutex);
            return send_json(out, out_len, response);
        }
        if(!s_ram_proof_ready || esp_timer_get_time() >= s_session_deadline_us) {
            wipe_secret();
            make_session_id(s_session_id);
            esp_fill_random(s_secret_raw, sizeof(s_secret_raw));
            encode_base64url(s_secret_raw, s_secret_b64);
            esp_err_t hash_err = calculate_proof_hash(s_status.device_id, s_session_id,
                                                       s_secret_raw, s_secret_hash);
            if(hash_err != ESP_OK) {
                wipe_secret();
                xSemaphoreGive(s_mutex);
                return send_error(out, out_len, "PERSISTENCE_FAILED");
            }
            s_session_deadline_us = esp_timer_get_time() + CLAIM_LIFETIME_US;
            s_ram_proof_ready = true;
        }
        cJSON *response = cJSON_CreateObject();
        if(!response) { xSemaphoreGive(s_mutex); return ESP_ERR_NO_MEM; }
        cJSON_AddNumberToObject(response, "protocol", 1);
        cJSON_AddStringToObject(response, "device_id", s_status.device_id);
        cJSON_AddStringToObject(response, "name", s_status.name);
        cJSON_AddStringToObject(response, "claim_session_id", s_session_id);
        cJSON_AddStringToObject(response, "claim_secret", s_secret_b64);
        cJSON_AddNumberToObject(response, "expires_in", 1800);
        xSemaphoreGive(s_mutex);
        return send_json(out, out_len, response);
    }

    if(strcmp(op->valuestring, "ack_claim") == 0) {
        cJSON *sid = cJSON_GetObjectItemCaseSensitive(request, "claim_session_id");
        cJSON *hash = cJSON_GetObjectItemCaseSensitive(request, "claim_secret_hash");
        bool valid = cJSON_IsString(sid) && cJSON_IsString(hash) &&
                     strlen(sid->valuestring) == 36 && strlen(hash->valuestring) == 64;
        if(!valid) {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "INVALID_REQUEST");
        }
        if(s_status.claim_acknowledged &&
           strcmp(sid->valuestring, s_session_id) == 0 &&
           constant_time_equal(hash->valuestring, s_secret_hash, 64)) {
            char accepted_id[37];
            snprintf(accepted_id, sizeof(accepted_id), "%s", s_session_id);
            char device_id[sizeof(s_status.device_id)];
            snprintf(device_id, sizeof(device_id), "%s", s_status.device_id);
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            cJSON *response = cJSON_CreateObject();
            if(!response) return ESP_ERR_NO_MEM;
            cJSON_AddNumberToObject(response, "protocol", 1);
            cJSON_AddStringToObject(response, "device_id", device_id);
            cJSON_AddStringToObject(response, "claim_session_id", accepted_id);
            cJSON_AddBoolToObject(response, "acknowledged", true);
            return send_json(out, out_len, response);
        }
        if(!s_ram_proof_ready || esp_timer_get_time() >= s_session_deadline_us) {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "CLAIM_EXPIRED");
        }
        if(strcmp(sid->valuestring, s_session_id) != 0) {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "CLAIM_SESSION_MISMATCH");
        }
        if(!constant_time_equal(hash->valuestring, s_secret_hash, 64)) {
            cJSON_Delete(request);
            xSemaphoreGive(s_mutex);
            return send_error(out, out_len, "CLAIM_PROOF_INVALID");
        }
        char accepted_id[37];
        char device_id[sizeof(s_status.device_id)];
        snprintf(accepted_id, sizeof(accepted_id), "%s", s_session_id);
        snprintf(device_id, sizeof(device_id), "%s", s_status.device_id);
        cJSON_Delete(request);
        esp_err_t err = persist_ack(s_session_id, s_secret_hash);
        if(err == ESP_OK) {
            s_status.claim_acknowledged = true;
            snprintf(s_status.message, sizeof(s_status.message), "Prueba de vinculación guardada");
            wipe_secret();
        }
        xSemaphoreGive(s_mutex);
        if(err != ESP_OK) return send_error(out, out_len, "PERSISTENCE_FAILED");
        cJSON *response = cJSON_CreateObject();
        if(!response) return ESP_ERR_NO_MEM;
        cJSON_AddNumberToObject(response, "protocol", 1);
        cJSON_AddStringToObject(response, "device_id", device_id);
        cJSON_AddStringToObject(response, "claim_session_id", accepted_id);
        cJSON_AddBoolToObject(response, "acknowledged", true);
        return send_json(out, out_len, response);
    }
    cJSON_Delete(request);
    xSemaphoreGive(s_mutex);
    return send_error(out, out_len, "INVALID_REQUEST");
}

static bool fixed_decimal(const char *text, size_t length, unsigned *value)
{
    unsigned parsed = 0;
    for(size_t i = 0; i < length; ++i) {
        if(text[i] < '0' || text[i] > '9') return false;
        parsed = parsed * 10U + (unsigned)(text[i] - '0');
    }
    *value = parsed;
    return true;
}

static int64_t days_from_civil(int year, unsigned month, unsigned day)
{
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned year_of_era = (unsigned)(year - era * 400);
    const unsigned shifted_month = month > 2 ? month - 3U : month + 9U;
    const unsigned day_of_year = (153U * shifted_month + 2U) / 5U + day - 1U;
    const unsigned day_of_era = year_of_era * 365U + year_of_era / 4U - year_of_era / 100U + day_of_year;
    return (int64_t)era * 146097 + (int64_t)day_of_era - 719468;
}

static bool parse_iso_utc_milliseconds(const char *text, int64_t *milliseconds)
{
    if(!text || !milliseconds) return false;
    size_t length = strlen(text);
    if(length != 20 && length != 24) return false;
    if(text[4] != '-' || text[7] != '-' || text[10] != 'T' ||
       text[13] != ':' || text[16] != ':' ||
       (length == 20 && text[19] != 'Z') ||
       (length == 24 && (text[19] != '.' || text[23] != 'Z'))) return false;
    unsigned year, month, day, hour, minute, second, millis = 0;
    if(!fixed_decimal(text, 4, &year) || !fixed_decimal(text + 5, 2, &month) ||
       !fixed_decimal(text + 8, 2, &day) || !fixed_decimal(text + 11, 2, &hour) ||
       !fixed_decimal(text + 14, 2, &minute) || !fixed_decimal(text + 17, 2, &second) ||
       (length == 24 && !fixed_decimal(text + 20, 3, &millis))) return false;
    if(year < 1970 || month < 1 || month > 12 || day < 1 || hour > 23 || minute > 59 || second > 59)
        return false;
    static const uint8_t month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned max_day = month_days[month - 1];
    if(month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) max_day = 29;
    if(day > max_day) return false;
    int64_t days = days_from_civil((int)year, month, day);
    *milliseconds = (((days * 24 + hour) * 60 + minute) * 60 + second) * 1000 + millis;
    return true;
}

static bool published_claim_expired(void)
{
    if(!s_status.claim_published || !s_claim_expires_at[0]) return false;
    time_t now = 0;
    time(&now);
    int64_t expires_ms = 0;
    if(now < 1760000000 || !parse_iso_utc_milliseconds(s_claim_expires_at, &expires_ms)) return false;
    return (int64_t)now * 1000 >= expires_ms;
}

static uint32_t parse_retry_after_seconds(const char *value)
{
    if(!value || !value[0]) return 0;
    char *end = NULL;
    unsigned long parsed = strtoul(value, &end, 10);
    if(end == value || parsed == 0) return 0;
    while(*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') ++end;
    if(*end != '\0') return 0;
    return parsed > 86400UL ? 86400U : (uint32_t)parsed;
}

static void clear_retry_delay(void)
{
    s_retry_not_before_us = 0;
    s_retry_not_before_epoch = 0;
    s_retry_is_waiting = false;
    s_status.retry_after_seconds = 0;
    s_status.backend_waiting = false;
    (void)persist_retry_until(0, false);
}

static void schedule_retry(uint32_t seconds, bool backend_waiting)
{
    if(seconds == 0) seconds = backend_waiting ? 5 : 15;
    time_t now = 0;
    time(&now);
    s_retry_not_before_us = esp_timer_get_time() + (int64_t)seconds * 1000000LL;
    s_retry_not_before_epoch = now >= 1760000000 ? (int64_t)now + seconds : 0;
    s_status.retry_after_seconds = seconds;
    s_status.backend_waiting = backend_waiting;
    s_retry_is_waiting = backend_waiting;
    if(s_retry_not_before_epoch > 0)
        (void)persist_retry_until((uint64_t)s_retry_not_before_epoch, backend_waiting);
}

static void parse_backend_error(const http_response_t *response, char code[48], uint32_t *retry_seconds)
{
    if(!response || !response->data[0]) return;
    cJSON *root = cJSON_Parse(response->data);
    if(!root) return;
    cJSON *error = cJSON_GetObjectItemCaseSensitive(root, "error");
    cJSON *error_code = cJSON_GetObjectItemCaseSensitive(error, "code");
    cJSON *retry = cJSON_GetObjectItemCaseSensitive(error, "retryAfterSeconds");
    if(cJSON_IsString(error_code) && error_code->valuestring)
        snprintf(code, 48, "%s", error_code->valuestring);
    if(*retry_seconds == 0 && cJSON_IsNumber(retry) && retry->valuedouble >= 1 && retry->valuedouble <= 86400)
        *retry_seconds = (uint32_t)retry->valuedouble;
    cJSON_Delete(root);
}

static esp_err_t backend_http_error(int status, const http_response_t *response, bool publishing)
{
    char code[48] = {0};
    uint32_t retry = parse_retry_after_seconds(response ? response->retry_after : NULL);
    parse_backend_error(response, code, &retry);
    ESP_LOGW(TAG, "Backend %s HTTP=%d code=%.47s",
             publishing ? "publish" : "link", status, code[0] ? code : "unavailable");
    if(strcmp(code, "CLAIM_NOT_READY") == 0) {
        schedule_retry(retry ? retry : 5, true);
        snprintf(s_status.message, sizeof(s_status.message),
                 "El servidor aún espera la prueba; reintento en %lu s.",
                 (unsigned long)s_status.retry_after_seconds);
        return ESP_ERR_TIMEOUT;
    }
    if(status == 410 || strcmp(code, "CLAIM_EXPIRED") == 0) {
        esp_err_t err = clear_pending_claim(true);
        if(err != ESP_OK) {
            snprintf(s_status.message, sizeof(s_status.message), "La sesión venció y no se pudo guardar el estado.");
            return err;
        }
        clear_retry_delay();
        snprintf(s_status.message, sizeof(s_status.message),
                 "La sesión de vinculación venció. Inicia una nueva desde HAVIT móvil.");
        return ESP_ERR_INVALID_STATE;
    }
    if(status == 429 || strcmp(code, "RATE_LIMITED") == 0) {
        schedule_retry(retry ? retry : 60, false);
        snprintf(s_status.message, sizeof(s_status.message),
                 "Límite temporal del servidor; espera %lu s para reintentar.",
                 (unsigned long)s_status.retry_after_seconds);
        return ESP_ERR_TIMEOUT;
    }
    if((status == 401 && (strcmp(code, "DEVICE_CREDENTIAL_REVOKED") == 0 ||
                          strcmp(code, "UNAUTHENTICATED") == 0)) ||
       (status == 403 && strcmp(code, "FORBIDDEN") == 0)) {
        s_credential_rejected_this_boot = true;
        s_status.device_credential_rejected = true;
        s_status.retry_after_seconds = 0;
        s_status.backend_waiting = false;
        snprintf(s_status.message, sizeof(s_status.message),
                 "La credencial de esta unidad fue rechazada. Requiere revisión de alta USB.");
        return ESP_ERR_INVALID_STATE;
    }
    if(strcmp(code, "CLAIM_PROOF_INVALID") == 0 || strcmp(code, "CLAIM_SUPERSEDED") == 0 ||
       strcmp(code, "IDEMPOTENCY_CONFLICT") == 0) {
        esp_err_t err = clear_pending_claim(false);
        snprintf(s_status.message, sizeof(s_status.message),
                 "La prueba dejó de ser válida. Inicia una nueva vinculación desde HAVIT móvil.");
        return err == ESP_OK ? ESP_ERR_INVALID_STATE : err;
    }
    if(publishing && status == 409 && strcmp(code, "DEVICE_ALREADY_LINKED") == 0) {
        /* Query the authoritative link endpoint before presenting a linked state. */
        return ESP_OK;
    }
    if(status >= 500 || status == 0) {
        schedule_retry(retry ? retry : 15, false);
        snprintf(s_status.message, sizeof(s_status.message),
                 "Backend no disponible; reintento en %lu s.",
                 (unsigned long)s_status.retry_after_seconds);
        return ESP_ERR_TIMEOUT;
    }
    s_status.retry_after_seconds = 0;
    s_status.backend_waiting = false;
    snprintf(s_status.message, sizeof(s_status.message),
             "Backend rechazó (%d, %.24s). Revisa vinculación.", status,
             code[0] ? code : "sin código");
    return ESP_ERR_INVALID_RESPONSE;
}

static esp_err_t http_event(esp_http_client_event_t *event)
{
    http_response_t *response = event->user_data;
    if(event->event_id == HTTP_EVENT_ON_HEADER && response && event->header_key &&
       event->header_value && strcasecmp(event->header_key, "Retry-After") == 0) {
        snprintf(response->retry_after, sizeof(response->retry_after), "%s", event->header_value);
    }
    if(event->event_id == HTTP_EVENT_ON_DATA && event->data_len > 0) {
        if(!response || response->length + (size_t)event->data_len >= sizeof(response->data)) {
            if(response) response->overflow = true;
            return ESP_FAIL;
        }
        memcpy(response->data + response->length, event->data, (size_t)event->data_len);
        response->length += (size_t)event->data_len;
        response->data[response->length] = '\0';
    }
    return ESP_OK;
}

static esp_err_t http_request(const char *url, esp_http_client_method_t method,
                              const char *body, http_response_t *response, int *status_code)
{
    char authorization[sizeof("Bearer ") + sizeof(s_device_token)];
    snprintf(authorization, sizeof(authorization), "Bearer %s", s_device_token);
    esp_http_client_config_t config = {
        .url = url,
        .method = method,
        .timeout_ms = 12000,
        .buffer_size = 512,
        .buffer_size_tx = 512,
        .event_handler = http_event,
        .user_data = response,
        .disable_auto_redirect = true,
        .max_authorization_retries = -1,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if(!client) return ESP_ERR_NO_MEM;
    esp_err_t err = esp_http_client_set_header(client, "Authorization", authorization);
    if(err == ESP_OK && body) {
        err = esp_http_client_set_header(client, "Content-Type", "application/json");
        if(err == ESP_OK) err = esp_http_client_set_post_field(client, body, (int)strlen(body));
    }
    if(err == ESP_OK) err = esp_http_client_perform(client);
    if(err == ESP_OK) *status_code = esp_http_client_get_status_code(client);
    if(response->overflow) err = ESP_ERR_INVALID_SIZE;
    esp_http_client_cleanup(client);
    memset(authorization, 0, sizeof(authorization));
    return err;
}

static bool ensure_system_time(void)
{
    time_t now = 0;
    time(&now);
    if(now > 1760000000) return true;
    if(!s_sntp_started) {
        const esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
        esp_err_t err = esp_netif_sntp_init(&config);
        if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "SNTP initialization unavailable (%s)", esp_err_to_name(err));
            return false;
        }
        s_sntp_started = true;
    }
    for(unsigned i = 0; i < 40; ++i) {
        time(&now);
        if(now > 1760000000) return true;
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    return false;
}

static esp_err_t publish_claim_if_needed(void)
{
    if(!s_status.claim_acknowledged || s_status.claim_published || s_status.claim_expired) return ESP_OK;
    cJSON *json = cJSON_CreateObject();
    if(!json) return ESP_ERR_NO_MEM;
    cJSON_AddStringToObject(json, "claimSessionId", s_session_id);
    cJSON_AddStringToObject(json, "claimSecretHash", s_secret_hash);
    cJSON_AddNumberToObject(json, "protocol", 1);
    char *body = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if(!body) return ESP_ERR_NO_MEM;
    char url[192];
    snprintf(url, sizeof(url), "%s/v1/device/claim-sessions", s_status.api_base_url);
    http_response_t response = {0};
    int status = 0;
    esp_err_t err = http_request(url, HTTP_METHOD_POST, body, &response, &status);
    memset(body, 0, strlen(body));
    free(body);
    if(err != ESP_OK) return backend_http_error(status, &response, true);
    if(status != 200 && status != 201) return backend_http_error(status, &response, true);
    cJSON *reply = cJSON_Parse(response.data);
    if(!reply) return ESP_ERR_INVALID_RESPONSE;
    cJSON *sid = cJSON_GetObjectItemCaseSensitive(reply, "claimSessionId");
    cJSON *device = cJSON_GetObjectItemCaseSensitive(reply, "deviceId");
    cJSON *expiry = cJSON_GetObjectItemCaseSensitive(reply, "expiresAt");
    bool valid = cJSON_IsString(sid) && cJSON_IsString(device) && cJSON_IsString(expiry) &&
                 strcmp(sid->valuestring, s_session_id) == 0 &&
                 strcmp(device->valuestring, s_status.device_id) == 0;
    char expires_at[40] = {0};
    int64_t expiry_ms = 0;
    if(valid) {
        snprintf(expires_at, sizeof(expires_at), "%s", expiry->valuestring);
        valid = parse_iso_utc_milliseconds(expires_at, &expiry_ms);
    }
    cJSON_Delete(reply);
    if(!valid) return ESP_ERR_INVALID_RESPONSE;
    err = persist_published(expires_at);
    if(err == ESP_OK) {
        s_status.claim_published = true;
        s_status.claim_expired = false;
        snprintf(s_claim_expires_at, sizeof(s_claim_expires_at), "%s", expires_at);
        clear_retry_delay();
    }
    return err;
}

static esp_err_t refresh_link_status(void)
{
    char url[160];
    snprintf(url, sizeof(url), "%s/v1/device/link", s_status.api_base_url);
    http_response_t response = {0};
    int status = 0;
    esp_err_t err = http_request(url, HTTP_METHOD_GET, NULL, &response, &status);
    if(err != ESP_OK) return backend_http_error(status, &response, false);
    if(status != 200) return backend_http_error(status, &response, false);
    cJSON *reply = cJSON_Parse(response.data);
    if(!reply) return ESP_ERR_INVALID_RESPONSE;
    cJSON *device = cJSON_GetObjectItemCaseSensitive(reply, "deviceId");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(reply, "state");
    cJSON *binding = cJSON_GetObjectItemCaseSensitive(reply, "bindingId");
    cJSON *account_name = cJSON_GetObjectItemCaseSensitive(reply, "displayName");
    if(!cJSON_IsString(account_name) || account_name->valuestring[0] == '\0')
        account_name = cJSON_GetObjectItemCaseSensitive(reply, "accountName");
    if(!cJSON_IsString(device) || strcmp(device->valuestring, s_status.device_id) != 0 ||
       !cJSON_IsString(state)) {
        cJSON_Delete(reply);
        return ESP_ERR_INVALID_RESPONSE;
    }
    if(strcmp(state->valuestring, "linked") == 0) {
        if(!cJSON_IsString(binding) || binding->valuestring[0] == '\0') {
            cJSON_Delete(reply);
            return ESP_ERR_INVALID_RESPONSE;
        }
        char confirmed_binding[sizeof(s_status.binding_id)];
        snprintf(confirmed_binding, sizeof(confirmed_binding), "%s", binding->valuestring);
        const bool binding_changed = s_status.binding_id[0] &&
                                     strcmp(s_status.binding_id, confirmed_binding) != 0;
        if(binding_changed && clear_pending_claim(false) != ESP_OK) {
            cJSON_Delete(reply);
            snprintf(s_status.message, sizeof(s_status.message), "No se pudo invalidar la prueba anterior.");
            return ESP_FAIL;
        }
        if(persist_link(confirmed_binding, true) != ESP_OK) {
            cJSON_Delete(reply);
            snprintf(s_status.message, sizeof(s_status.message), "No se pudo guardar la vinculación confirmada.");
            return ESP_FAIL;
        }
        snprintf(s_status.binding_id, sizeof(s_status.binding_id), "%s", confirmed_binding);
        if(binding_changed) s_status.account_name[0] = '\0';
        if(cJSON_IsString(account_name) && account_name->valuestring[0]) {
            size_t used = 0;
            for(const unsigned char *p = (const unsigned char *)account_name->valuestring;
                *p && used + 1 < sizeof(s_status.account_name); ++p) {
                if(*p >= 0x20 && *p != 0x7f) s_status.account_name[used++] = (char)*p;
            }
            s_status.account_name[used] = '\0';
        }
        s_status.account_linked = true;
        s_status.claim_expired = false;
        snprintf(s_status.message, sizeof(s_status.message), "Dispositivo vinculado a la cuenta");
    } else if(strcmp(state->valuestring, "unclaimed") == 0) {
        bool had_previous_binding = s_status.binding_id[0] != '\0';
        /* Drop old ownership-scoped state before erasing the marker that proves it existed. */
        if(had_previous_binding && clear_pending_claim(false) != ESP_OK) {
            cJSON_Delete(reply);
            return ESP_FAIL;
        }
        s_status.account_linked = false;
        s_status.binding_id[0] = '\0';
        s_status.account_name[0] = '\0';
        if(persist_link("", false) != ESP_OK) {
            cJSON_Delete(reply);
            return ESP_FAIL;
        }
        /* Server ownership epoch changed: the next owner must open a fresh BLE proof. */
        if(had_previous_binding)
            snprintf(s_status.message, sizeof(s_status.message),
                     "Vínculo eliminado. Inicia una nueva configuración Bluetooth.");
        else if(s_status.claim_expired)
            snprintf(s_status.message, sizeof(s_status.message),
                     "La sesión de vinculación venció. Inicia una nueva desde HAVIT móvil.");
        else
            snprintf(s_status.message, sizeof(s_status.message),
                     s_status.claim_published ? "Esperando que HAVIT móvil vincule la cuenta" :
                     "Esperando confirmación de la prueba de vinculación");
    } else {
        cJSON_Delete(reply);
        return ESP_ERR_INVALID_RESPONSE;
    }
    cJSON_Delete(reply);
    clear_retry_delay();
    return ESP_OK;
}

esp_err_t havit_device_claim_init(const char *fallback_device_id, const char *fallback_name)
{
    if(!fallback_device_id || !fallback_name ||
       strlen(fallback_device_id) >= sizeof(s_status.device_id) ||
       strlen(fallback_name) >= sizeof(s_status.name))
        return ESP_ERR_INVALID_ARG;
    if(!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
        if(!s_mutex) return ESP_ERR_NO_MEM;
    }
    memset(s_device_token, 0, sizeof(s_device_token));
    memset(s_session_id, 0, sizeof(s_session_id));
    memset(s_secret_hash, 0, sizeof(s_secret_hash));
    memset(s_claim_expires_at, 0, sizeof(s_claim_expires_at));
    s_status.identity_provisioned = false;
    s_status.claim_acknowledged = false;
    s_status.claim_published = false;
    s_status.claim_expired = false;
    s_status.device_credential_rejected = false;
    s_status.backend_waiting = false;
    s_status.retry_after_seconds = 0;
    s_status.account_linked = false;
    s_status.binding_id[0] = '\0';
    s_status.account_name[0] = '\0';
    s_retry_not_before_us = 0;
    s_retry_not_before_epoch = 0;
    s_retry_is_waiting = false;
    s_credential_rejected_this_boot = false;
    s_session_deadline_us = 0;
    wipe_secret();
    snprintf(s_status.device_id, sizeof(s_status.device_id), "%s", fallback_device_id);
    snprintf(s_status.name, sizeof(s_status.name), "%s", fallback_name);
    snprintf(s_status.api_base_url, sizeof(s_status.api_base_url), "%s", API_BASE_URL);
    snprintf(s_status.message, sizeof(s_status.message), "Unidad pendiente de alta USB");

    esp_err_t err = nvs_flash_init_partition(IDENTITY_PARTITION);
    if(err != ESP_OK) {
        ESP_LOGW(TAG, "Identity partition unavailable (%s); online claiming disabled",
                 esp_err_to_name(err));
        return ESP_OK;
    }
    nvs_handle_t handle;
    err = nvs_open_from_partition(IDENTITY_PARTITION, IDENTITY_NAMESPACE, NVS_READONLY, &handle);
    if(err != ESP_OK) {
        ESP_LOGI(TAG, "Device not enrolled; BLE setup remains available");
        return ESP_OK;
    }
    char device_id[sizeof(s_status.device_id)] = {0};
    char token[sizeof(s_device_token)] = {0};
    char base_url[sizeof(s_status.api_base_url)] = {0};
    esp_err_t id_err = load_nvs_string(handle, "device_id", device_id, sizeof(device_id));
    esp_err_t token_err = load_nvs_string(handle, "device_token", token, sizeof(token));
    esp_err_t url_err = load_nvs_string(handle, "api_base_url", base_url, sizeof(base_url));
    nvs_close(handle);
    if(id_err != ESP_OK || token_err != ESP_OK || url_err != ESP_OK ||
       strcmp(device_id, fallback_device_id) != 0 || !valid_device_token(token) ||
       strcmp(base_url, API_BASE_URL) != 0) {
        ESP_LOGW(TAG, "Identity record is incomplete or does not match this unit");
        memset(token, 0, sizeof(token));
        return ESP_OK;
    }
    snprintf(s_status.device_id, sizeof(s_status.device_id), "%s", device_id);
    snprintf(s_device_token, sizeof(s_device_token), "%s", token);
    snprintf(s_status.api_base_url, sizeof(s_status.api_base_url), "%s", base_url);
    memset(token, 0, sizeof(token));
    s_status.identity_provisioned = true;
    if(!load_claim_state()) {
        s_status.claim_acknowledged = false;
        s_status.claim_published = false;
    }
    s_status.backend_waiting = s_retry_is_waiting;
    snprintf(s_status.message, sizeof(s_status.message),
             s_status.claim_expired ? "La sesión de vinculación venció; espera una nueva desde el móvil." :
             s_status.binding_id[0] ? "Identidad cargada; se verificará la vinculación con el servidor." :
             "Identidad del dispositivo cargada");
    ESP_LOGI(TAG, "Identity loaded for unit %s; API credentials are not logged", s_status.device_id);
    return ESP_OK;
}

esp_err_t havit_device_claim_register_endpoint(void)
{
    return wifi_prov_mgr_endpoint_register("havit-claim", claim_endpoint, NULL);
}

esp_err_t havit_device_claim_refresh(void)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if(!s_status.identity_provisioned) {
        snprintf(s_status.message, sizeof(s_status.message), "Unidad pendiente de alta USB");
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }
    if(s_credential_rejected_this_boot) {
        s_status.device_credential_rejected = true;
        snprintf(s_status.message, sizeof(s_status.message),
                 "La credencial de esta unidad fue rechazada. Requiere revisión de alta USB.");
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_STATE;
    }
    if(!ensure_system_time()) {
        snprintf(s_status.message, sizeof(s_status.message), "Esperando hora de red para HTTPS seguro");
        xSemaphoreGive(s_mutex);
        return ESP_ERR_TIMEOUT;
    }
    time_t now = 0;
    time(&now);
    if(s_retry_not_before_epoch > (int64_t)now) {
        s_status.retry_after_seconds = (uint32_t)(s_retry_not_before_epoch - (int64_t)now);
        s_status.backend_waiting = s_retry_is_waiting;
        snprintf(s_status.message, sizeof(s_status.message),
                 s_retry_is_waiting ? "El servidor aún procesa la vinculación; reintento en %lu s." :
                 "Límite temporal del servidor; espera %lu s para reintentar.",
                 (unsigned long)s_status.retry_after_seconds);
        xSemaphoreGive(s_mutex);
        return ESP_ERR_TIMEOUT;
    }
    if(s_retry_not_before_epoch > 0) clear_retry_delay();

    if(s_status.claim_published && s_claim_expires_at[0]) {
        int64_t expires_ms = 0;
        if(!parse_iso_utc_milliseconds(s_claim_expires_at, &expires_ms)) {
            snprintf(s_status.message, sizeof(s_status.message),
                     "El servidor devolvió una fecha de caducidad inválida.");
            xSemaphoreGive(s_mutex);
            return ESP_ERR_INVALID_RESPONSE;
        }
        if((int64_t)now * 1000 >= expires_ms) {
            esp_err_t expire_err = clear_pending_claim(true);
            if(expire_err != ESP_OK) {
                snprintf(s_status.message, sizeof(s_status.message),
                         "La sesión venció y no se pudo guardar el estado.");
                xSemaphoreGive(s_mutex);
                return expire_err;
            }
            snprintf(s_status.message, sizeof(s_status.message),
                     "La sesión de vinculación venció. Inicia una nueva desde HAVIT móvil.");
        }
    }
    esp_err_t err = publish_claim_if_needed();
    if(err != ESP_OK && !s_status.claim_expired) {
        if(s_status.device_credential_rejected || s_status.claim_acknowledged ||
           err != ESP_ERR_INVALID_STATE) {
            xSemaphoreGive(s_mutex);
            return err;
        }
        /* A stale proof was invalidated by an owner unlink; query authoritative link state. */
    }
    err = refresh_link_status();
    if(err != ESP_OK) {
        xSemaphoreGive(s_mutex);
        return err;
    }
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

void havit_device_claim_snapshot(havit_device_claim_snapshot_t *out)
{
    if(!out) return;
    if(s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out = s_status;
    if(s_mutex) xSemaphoreGive(s_mutex);
}

typedef struct { char *data; size_t length, capacity; bool overflow; } tasks_http_t;
static esp_err_t tasks_http_event(esp_http_client_event_t *event)
{
    tasks_http_t *r = event->user_data;
    if(event->event_id == HTTP_EVENT_ON_DATA && event->data_len > 0) {
        if(r->length + event->data_len >= r->capacity) { r->overflow = true; return ESP_FAIL; }
        memcpy(r->data + r->length, event->data, event->data_len);
        r->length += event->data_len; r->data[r->length] = 0;
    }
    return ESP_OK;
}

esp_err_t havit_device_api_request(const char *path, const char *body, char *response,
                                   size_t capacity, int *http_status)
{
    if(!path || strncmp(path, "/v1/device/", 11) != 0 || !response || capacity < 2 || !http_status || !s_mutex)
        return ESP_ERR_INVALID_ARG;
    char authorization[sizeof("Bearer ") + sizeof(s_device_token)], url[256];
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool allowed = s_status.identity_provisioned && s_status.account_linked;
    if(allowed) snprintf(authorization, sizeof(authorization), "Bearer %s", s_device_token);
    xSemaphoreGive(s_mutex);
    if(!allowed) return ESP_ERR_INVALID_STATE;
    if(snprintf(url, sizeof(url), "%s%s", API_BASE_URL, path) >= sizeof(url)) return ESP_ERR_INVALID_SIZE;
    response[0] = 0; *http_status = 0;
    tasks_http_t reply = {.data = response, .capacity = capacity};
    esp_http_client_config_t config = {.url = url, .method = body ? HTTP_METHOD_POST : HTTP_METHOD_GET,
        .timeout_ms = 15000, .buffer_size = 1024, .buffer_size_tx = 1024,
        .event_handler = tasks_http_event, .user_data = &reply, .disable_auto_redirect = true,
        .max_authorization_retries = -1, .crt_bundle_attach = esp_crt_bundle_attach};
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if(!client) { memset(authorization, 0, sizeof(authorization)); return ESP_ERR_NO_MEM; }
    esp_err_t err = esp_http_client_set_header(client, "Authorization", authorization);
    if(err == ESP_OK && body) {
        err = esp_http_client_set_header(client, "Content-Type", "application/json");
        if(err == ESP_OK) err = esp_http_client_set_post_field(client, body, strlen(body));
    }
    if(err == ESP_OK) err = esp_http_client_perform(client);
    if(err == ESP_OK) *http_status = esp_http_client_get_status_code(client);
    if(reply.overflow) err = ESP_ERR_INVALID_SIZE;
    esp_http_client_cleanup(client); memset(authorization, 0, sizeof(authorization));
    return err;
}
