idf_component_register(
    SRCS "../../../services/connectivity.c"
         "../../../services/device_claim.c"
         "../../../services/tasks.c"
         "../../../services/ble_relay.c"
    INCLUDE_DIRS "../../../services/include"
    REQUIRES esp_wifi esp_netif esp_event nvs_flash wifi_provisioning
             protocomm esp_timer esp_hw_support json freertos esp_http_client
             esp_netif mbedtls lwip)
