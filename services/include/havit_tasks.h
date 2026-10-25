#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include "esp_err.h"
#ifdef __cplusplus
extern "C" {
#endif
#define HAVIT_TASK_LIMIT 64
typedef struct {
    char id[37], title[161], detail[2049], due_at[32], created_at[32], updated_at[32];
    int version;
    bool completed;
} havit_task_t;
typedef struct {
    int count, pending, conflicts, utc_offset;
    uint32_t revision, ui_revision;
    bool ready, online, clock_valid;
    char time_zone[81], message[96];
} havit_tasks_status_t;
esp_err_t havit_tasks_init(void);
esp_err_t havit_tasks_factory_reset(void);
void havit_tasks_tick(bool wifi_connected);
void havit_tasks_status(havit_tasks_status_t *out);
bool havit_tasks_get(int index, havit_task_t *out);
bool havit_tasks_save(const havit_task_t *task, bool create);
bool havit_tasks_delete(const havit_task_t *task);
/* Internal adapter for relay snapshots. Never register this plaintext handler
 * directly on BLE; apply_snapshot is called only after backend AEAD validation. */
esp_err_t havit_tasks_ble_endpoint(uint32_t session_id, const uint8_t *in, ssize_t len,
                                   uint8_t **out, ssize_t *out_len, void *context);
void havit_tasks_resolve(bool use_local);
void havit_tasks_refresh(void);
void havit_tasks_local_time(struct tm *out);
void havit_tasks_date(const char *utc, char out[11]);
bool havit_tasks_due_from_local(const char *local, char out[32]);
void havit_tasks_due_local(const char *utc, char out[24]);
#ifdef __cplusplus
}
#endif
