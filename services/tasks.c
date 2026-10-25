#include "havit_tasks.h"
#include "havit_device_claim.h"
#include "havit_connectivity.h"
#include "cJSON.h"
#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define RESPONSE_CAP (196608)
#define CACHE_CAP (524288)
static SemaphoreHandle_t s_lock;
static nvs_handle_t s_nvs;
static cJSON *s_root, *s_view;
static cJSON *s_order[HAVIT_TASK_LIMIT];
static havit_tasks_status_t s_status;
static int64_t s_next_attempt;
static bool s_force;
static int64_t s_ble_confirmed_until;
static void *json_alloc(size_t n) { return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
static const char *str(const cJSON *obj, const char *key) {
    cJSON *v = cJSON_GetObjectItemCaseSensitive(obj, key);
    return cJSON_IsString(v) ? v->valuestring : "";
}
static int num(const cJSON *obj, const char *key) {
    cJSON *v = cJSON_GetObjectItemCaseSensitive(obj, key); return cJSON_IsNumber(v) ? v->valueint : 0;
}
static bool valid_uuid_text(const char *value) {
    if(!value || strlen(value)!=36)return false;
    for(int i=0;i<36;i++){
        if(i==8||i==13||i==18||i==23){if(value[i]!='-')return false;}
        else if(!((value[i]>='0'&&value[i]<='9')||(value[i]>='a'&&value[i]<='f')))return false;
    }
    return true;
}
static cJSON *pending(void) { return cJSON_GetObjectItemCaseSensitive(s_root, "pending"); }
static cJSON *snapshot(void) { return cJSON_GetObjectItemCaseSensitive(s_root, "snapshot"); }
static cJSON *task_array(void) { return cJSON_GetObjectItemCaseSensitive(snapshot(), "tasks"); }
static void replace(cJSON *obj, const char *key, cJSON *value) {
    cJSON_DeleteItemFromObjectCaseSensitive(obj, key); cJSON_AddItemToObject(obj, key, value);
}
static void uuid(char out[37]) {
    uint8_t b[16]; esp_fill_random(b, 16); b[6] = (b[6] & 15) | 64; b[8] = (b[8] & 63) | 128;
    snprintf(out, 37, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
}
static int index_of(cJSON *array, const char *key, const char *value) {
    int i = 0; cJSON *item;
    cJSON_ArrayForEach(item, array) { if(strcmp(str(item, key), value) == 0) return i; ++i; }
    return -1;
}
static void apply(cJSON *array, const char *id, cJSON *task) {
    int index = index_of(array, "id", id);
    if(index >= 0) cJSON_DeleteItemFromArray(array, index);
    if(task) cJSON_AddItemToArray(array, cJSON_Duplicate(task, true));
}
static int compare_tasks(const void *a, const void *b) {
    cJSON *x = *(cJSON **)a, *y = *(cJSON **)b;
    bool xc = strcmp(str(x, "status"), "completed") == 0, yc = strcmp(str(y, "status"), "completed") == 0;
    if(xc != yc) return xc ? 1 : -1;
    const char *xd = str(x, "dueAt"), *yd = str(y, "dueAt");
    int cmp = strcmp(xd[0] ? xd : "9999", yd[0] ? yd : "9999");
    if(cmp) return cmp;
    cmp = strcmp(str(x, "createdAt"), str(y, "createdAt"));
    return cmp ? cmp : strcmp(str(x, "id"), str(y, "id"));
}
static void rebuild(void) {
    cJSON_Delete(s_view); s_view = cJSON_Duplicate(task_array(), true);
    if(!s_view) s_view = cJSON_CreateArray();
    s_status.pending = cJSON_GetArraySize(pending()); s_status.conflicts = 0;
    cJSON *event;
    cJSON_ArrayForEach(event, pending()) {
        if(str(event, "problem")[0]) { s_status.conflicts++; continue; }
        const char *id = str(event, "taskId");
        cJSON *fields = cJSON_GetObjectItemCaseSensitive(event, "task");
        if(strcmp(str(event, "kind"), "delete") == 0) { apply(s_view, id, NULL); continue; }
        cJSON *task = cJSON_Duplicate(fields, true);
        if(!task) continue;
        int old_index = index_of(s_view, "id", id);
        cJSON *old_task = cJSON_GetArrayItem(s_view, old_index);
        char timestamp[32]; time_t now = time(NULL); struct tm utc; gmtime_r(&now, &utc);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S.000Z", &utc);
        cJSON_AddStringToObject(task, "createdAt", str(old_task, "createdAt")[0] ? str(old_task, "createdAt") : timestamp);
        cJSON_AddStringToObject(task, "updatedAt", timestamp);
        cJSON_AddStringToObject(task, "id", id);
        cJSON_AddNumberToObject(task, "taskVersion", num(event, "baseTaskVersion") + 1);
        apply(s_view, id, task); cJSON_Delete(task);
    }
    s_status.count = 0; cJSON *t;
    cJSON_ArrayForEach(t, s_view) { if(s_status.count < HAVIT_TASK_LIMIT) s_order[s_status.count++] = t; }
    qsort(s_order, s_status.count, sizeof(s_order[0]), compare_tasks);
    s_status.revision = num(snapshot(), "revision");
    s_status.utc_offset = num(snapshot(), "utcOffsetSeconds");
    snprintf(s_status.time_zone, sizeof(s_status.time_zone), "%s", str(snapshot(), "timeZone"));
    s_status.ui_revision++;
}
static esp_err_t persist(void) {
    char *raw = cJSON_PrintUnformatted(s_root);
    if(!raw) return ESP_ERR_NO_MEM;
    size_t len = strlen(raw) + 1;
    esp_err_t err = len <= CACHE_CAP ? nvs_set_blob(s_nvs, "state", raw, len) : ESP_ERR_INVALID_SIZE;
    if(err == ESP_OK) err = nvs_commit(s_nvs);
    cJSON_free(raw);
    if(err != ESP_OK) snprintf(s_status.message, sizeof(s_status.message), "No se pudo guardar. Conserva el dispositivo encendido.");
    return err;
}
static void empty_state(const char *binding) {
    cJSON_Delete(s_root); s_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(s_root, "schema", 1); cJSON_AddStringToObject(s_root, "binding", binding);
    cJSON *snap = cJSON_AddObjectToObject(s_root, "snapshot");
    cJSON_AddArrayToObject(snap, "tasks"); cJSON_AddNumberToObject(snap, "revision", 0);
    cJSON_AddArrayToObject(s_root, "pending"); rebuild();
}
esp_err_t havit_tasks_init(void) {
    if(s_lock) return ESP_OK;
    s_lock = xSemaphoreCreateMutex(); if(!s_lock) return ESP_ERR_NO_MEM;
    cJSON_Hooks hooks = {.malloc_fn = json_alloc, .free_fn = heap_caps_free}; cJSON_InitHooks(&hooks);
    setenv("TZ", "UTC0", 1); tzset();
    esp_err_t err = nvs_flash_init_partition("havit_tasks");
    if(err == ESP_OK) err = nvs_open_from_partition("havit_tasks", "tasks", NVS_READWRITE, &s_nvs);
    if(err != ESP_OK) { snprintf(s_status.message, sizeof(s_status.message), "Memoria de actividades no disponible"); return err; }
    size_t len = 0;
    if(nvs_get_blob(s_nvs, "state", NULL, &len) == ESP_OK && len > 0 && len <= CACHE_CAP) {
        char *raw = json_alloc(len);
        if(raw && nvs_get_blob(s_nvs, "state", raw, &len) == ESP_OK) s_root = cJSON_ParseWithLength(raw, len);
        heap_caps_free(raw);
    }
    if(!s_root || num(s_root, "schema") != 1 || !cJSON_IsArray(pending()) || !cJSON_IsArray(task_array()) ||
       cJSON_GetArraySize(task_array()) > HAVIT_TASK_LIMIT || cJSON_GetArraySize(pending()) > 32) empty_state("");
    s_status.ready = true; rebuild();
    snprintf(s_status.message, sizeof(s_status.message), "Conectando actividades…");
    return ESP_OK;
}
esp_err_t havit_tasks_factory_reset(void) {
    if(!s_lock||!s_status.ready)return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_lock,portMAX_DELAY);
    nvs_close(s_nvs);
    esp_err_t err=nvs_flash_erase_partition("havit_tasks");
    if(err==ESP_OK)err=nvs_flash_init_partition("havit_tasks");
    if(err==ESP_OK)err=nvs_open_from_partition("havit_tasks","tasks",NVS_READWRITE,&s_nvs);
    if(err==ESP_OK){
        empty_state("");s_status.ready=true;s_status.online=false;s_status.pending=0;s_status.conflicts=0;
        s_status.message[0]=0;err=persist();rebuild();
    }
    xSemaphoreGive(s_lock);
    return err;
}
void havit_tasks_status(havit_tasks_status_t *out) {
    if(!out) return;
    if(s_lock) xSemaphoreTake(s_lock, portMAX_DELAY);
    *out = s_status; out->clock_valid = time(NULL) > 1760000000;
    if(s_lock) xSemaphoreGive(s_lock);
}
bool havit_tasks_get(int index, havit_task_t *out) {
    if(!s_lock || !out) return false;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if(index < 0 || index >= s_status.count) { xSemaphoreGive(s_lock); return false; }
    cJSON *t = s_order[index]; memset(out, 0, sizeof(*out));
    snprintf(out->id, sizeof(out->id), "%s", str(t, "id"));
    snprintf(out->title, sizeof(out->title), "%s", str(t, "title"));
    snprintf(out->detail, sizeof(out->detail), "%s", str(t, "detail"));
    snprintf(out->due_at, sizeof(out->due_at), "%s", str(t, "dueAt"));
    snprintf(out->created_at, sizeof(out->created_at), "%s", str(t, "createdAt"));
    snprintf(out->updated_at, sizeof(out->updated_at), "%s", str(t, "updatedAt"));
    out->version = num(t, "taskVersion"); out->completed = strcmp(str(t, "status"), "completed") == 0;
    xSemaphoreGive(s_lock); return true;
}
static bool enqueue(const havit_task_t *task, const char *kind) {
    if(!s_lock || !task) return false;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if(!s_status.ready || !str(s_root, "binding")[0] || s_status.pending >= 32 ||
       (strcmp(kind, "create") == 0 && s_status.count >= HAVIT_TASK_LIMIT)) {
        snprintf(s_status.message, sizeof(s_status.message), "Vincula HAVIT o envia los cambios pendientes.");
        xSemaphoreGive(s_lock); return false;
    }
    char event_id[37], task_id[37]; uuid(event_id);
    if(strcmp(kind, "create") == 0) uuid(task_id); else snprintf(task_id, sizeof(task_id), "%s", task->id);
    cJSON *e = cJSON_CreateObject(); cJSON_AddStringToObject(e, "bindingId", str(s_root, "binding"));
    cJSON_AddStringToObject(e, "eventId", event_id); cJSON_AddStringToObject(e, "taskId", task_id);
    cJSON_AddNumberToObject(e, "baseTaskVersion", strcmp(kind, "create") == 0 ? 0 : task->version);
    cJSON_AddStringToObject(e, "kind", kind);
    if(strcmp(kind, "delete") != 0) {
        cJSON *f = cJSON_AddObjectToObject(e, "task");
        cJSON_AddStringToObject(f, "title", task->title); cJSON_AddStringToObject(f, "detail", task->detail);
        if(task->due_at[0]) cJSON_AddStringToObject(f, "dueAt", task->due_at); else cJSON_AddNullToObject(f, "dueAt");
        cJSON_AddStringToObject(f, "status", task->completed ? "completed" : "pending");
        char *encoded = cJSON_PrintUnformatted(f);
        bool fits = encoded && strlen(encoded) <= 2400;
        cJSON_free(encoded);
        if(!fits) {
            cJSON_Delete(e);
            snprintf(s_status.message, sizeof(s_status.message), "Acorta los detalles para guardar la actividad.");
            xSemaphoreGive(s_lock); return false;
        }
    }
    cJSON_AddItemToArray(pending(), e);
    bool ok = persist() == ESP_OK;
    if(!ok) cJSON_DeleteItemFromArray(pending(), cJSON_GetArraySize(pending()) - 1);
    rebuild(); s_force = ok;
    if(ok) snprintf(s_status.message, sizeof(s_status.message), "Cambio guardado. Pendiente de enviar.");
    xSemaphoreGive(s_lock); return ok;
}
bool havit_tasks_save(const havit_task_t *task, bool create) {
    if(!task || !task->title[0] || strlen(task->title) > 160 || strlen(task->detail) > 2048) return false;
    return enqueue(task, create ? "create" : "update");
}
bool havit_tasks_delete(const havit_task_t *task) { return enqueue(task, "delete"); }
void havit_tasks_refresh(void) { if(s_lock) { xSemaphoreTake(s_lock, portMAX_DELAY); s_force = true; xSemaphoreGive(s_lock); } }
static void mark_conflicts(const char *task_id) {
    cJSON *e; cJSON_ArrayForEach(e, pending()) if(strcmp(str(e, "taskId"), task_id) == 0)
        replace(e, "problem", cJSON_CreateString("Cambio en conflicto. Revisa antes de enviar."));
}
void havit_tasks_resolve(bool use_local) {
    if(!s_lock) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    cJSON *first = NULL, *e; cJSON_ArrayForEach(e, pending()) if(str(e, "problem")[0]) { first = e; break; }
    if(first) {
        char id[37]; snprintf(id, sizeof(id), "%s", str(first, "taskId"));
        cJSON *last = NULL;
        for(int i = 0; i < cJSON_GetArraySize(pending());) {
            e = cJSON_GetArrayItem(pending(), i);
            if(strcmp(str(e, "taskId"), id) == 0) { cJSON_Delete(last); last = cJSON_DetachItemFromArray(pending(), i); }
            else ++i;
        }
        if(use_local && last) {
            int index = index_of(task_array(), "id", id); cJSON *current = cJSON_GetArrayItem(task_array(), index);
            if(current || strcmp(str(last, "kind"), "delete") != 0) {
                char event_id[37]; uuid(event_id); replace(last, "eventId", cJSON_CreateString(event_id));
                replace(last, "baseTaskVersion", cJSON_CreateNumber(current ? num(current, "taskVersion") : 0));
                cJSON_DeleteItemFromObjectCaseSensitive(last, "problem");
                if(current && strcmp(str(last, "kind"), "create") == 0) replace(last, "kind", cJSON_CreateString("update"));
                if(!current) { uuid(id); replace(last, "taskId", cJSON_CreateString(id)); replace(last, "kind", cJSON_CreateString("create")); }
                cJSON_AddItemToArray(pending(), last); last = NULL;
            }
        }
        cJSON_Delete(last); persist(); rebuild(); s_force = true;
    }
    xSemaphoreGive(s_lock);
}
static bool valid_snapshot(cJSON *r, const char *binding) {
    if(!r || num(r, "protocol") != 1 || strcmp(str(r, "bindingId"), binding) != 0 ||
       !cJSON_IsArray(cJSON_GetObjectItemCaseSensitive(r, "tasks")) || abs(num(r, "utcOffsetSeconds")) > 50400) return false;
    cJSON *tasks = cJSON_GetObjectItemCaseSensitive(r, "tasks");
    if(cJSON_GetArraySize(tasks) > HAVIT_TASK_LIMIT) return false;
    cJSON *t; cJSON_ArrayForEach(t, tasks) {
        if(strlen(str(t, "id")) != 36 || !str(t, "title")[0] || strlen(str(t, "title")) > 160 ||
           strlen(str(t, "detail")) > 2048 || strlen(str(t, "dueAt")) > 31 || num(t, "taskVersion") < 1 ||
           (strcmp(str(t, "status"), "pending") != 0 && strcmp(str(t, "status"), "completed") != 0)) return false;
    }
    return true;
}

/* Wi-Fi path for a server-authorized factory reset. The task cache is erased
 * before acknowledging the reset; the device credential remains available
 * until the authenticated acknowledgement succeeds. */
static bool handle_wifi_factory_reset(const char *request_id, const char *binding_id, const char *device_id) {
    if(!valid_uuid_text(request_id)||!valid_uuid_text(binding_id)||!device_id||!device_id[0])return false;
    if(havit_tasks_factory_reset()!=ESP_OK)return false;
    cJSON *body=cJSON_CreateObject();
    if(!body)return false;
    cJSON_AddStringToObject(body,"bindingId",binding_id);
    cJSON_AddStringToObject(body,"requestId",request_id);
    char *encoded=cJSON_PrintUnformatted(body);cJSON_Delete(body);
    char *response=json_alloc(2048);
    if(!encoded||!response){cJSON_free(encoded);heap_caps_free(response);return false;}
    int http=0;
    esp_err_t err=havit_device_api_request("/v1/device/factory-reset/complete",encoded,response,2048,&http);
    cJSON_free(encoded);
    cJSON *reply=err==ESP_OK&&http==200?cJSON_Parse(response):NULL;
    bool acknowledged=reply&&strcmp(str(reply,"requestId"),request_id)==0&&
        strcmp(str(reply,"deviceId"),device_id)==0&&strcmp(str(reply,"state"),"completed")==0;
    cJSON_Delete(reply);heap_caps_free(response);
    if(!acknowledged)return false;
    if(havit_device_claim_factory_reset()!=ESP_OK)return false;
    (void)havit_connectivity_finish_factory_reset();
    return true;
}

static esp_err_t task_sync_send_json(uint8_t **out, ssize_t *out_len, cJSON *json) {
    if(!json) return ESP_ERR_NO_MEM;
    char *encoded = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if(!encoded) return ESP_ERR_NO_MEM;
    *out = (uint8_t *)encoded;
    *out_len = (ssize_t)strlen(encoded);
    return ESP_OK;
}

static esp_err_t task_sync_error(uint8_t **out, ssize_t *out_len, const char *code) {
    cJSON *root = cJSON_CreateObject(), *error = cJSON_CreateObject();
    if(!root || !error) { cJSON_Delete(root); cJSON_Delete(error); return ESP_ERR_NO_MEM; }
    cJSON_AddNumberToObject(root, "protocol", 1);
    cJSON_AddStringToObject(error, "code", code);
    cJSON_AddItemToObject(root, "error", error);
    return task_sync_send_json(out, out_len, root);
}

/*
 * Internal snapshot adapter. It is not exposed on BLE. ble_relay.c encrypts
 * requests and authenticates backend responses before calling apply_snapshot.
 * The locally persisted binding is mandatory; ownership cannot change here.
 */
esp_err_t havit_tasks_ble_endpoint(uint32_t session_id, const uint8_t *in, ssize_t len,
                                   uint8_t **out, ssize_t *out_len, void *context) {
    (void)session_id; (void)context;
    if(!in || len <= 0 || len > 220000) return task_sync_error(out, out_len, "INVALID_REQUEST");
    cJSON *request = cJSON_ParseWithLength((const char *)in, (size_t)len);
    cJSON *protocol = cJSON_GetObjectItemCaseSensitive(request, "protocol");
    cJSON *op = cJSON_GetObjectItemCaseSensitive(request, "op");
    if(!request || !cJSON_IsNumber(protocol) || protocol->valuedouble != 1.0 || !cJSON_IsString(op)) {
        cJSON_Delete(request); return task_sync_error(out, out_len, "UNSUPPORTED_PROTOCOL");
    }
    havit_device_claim_snapshot_t claim; havit_device_claim_snapshot(&claim);
    xSemaphoreTake(s_lock, portMAX_DELAY);
    const char *local_binding = str(s_root, "binding");
    if(!s_status.ready || !claim.binding_id[0] || strcmp(local_binding, claim.binding_id) != 0) {
        xSemaphoreGive(s_lock); cJSON_Delete(request);
        return task_sync_error(out, out_len, "DEVICE_NOT_LINKED");
    }
    cJSON *response = cJSON_CreateObject();
    if(!response) { xSemaphoreGive(s_lock); cJSON_Delete(request); return ESP_ERR_NO_MEM; }
    cJSON_AddNumberToObject(response, "protocol", 1);
    cJSON_AddStringToObject(response, "deviceId", claim.device_id);
    cJSON_AddStringToObject(response, "bindingId", local_binding);
    if(strcmp(op->valuestring, "snapshot") == 0) {
        cJSON *snap = cJSON_Duplicate(snapshot(), true);
        cJSON *events = cJSON_Duplicate(pending(), true);
        if(!snap || !events) {
            cJSON_Delete(snap); cJSON_Delete(events); cJSON_Delete(response);
            xSemaphoreGive(s_lock); cJSON_Delete(request); return ESP_ERR_NO_MEM;
        }
        cJSON_AddItemToObject(response, "snapshot", snap);
        cJSON_AddItemToObject(response, "pending", events);
    } else if(strcmp(op->valuestring, "apply_snapshot") == 0) {
        cJSON *incoming = cJSON_GetObjectItemCaseSensitive(request, "snapshot");
        cJSON *acks = cJSON_GetObjectItemCaseSensitive(request, "ackEventIds");
        cJSON *conflicts = cJSON_GetObjectItemCaseSensitive(request, "conflictEventIds");
        if(!valid_snapshot(incoming, local_binding) || !cJSON_IsArray(acks) ||
           strcmp(str(incoming, "deviceId"), claim.device_id) != 0 ||
           (conflicts && (!cJSON_IsArray(conflicts) || cJSON_GetArraySize(conflicts) > 32)) ||
           cJSON_GetArraySize(acks) > 32 || num(incoming, "revision") < num(snapshot(), "revision")) {
            cJSON_Delete(response); xSemaphoreGive(s_lock); cJSON_Delete(request);
            return task_sync_error(out, out_len, "INVALID_SNAPSHOT");
        }
        cJSON *ack;
        cJSON_ArrayForEach(ack, acks) {
            if(!cJSON_IsString(ack) || strlen(ack->valuestring) != 36) {
                cJSON_Delete(response); xSemaphoreGive(s_lock); cJSON_Delete(request);
                return task_sync_error(out, out_len, "INVALID_ACK");
            }
        }
        cJSON_ArrayForEach(ack, conflicts) {
            if(!cJSON_IsString(ack) || strlen(ack->valuestring) != 36) {
                cJSON_Delete(response); xSemaphoreGive(s_lock); cJSON_Delete(request);
                return task_sync_error(out, out_len, "INVALID_ACK");
            }
        }
        cJSON *rollback = cJSON_Duplicate(s_root, true);
        cJSON *copy = cJSON_Duplicate(incoming, true);
        if(!rollback || !copy) {
            cJSON_Delete(rollback); cJSON_Delete(copy); cJSON_Delete(response);
            xSemaphoreGive(s_lock); cJSON_Delete(request); return ESP_ERR_NO_MEM;
        }
        replace(s_root, "snapshot", copy);
        cJSON *failed;
        cJSON_ArrayForEach(failed, conflicts) {
            int at = index_of(pending(), "eventId", failed->valuestring);
            cJSON *event = cJSON_GetArrayItem(pending(), at);
            if(event) mark_conflicts(str(event, "taskId"));
        }
        for(int i = 0; i < cJSON_GetArraySize(pending());) {
            cJSON *event = cJSON_GetArrayItem(pending(), i); bool acknowledged = false;
            cJSON_ArrayForEach(ack, acks) {
                if(strcmp(str(event, "eventId"), ack->valuestring) == 0) { acknowledged = true; break; }
            }
            if(acknowledged) cJSON_DeleteItemFromArray(pending(), i); else ++i;
        }
        bool cache_changed = cJSON_GetArraySize(acks) > 0 || cJSON_GetArraySize(conflicts) > 0 ||
            num(incoming,"revision") != num(cJSON_GetObjectItemCaseSensitive(rollback,"snapshot"),"revision") ||
            strcmp(str(incoming,"timeZone"),str(cJSON_GetObjectItemCaseSensitive(rollback,"snapshot"),"timeZone")) != 0 ||
            num(incoming,"utcOffsetSeconds") != num(cJSON_GetObjectItemCaseSensitive(rollback,"snapshot"),"utcOffsetSeconds");
        if(cache_changed && persist() != ESP_OK) {
            cJSON_Delete(s_root); s_root = rollback; rollback = NULL; rebuild();
            cJSON_Delete(response); xSemaphoreGive(s_lock); cJSON_Delete(request);
            return task_sync_error(out, out_len, "PERSISTENCE_FAILED");
        }
        cJSON_Delete(rollback); rebuild(); s_force = true;
        s_ble_confirmed_until = esp_timer_get_time() + 30000000LL;
        s_status.online = true;
        snprintf(s_status.message, sizeof(s_status.message), s_status.conflicts ? "Hay cambios en conflicto. Pulsa Revisar." : "Actividades actualizadas por Bluetooth");
        int year,month,day,hour,minute,second;
        if(sscanf(str(incoming,"serverTime"),"%d-%d-%dT%d:%d:%d",&year,&month,&day,&hour,&minute,&second)==6 && year>=2020 && year<2100) {
            struct tm utc={.tm_year=year-1900,.tm_mon=month-1,.tm_mday=day,.tm_hour=hour,.tm_min=minute,.tm_sec=second};
            struct timeval tv={.tv_sec=mktime(&utc)}; settimeofday(&tv,NULL);
        }
        cJSON_AddBoolToObject(response, "applied", true);
    } else {
        cJSON_Delete(response); xSemaphoreGive(s_lock); cJSON_Delete(request);
        return task_sync_error(out, out_len, "UNSUPPORTED_OPERATION");
    }
    xSemaphoreGive(s_lock); cJSON_Delete(request);
    return task_sync_send_json(out, out_len, response);
}

void havit_tasks_tick(bool wifi_connected) {
    if(!s_lock || !s_status.ready) return;
    havit_device_claim_snapshot_t claim; havit_device_claim_snapshot(&claim);
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if(strcmp(str(s_root, "binding"), claim.binding_id) != 0) {
        empty_state(claim.binding_id); persist(); s_force = true;
    }
    if(!wifi_connected || !claim.account_linked) {
        if(esp_timer_get_time() < s_ble_confirmed_until) { xSemaphoreGive(s_lock); return; }
        s_status.online = false;
        snprintf(s_status.message, sizeof(s_status.message), claim.binding_id[0] ? "Sin conexion. Actividades guardadas en HAVIT." : "Vincula tu cuenta para compartir actividades.");
        xSemaphoreGive(s_lock); return;
    }
    int64_t now = esp_timer_get_time();
    if(now < s_next_attempt && !s_force) { xSemaphoreGive(s_lock); return; }
    s_force = false; s_next_attempt = now + 15000000LL;
    cJSON *send = NULL, *e;
    cJSON_ArrayForEach(e, pending()) if(!str(e, "problem")[0]) { send = cJSON_Duplicate(e, true); break; }
    xSemaphoreGive(s_lock);
    char *response = json_alloc(RESPONSE_CAP);
    if(!response) { cJSON_Delete(send); return; }
    int http = 0; esp_err_t err = ESP_OK;
    if(send) {
        char *body = cJSON_PrintUnformatted(send);
        err = body ? havit_device_api_request("/v1/device/events", body, response, RESPONSE_CAP, &http) : ESP_ERR_NO_MEM;
        cJSON_free(body);
        xSemaphoreTake(s_lock, portMAX_DELAY);
        int index = index_of(pending(), "eventId", str(send, "eventId"));
        if(err == ESP_OK && http == 200 && index >= 0) {
            cJSON *result = cJSON_Parse(response), *task = cJSON_GetObjectItemCaseSensitive(result, "task");
            bool deleting = strcmp(str(send, "kind"), "delete") == 0;
            bool valid_result = deleting ? cJSON_IsNull(task) : cJSON_IsObject(task) &&
                strcmp(str(task, "id"), str(send, "taskId")) == 0 && num(task, "taskVersion") > num(send, "baseTaskVersion");
            if(result && valid_result && strcmp(str(result, "id"), str(send, "taskId")) == 0) {
                apply(task_array(), str(send, "taskId"), cJSON_IsObject(task) ? task : NULL);
                cJSON *old = cJSON_DetachItemFromArray(pending(), index);
                if(persist() != ESP_OK) cJSON_InsertItemInArray(pending(), index, old); else cJSON_Delete(old);
            }
            cJSON_Delete(result);
        } else if(err == ESP_OK && (http == 400 || http == 409 || http == 403)) {
            cJSON *r = cJSON_Parse(response), *error = cJSON_GetObjectItemCaseSensitive(r, "error");
            const char *code = str(error, "code");
            if(strcmp(code, "BINDING_CHANGED") == 0 || strcmp(code, "DEVICE_NOT_LINKED") == 0) {
                empty_state(""); persist();
            } else { mark_conflicts(str(send, "taskId")); persist(); }
            cJSON_Delete(r);
        }
        if(http == 429) s_next_attempt = esp_timer_get_time() + 60000000LL;
        rebuild(); xSemaphoreGive(s_lock); cJSON_Delete(send);
    }
    char path[112]; snprintf(path, sizeof(path), "/v1/device/sync?bindingId=%s", claim.binding_id);
    err = havit_device_api_request(path, NULL, response, RESPONSE_CAP, &http);
    cJSON *reply = err == ESP_OK && http == 200 ? cJSON_Parse(response) : NULL;
    char reset_id[37]={0};
    const char *server_reset_id=str(reply,"factoryResetRequestId");
    if(valid_uuid_text(server_reset_id))snprintf(reset_id,sizeof(reset_id),"%s",server_reset_id);
    if(reset_id[0]){
        cJSON_Delete(reply);heap_caps_free(response);cJSON_Delete(send);
        if(handle_wifi_factory_reset(reset_id,claim.binding_id,claim.device_id))return;
        xSemaphoreTake(s_lock,portMAX_DELAY);
        s_status.online=false;
        snprintf(s_status.message,sizeof(s_status.message),"No se pudo completar el reinicio por Wi-Fi. HAVIT conserva la solicitud para reintentar.");
        xSemaphoreGive(s_lock);
        return;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if(valid_snapshot(reply, claim.binding_id)) {
        bool changed = num(reply, "revision") != num(snapshot(), "revision") ||
            num(reply, "utcOffsetSeconds") != num(snapshot(), "utcOffsetSeconds") ||
            strcmp(str(reply, "timeZone"), str(snapshot(), "timeZone")) != 0;
        replace(s_root, "snapshot", reply); reply = NULL; s_status.online = true;
        if(changed) { persist(); rebuild(); }
        snprintf(s_status.message, sizeof(s_status.message), s_status.conflicts ? "Hay cambios en conflicto. Pulsa Revisar." : s_status.pending ? "%d cambios pendientes de enviar" : "Actividades actualizadas por Wi-Fi", s_status.pending);
    } else {
        s_status.online = false;
        snprintf(s_status.message, sizeof(s_status.message), "Sin respuesta del servidor. Cambios guardados.");
        if(http == 429) s_next_attempt = esp_timer_get_time() + 60000000LL;
    }
    cJSON_Delete(reply); heap_caps_free(response); xSemaphoreGive(s_lock);
}
static int offset_at(time_t utc) {
    if(!s_lock) return 0;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    int offset = num(snapshot(), "utcOffsetSeconds");
    cJSON *entry;
    cJSON_ArrayForEach(entry, cJSON_GetObjectItemCaseSensitive(snapshot(), "utcOffsetTransitions")) {
        cJSON *at = cJSON_GetObjectItemCaseSensitive(entry, "at");
        if(cJSON_IsNumber(at) && at->valuedouble <= (double)utc) offset = num(entry, "offset");
        else break;
    }
    xSemaphoreGive(s_lock); return offset;
}
void havit_tasks_local_time(struct tm *out) {
    time_t utc = time(NULL); utc += offset_at(utc); gmtime_r(&utc, out);
}
static time_t parse_utc(const char *utc) {
    struct tm tm = {0}; int y, m, d, h, min, sec;
    if(sscanf(utc, "%d-%d-%dT%d:%d:%d", &y, &m, &d, &h, &min, &sec) != 6) return 0;
    tm.tm_year = y - 1900; tm.tm_mon = m - 1; tm.tm_mday = d; tm.tm_hour = h; tm.tm_min = min; tm.tm_sec = sec;
    return mktime(&tm);
}
void havit_tasks_date(const char *utc, char out[11]) {
    out[0] = 0; time_t date = parse_utc(utc); if(!date) return;
    date += offset_at(date);
    struct tm tm; gmtime_r(&date, &tm); strftime(out, 11, "%Y-%m-%d", &tm);
}
bool havit_tasks_due_from_local(const char *local, char out[32]) {
    out[0] = 0; if(!local[0]) return true;
    int y, m, d, h, min; char extra;
    if(sscanf(local, "%d-%d-%d %d:%d%c", &y, &m, &d, &h, &min, &extra) != 5 || y < 2020 || y > 2099 || m < 1 || m > 12 || d < 1 || d > 31 || h < 0 || h > 23 || min < 0 || min > 59) return false;
    struct tm tm = {.tm_year = y - 1900, .tm_mon = m - 1, .tm_mday = d, .tm_hour = h, .tm_min = min};
    time_t date = mktime(&tm);
    if(tm.tm_year != y - 1900 || tm.tm_mon != m - 1 || tm.tm_mday != d) return false;
    havit_tasks_status_t status; havit_tasks_status(&status);
    if(!status.clock_valid || !status.time_zone[0]) return false;
    time_t local_epoch = date;
    date = local_epoch - offset_at(local_epoch);
    date = local_epoch - offset_at(date);
    // A nonexistent hour at a daylight-saving jump must not silently change.
    if(date + offset_at(date) != local_epoch) return false;
    gmtime_r(&date, &tm); strftime(out, 32, "%Y-%m-%dT%H:%M:%S.000Z", &tm); return true;
}
void havit_tasks_due_local(const char *utc, char out[24]) {
    out[0] = 0; time_t date = parse_utc(utc); if(!date) return;
    date += offset_at(date); struct tm tm; gmtime_r(&date, &tm);
    strftime(out, 24, "%Y-%m-%d %H:%M", &tm);
}
