#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool identity_provisioned;
    bool claim_acknowledged;
    bool claim_published;
    bool claim_expired;
    bool device_credential_rejected;
    bool backend_waiting;
    uint32_t retry_after_seconds;
    bool account_linked;
    char device_id[24];
    char name[16];
    char binding_id[40];
    char account_name[40];
    char api_base_url[128];
    char message[96];
} havit_device_claim_snapshot_t;

/* Load the operator-provisioned identity partition without erasing any NVS. */
esp_err_t havit_device_claim_init(const char *fallback_device_id, const char *fallback_name);
/* A deliberate new BLE setup invalidates an unlinked prior proof. */
esp_err_t havit_device_claim_begin_window(void);
/* Register only after the provisioning manager has started its secure session. */
esp_err_t havit_device_claim_register_endpoint(void);
/* Publish an ACKed claim (if any) and read the authoritative account link. */
esp_err_t havit_device_claim_refresh(void);
void havit_device_claim_snapshot(havit_device_claim_snapshot_t *out);
/* Internal derived key; never expose through logs, HTTP or BLE. */
esp_err_t havit_device_relay_key(uint8_t out[32]);
bool havit_device_claim_confirm_bridge(const char *binding_id);
/* Factory reset clears claim state only; the enrolled device identity/token remain. */
esp_err_t havit_device_claim_factory_reset(void);
/* Fixed backend only. Caller owns the bounded response buffer; no credentials escape. */
esp_err_t havit_device_api_request(const char *path, const char *body, char *response,
                                   size_t capacity, int *http_status);

#ifdef __cplusplus
}
#endif
