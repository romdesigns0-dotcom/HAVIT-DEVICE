#include "havit_nvs_store.h"

#include <string.h>

static bool read_bool(void *context, const char *key, bool *value)
{
    if(context == NULL || key == NULL || value == NULL) return false;

    /* NVS keys are limited to 15 characters; map app-level names to short keys. */
    if(strcmp(key, "onboarding_complete") != 0) return false;

    uint8_t stored = 0;
    if(nvs_get_u8(*(nvs_handle_t *)context, "onboarded", &stored) != ESP_OK) {
        return false;
    }
    *value = stored != 0;
    return true;
}

static bool write_bool(void *context, const char *key, bool value)
{
    if(context == NULL || key == NULL || strcmp(key, "onboarding_complete") != 0) {
        return false;
    }

    nvs_handle_t handle = *(nvs_handle_t *)context;
    if(nvs_set_u8(handle, "onboarded", value ? 1 : 0) != ESP_OK) return false;
    return nvs_commit(handle) == ESP_OK;
}

havit_store_t havit_store_nvs_create(nvs_handle_t handle)
{
    havit_store_t store = {
        .context = NULL,
        .read_bool = read_bool,
        .write_bool = write_bool
    };

    /* Keep the NVS handle address stable for the lifetime of the app. */
    static nvs_handle_t store_handle;
    store_handle = handle;
    store.context = &store_handle;
    return store;
}
