/**
 * @file havit_store_memory.c
 */

#include "havit_store_memory.h"

#include <string.h>

#define MEMORY_STORE_ENTRY_COUNT 8
#define MEMORY_STORE_KEY_MAX 32

typedef struct {
    char key[MEMORY_STORE_KEY_MAX];
    bool value;
    bool used;
} memory_store_entry_t;

static memory_store_entry_t entries[MEMORY_STORE_ENTRY_COUNT];

static bool memory_read_bool(void *context, const char *key, bool *value)
{
    (void)context;
    if(key == NULL || value == NULL) return false;

    for(size_t i = 0; i < MEMORY_STORE_ENTRY_COUNT; i++) {
        if(entries[i].used && strcmp(entries[i].key, key) == 0) {
            *value = entries[i].value;
            return true;
        }
    }
    return false;
}

static bool memory_write_bool(void *context, const char *key, bool value)
{
    (void)context;
    if(key == NULL || strlen(key) >= MEMORY_STORE_KEY_MAX) return false;

    for(size_t i = 0; i < MEMORY_STORE_ENTRY_COUNT; i++) {
        if(entries[i].used && strcmp(entries[i].key, key) == 0) {
            entries[i].value = value;
            return true;
        }
    }

    for(size_t i = 0; i < MEMORY_STORE_ENTRY_COUNT; i++) {
        if(!entries[i].used) {
            strcpy(entries[i].key, key);
            entries[i].value = value;
            entries[i].used = true;
            return true;
        }
    }
    return false;
}

havit_store_t havit_store_memory_create(void)
{
    havit_store_t store = {
        .context = NULL,
        .read_bool = memory_read_bool,
        .write_bool = memory_write_bool
    };
    return store;
}
