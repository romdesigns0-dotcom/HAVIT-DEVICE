/**
 * @file havit_store_memory.h
 * Volatile persistence adapter for desktop simulation only.
 */

#ifndef HAVIT_STORE_MEMORY_H
#define HAVIT_STORE_MEMORY_H

#include "havit_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Return a store backed by RAM. Values reset when the simulator exits. */
havit_store_t havit_store_memory_create(void);

#ifdef __cplusplus
}
#endif

#endif /* HAVIT_STORE_MEMORY_H */
