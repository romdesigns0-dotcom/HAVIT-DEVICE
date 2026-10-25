#pragma once
#include "esp_err.h"
#include <stdbool.h>
esp_err_t havit_ble_relay_start(void);
void havit_ble_relay_stop(void);
void havit_ble_relay_disconnected(void);
bool havit_ble_relay_active(void);
