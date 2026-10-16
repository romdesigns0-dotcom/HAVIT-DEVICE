/**
 * @file havit_store.h
 * Small persistence port used by the Havit OS application layer.
 *
 * The application owns no storage implementation. The simulator and the
 * eventual ESP-IDF target provide adapters (for example, RAM and NVS).
 */

#ifndef HAVIT_STORE_H
#define HAVIT_STORE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *context;
    bool (*read_bool)(void *context, const char *key, bool *value);
    bool (*write_bool)(void *context, const char *key, bool value);
} havit_store_t;

#ifdef __cplusplus
}
#endif

#endif /* HAVIT_STORE_H */
