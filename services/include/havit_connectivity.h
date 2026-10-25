#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    HAVIT_NET_STARTING, HAVIT_NET_BLE_READY, HAVIT_NET_BLE_CONNECTED,
    HAVIT_NET_SECURE, HAVIT_NET_SCAN, HAVIT_NET_CONNECTING,
    HAVIT_NET_ONLINE, HAVIT_NET_LINK_PENDING, HAVIT_NET_LINKED,
    HAVIT_NET_BACKEND_ERROR, HAVIT_NET_ERROR
} havit_net_state_t;
typedef struct {
    uint32_t revision;
    havit_net_state_t state;
    bool provisioning;
    bool wifi_connected;
    bool presence_confirmed;
    bool fatal_error;
    bool has_saved_network;
    bool identity_provisioned;
    bool claim_acknowledged;
    bool claim_published;
    bool claim_expired;
    bool device_credential_rejected;
    bool backend_waiting;
    uint32_t retry_after_seconds;
    bool account_linked;
    char name[16];
    char device_id[24];
    char pop[7];
    char ip[16];
    char binding_id[40];
    char account_name[40];
    char message[96];
} havit_net_snapshot_t;

esp_err_t havit_connectivity_init(void);
void havit_connectivity_snapshot(havit_net_snapshot_t *out);
/* Opens BLE provisioning only when requested by the setup UI. */
esp_err_t havit_connectivity_begin(bool reconfigure);
esp_err_t havit_connectivity_retry(void);
/* Explicit UI confirmation: remove only Wi-Fi credentials, then start BLE. */
esp_err_t havit_connectivity_reset_wifi(void);
esp_err_t havit_connectivity_refresh_backend(void);
esp_err_t havit_connectivity_bridge_confirmed(void);
/* Authenticated BLE factory-reset ticket; processed by the network worker. */
esp_err_t havit_connectivity_factory_reset(void);
/* Called after the authenticated Wi-Fi reset acknowledgement; clears saved Wi-Fi. */
esp_err_t havit_connectivity_finish_factory_reset(void);
#ifdef __cplusplus
}
#endif
