#include "esp_display_panel.hpp"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "draw/sw/lv_draw_sw_utils.h"
#include "core/lv_refr.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi_config.h"
#include "ui_gen.h"
#include "ui_ring_overlay.h"
#include "src/libs/fsdrv/lv_fsdrv.h"

#include <cstdint>
#include <cstdarg>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <atomic>
#include <initializer_list>
#include <ctime>

extern "C" {
#include "havit_app.h"
#include "havit_nvs_store.h"
void havit_ui_fonts_init(void);
// Lifecycle functions provided by the pinned esp_wifi_config component.
void wifi_cfg_start_provisioning(void);
void wifi_cfg_stop_provisioning(void);
}

using esp_panel::board::Board;
using esp_panel::drivers::LCD;
using esp_panel::drivers::Touch;
using esp_panel::drivers::TouchPoint;

extern "C" {
extern const uint8_t ring1_data[] asm("_binary_ring1_rgb565a8_start");
extern const uint8_t ring1_end[] asm("_binary_ring1_rgb565a8_end");
extern const uint8_t ring2_data[] asm("_binary_ring2_rgb565a8_start");
extern const uint8_t ring2_end[] asm("_binary_ring2_rgb565a8_end");
extern const uint8_t ring3_data[] asm("_binary_ring3_rgb565a8_start");
extern const uint8_t ring3_end[] asm("_binary_ring3_rgb565a8_end");
extern const uint8_t google_data[] asm("_binary_google_logo_rgb565a8_start");
extern const uint8_t google_end[] asm("_binary_google_logo_rgb565a8_end");
extern const uint8_t separator_data[] asm("_binary_separator_rgb565a8_start");
extern const uint8_t separator_end[] asm("_binary_separator_rgb565a8_end");
extern const uint8_t pill_data[] asm("_binary_continue_pill_rgb565a8_start");
extern const uint8_t pill_end[] asm("_binary_continue_pill_rgb565a8_end");
extern const uint8_t arrow_data[] asm("_binary_arrow_rgb565a8_start");
extern const uint8_t arrow_end[] asm("_binary_arrow_rgb565a8_end");
extern const uint8_t arrow_welcome_data[] asm("_binary_arrow_welcome_rgb565a8_start");
extern const uint8_t arrow_welcome_end[] asm("_binary_arrow_welcome_rgb565a8_end");
}

namespace {

constexpr int SCREEN_SIZE = 360;
constexpr char HAVIT_API_BASE[] = "https://us-central1-havit-c8c1d.cloudfunctions.net";
constexpr uint32_t INTRO_BLINK_MS = 130;
constexpr uint32_t INTRO_BLINK_INTERVAL_MS = 1850;
constexpr uint32_t UI_BACKGROUND = 0x161621;
constexpr uint32_t WHITE = 0xFFFFFF;
constexpr uint32_t MUTED = 0xFFFFFF;
constexpr uint32_t ACCENT = 0x5250F3;
constexpr uint32_t CIRCLE_BUTTON = 0xD9D9D9;
constexpr size_t LOCAL_TASK_CAPACITY = 16;

const char *TAG = "havit_os";
LCD *s_lcd = nullptr;
Touch *s_touch = nullptr;
havit_app_t s_app{};
std::atomic_bool s_wifi_connected{false};
bool s_pairing_view = false;
bool s_wifi_setup_view = false;
std::atomic_bool s_pairing_cancelled{false};
std::atomic_bool s_pairing_running{false};
std::atomic_bool s_pairing_requested{false};
std::atomic_bool s_ui_dirty{false};
std::atomic_bool s_connection_widgets_dirty{false};
std::atomic_bool s_pairing_widgets_dirty{false};
std::atomic_bool s_intro_blink_active{false};
std::atomic_bool s_ble_available{false};
std::atomic_bool s_account_ready{false};
bool s_initial_account_sync_pending = true;
std::atomic_bool s_touch_pending{false};
std::atomic_int s_http_status{0};
std::atomic_int s_network_command{0};
std::atomic_bool s_network_running{false};
bool s_forget_confirm = false;
bool s_manual_wifi_view = false;
bool s_resume_pairing = false;
char s_network_name[33] = "";
char s_network_ip[16] = "";
portMUX_TYPE s_text_lock = portMUX_INITIALIZER_UNLOCKED;
char s_setup_ssid[33] = "HAVIT-SETUP";
char s_setup_password[13] = "";
char s_device_id[40] = "";
char s_device_token[96] = "";
char s_pairing_id[48] = "";
char s_pairing_code[16] = "";
char s_pairing_token[96] = "";
char s_pairing_message[128] = "Preparando el código…";
char s_display_name[HAVIT_USER_NAME_MAX] = "";
uint32_t s_synced_task_count = 0;
struct LocalTask {
    char title[48];
    char category[20];
    char due_date[12];
    uint8_t completed;
    uint8_t reserved[7];
    int64_t created_at_us;
};
struct LocalTaskStore {
    uint32_t version;
    uint32_t count;
    LocalTask tasks[LOCAL_TASK_CAPACITY];
};
LocalTaskStore s_local_tasks{};
enum class AppView : uint8_t {
    Today, AccountSettings, AddMenu, TaskList, QuickForm, QuickTimer, QuickComplete,
    ProjectForm, ProjectDetails, ProjectSchedule, ProjectGenerate,
    ProjectTimeline, ProjectCalendar, ProjectBreakdown,
    ProjectNotifications, ProjectSaved, AgendaDay, AgendaWeek
};
struct CachedPrototypeViews {
    lv_obj_t *root = nullptr;
    lv_obj_t *ring = nullptr;
    lv_obj_t *today = nullptr;
    lv_obj_t *today_count_label = nullptr;
    lv_obj_t *today_date_label = nullptr;
    lv_obj_t *quick_form = nullptr;
    lv_obj_t *quick_timer = nullptr;
    lv_obj_t *quick_name_input = nullptr;
    lv_obj_t *quick_duration_select = nullptr;
    lv_obj_t *quick_keyboard = nullptr;
    lv_obj_t *timer_label = nullptr;
    lv_obj_t *timer_title_label = nullptr;
};
CachedPrototypeViews s_prototype_views{};
AppView s_active_prototype_view = AppView::Today;
bool s_quick_form_reset_requested = false;
uint8_t s_project_calendar_month = 10;
uint16_t s_project_calendar_year = 2026;
uint8_t s_project_calendar_selected_day = 0;
uint8_t s_calendar_month = 0;
uint16_t s_calendar_year = 0;
uint8_t s_calendar_selected_day = 0;
bool s_calendar_initialized = false;
bool s_task_completed_filter = false;
char s_task_date_filter[32] = "";
AppView s_app_view = AppView::Today;
AppView s_previous_app_view = AppView::Today;
lv_obj_t *s_quick_name_input = nullptr;
lv_obj_t *s_quick_duration_select = nullptr;
lv_obj_t *s_project_name_input = nullptr;
lv_obj_t *s_project_subject_input = nullptr;
lv_obj_t *s_project_description_input = nullptr;
lv_obj_t *s_project_due_input = nullptr;
lv_obj_t *s_project_start_input = nullptr;
lv_obj_t *s_keyboard = nullptr;
lv_obj_t *s_timer_label = nullptr;
lv_obj_t *s_quick_timer_title_label = nullptr;
lv_obj_t *s_project_notice_sound = nullptr;
lv_obj_t *s_project_notice_light = nullptr;
lv_obj_t *s_project_notice_tone = nullptr;
lv_timer_t *s_quick_timer = nullptr;
char s_quick_name[48] = "";
char s_project_name[48] = "";
char s_project_subject[20] = "";
char s_project_description[96] = "";
char s_project_due[24] = "";
char s_project_start[24] = "";
uint16_t s_task_list_page = 0;
static constexpr uint32_t TASK_PAGE_SIZE = 2;
uint32_t s_task_page_count = 1;
struct TaskListWidgets {
    lv_obj_t *count = nullptr;
    lv_obj_t *pending_card = nullptr;
    lv_obj_t *pending_label = nullptr;
    lv_obj_t *completed_card = nullptr;
    lv_obj_t *completed_label = nullptr;
    lv_obj_t *date_label = nullptr;
    lv_obj_t *date_clear = nullptr;
    lv_obj_t *rows = nullptr;
    lv_obj_t *previous = nullptr;
    lv_obj_t *page = nullptr;
    lv_obj_t *next = nullptr;
};
struct CalendarDayWidgets {
    lv_obj_t *button = nullptr;
    lv_obj_t *label = nullptr;
    lv_obj_t *marker = nullptr;
};
struct ProjectCalendarDayWidgets {
    lv_obj_t *button = nullptr;
    lv_obj_t *label = nullptr;
};
TaskListWidgets s_task_list_widgets{};
CalendarDayWidgets s_calendar_day_widgets[31]{};
ProjectCalendarDayWidgets s_project_calendar_day_widgets[31]{};
lv_obj_t *s_calendar_month_label = nullptr;
lv_obj_t *s_calendar_selected_label = nullptr;
lv_obj_t *s_calendar_task_label = nullptr;
lv_obj_t *s_project_calendar_month_label = nullptr;
lv_obj_t *s_project_calendar_year_label = nullptr;
lv_timer_t *s_welcome_timer = nullptr;
bool s_ui_feedback_pending = false;
int64_t s_ui_feedback_started_us = 0;
int64_t s_touch_detected_us = 0;
int64_t s_action_touch_us = 0;
int64_t s_ui_navigation_started_us = 0;
int64_t s_ui_render_started_us = 0;
int64_t s_ui_render_ready_us = 0;
uint32_t s_ui_build_duration_us = 0;
std::atomic_bool s_ui_measurement_pending{false};
std::atomic_bool s_final_display_flush_in_flight{false};
std::atomic_bool s_final_display_flush_queued{false};
std::atomic<uint32_t> s_display_flushes_in_flight{0};
std::atomic_bool s_display_completion_pending{false};
std::atomic<uint32_t> s_display_completed_us_low{0};
uint32_t s_quick_duration_seconds = 1200;
int64_t s_quick_deadline_us = 0;
int64_t s_quick_paused_remaining_us = 0;
bool s_quick_running = false;
bool s_quick_paused = false;
bool s_quick_expired = false;
bool s_project_saved = false;
bool s_project_dates_invalid = false;
uint8_t s_notice_sound = 1;
uint8_t s_notice_light = 0;
uint8_t s_notice_tone = 0;
static nvs_handle_t s_havit_store_handle;
bool s_lvgl_ready = false;
const lv_font_t *s_font_15 = nullptr;
const lv_font_t *s_font_20 = nullptr;
const lv_font_t *s_font_22 = nullptr;
const lv_font_t *s_font_24 = nullptr;
lv_obj_t *s_home_wifi_label = nullptr;
lv_obj_t *s_wifi_status_label = nullptr;
lv_obj_t *s_wifi_ssid_label = nullptr;
lv_obj_t *s_wifi_ip_label = nullptr;
lv_obj_t *s_wifi_ble_status_label = nullptr;
lv_obj_t *s_wifi_ble_button_label = nullptr;
lv_obj_t *s_wifi_toggle_button_label = nullptr;
lv_obj_t *s_pairing_status_label = nullptr;
lv_obj_t *s_pairing_network_label = nullptr;
lv_obj_t *s_pairing_code_label = nullptr;
lv_obj_t *s_pairing_message_label = nullptr;
lv_obj_t *s_pairing_button_label = nullptr;

lv_image_dsc_t s_ring1{};
lv_image_dsc_t s_ring2{};
lv_image_dsc_t s_ring3{};
lv_image_dsc_t s_google{};
lv_image_dsc_t s_separator{};
lv_image_dsc_t s_pill{};
lv_image_dsc_t s_arrow{};
lv_image_dsc_t s_arrow_welcome{};

enum class UiAction : uintptr_t {
    ContinueOnboarding = 1,
    PairDevice,
    ContinueHome,
    ShowWifiSetup,
    CloseOverlay,
    RetryPairing,
    EnableBluetooth,
    DisableBluetooth,
    ForgetWifi,
    ConfirmForgetWifi,
    ToggleWifi,
    ToggleBluetooth,
    ShowManualWifi,
    OpenConnections,
    OpenSettings,
    OpenAddMenu,
    OpenTaskList,
    OpenAgendaDay,
    OpenAgendaWeek,
    OpenCalendar,
    OpenTasksForSelectedDay,
    ClearTaskDateFilter,
    ToggleTaskFilter,
    OpenQuickForm,
    OpenQuickTimer,
    OpenProjectForm,
    OpenProjectDetails,
    QuickStart,
    QuickPauseResume,
    QuickComplete,
    QuickCancel,
    SaveProject,
    SaveProjectConfirm,
    GoBack,
    PreviousTaskPage,
    NextTaskPage,
    ToggleTask,
    OpenNotifications,
    OpenProjectSchedule,
    GenerateProjectTimeline,
    OpenProjectCalendar,
    OpenProjectBreakdown,
    ShowQuickComplete,
    NextProjectMonth,
    PreviousProjectMonth,
    NextCalendarMonth,
    PreviousCalendarMonth,
};

static void render_screen(lv_obj_t *screen, havit_screen_t screen_id);
static void render_async(void *);
static void finish_render_and_load(lv_obj_t *screen, bool load_screen);
static void set_pairing_message(const char *format, ...);
static void start_pairing_task(void *);
static void quick_timer_tick(lv_timer_t *timer);
static void add_action_area(lv_obj_t *screen, int x, int y, int width, int height, UiAction action);
static void update_task_list_view(void);
static void update_calendar_view(void);
static void update_project_calendar_view(void);
static void welcome_timeout(lv_timer_t *timer);
static void persist_local_tasks();
static bool valid_project_date(const char *value);
static bool is_prototype_view(AppView view);
static void show_prototype_view(AppView view, lv_obj_t *initial_root = nullptr);
static void display_render_event(lv_event_t *event);
static void log_ui_heap(const char *stage);
static void begin_ui_measurement(void);

static void load_local_tasks()
{
    size_t size = sizeof(s_local_tasks);
    if(nvs_get_blob(s_havit_store_handle, "local_tasks", &s_local_tasks, &size) != ESP_OK ||
       size != sizeof(s_local_tasks) || s_local_tasks.version != 1 || s_local_tasks.count > LOCAL_TASK_CAPACITY) {
        std::memset(&s_local_tasks, 0, sizeof(s_local_tasks));
        s_local_tasks.version = 1;
    }
}

static void persist_local_tasks()
{
    s_local_tasks.version = 1;
    if(nvs_set_blob(s_havit_store_handle, "local_tasks", &s_local_tasks, sizeof(s_local_tasks)) == ESP_OK) {
        (void)nvs_commit(s_havit_store_handle);
    }
}

static bool add_local_task(const char *title, const char *category, const char *due_date = nullptr)
{
    if(title == nullptr || title[0] == '\0' || s_local_tasks.count >= LOCAL_TASK_CAPACITY) return false;
    LocalTask &task = s_local_tasks.tasks[s_local_tasks.count++];
    std::memset(&task, 0, sizeof(task));
    std::snprintf(task.title, sizeof(task.title), "%s", title);
    std::snprintf(task.category, sizeof(task.category), "%s", category ? category : "Tarea");
    if(due_date) std::snprintf(task.due_date, sizeof(task.due_date), "%s", due_date);
    task.created_at_us = esp_timer_get_time();
    persist_local_tasks();
    return true;
}
static void on_wifi_config_event(void *, esp_event_base_t, int32_t event_id, void *)
{
    bool provisioning_state_changed = false;
    if(event_id == WIFI_CFG_EVENT_GOT_IP) {
        s_wifi_connected = true;
        ESP_LOGI(TAG, "Wi-Fi has an IP; requesting account pairing/sync");
        s_pairing_requested = true;
    } else if(event_id == WIFI_CFG_EVENT_DISCONNECTED || event_id == WIFI_CFG_EVENT_LOST_IP) {
        s_wifi_connected = false;
        s_pairing_cancelled = true;
        set_pairing_message("Wi-Fi desconectado. Abre Conexiones.");
    } else if(event_id == WIFI_CFG_EVENT_PROVISIONING_STARTED) {
        s_ble_available = true;
        provisioning_state_changed = true;
    } else if(event_id == WIFI_CFG_EVENT_PROVISIONING_STOPPED) {
        s_ble_available = false;
        provisioning_state_changed = true;
    }
    s_connection_widgets_dirty = true;
    // Only rebuild settings when the BLE button's available action changes.
    if(provisioning_state_changed && s_wifi_setup_view && !s_manual_wifi_view && !s_forget_confirm) {
        s_ui_dirty = true;
    }
}

static void start_wifi_services(nvs_handle_t store)
{
    uint8_t mac[6] = {};
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
    std::snprintf(s_device_id, sizeof(s_device_id), "%02X%02X%02X%02X%02X%02X",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    std::snprintf(s_setup_ssid, sizeof(s_setup_ssid), "HAVIT-SETUP-%02X%02X", mac[4], mac[5]);

    size_t password_length = sizeof(s_setup_password);
    if(nvs_get_str(store, "setup_pass", s_setup_password, &password_length) != ESP_OK ||
       std::strlen(s_setup_password) < 8) {
        for(int i = 0; i < 8; ++i) s_setup_password[i] = static_cast<char>('0' + (esp_random() % 10));
        s_setup_password[8] = '\0';
        ESP_ERROR_CHECK(nvs_set_str(store, "setup_pass", s_setup_password));
        ESP_ERROR_CHECK(nvs_commit(store));
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_CFG_EVENT, ESP_EVENT_ANY_ID,
                                               &on_wifi_config_event, nullptr));
    wifi_cfg_config_t config = WIFI_CFG_DEFAULT_CONFIG();
    config.provisioning_mode = WIFI_PROV_ON_FAILURE;
    config.stop_provisioning_on_connect = true;
    config.provisioning_teardown_delay_ms = 5000;
    config.enable_ap = true;
    config.default_ap.ssid[0] = '\0';
    std::snprintf(config.default_ap.ssid, sizeof(config.default_ap.ssid), "%s", s_setup_ssid);
    std::snprintf(config.default_ap.password, sizeof(config.default_ap.password), "%s", s_setup_password);
    config.improv.firmware_name = "HAVIT OS";
    config.improv.firmware_version = "0.1.0";
    config.improv.device_name = "HAVIT";
    config.improv.ble_device_name = "HAVIT-{id}";
    ESP_ERROR_CHECK(wifi_cfg_init(&config));
    s_wifi_connected = wifi_cfg_is_connected();
    ESP_LOGI(TAG, "Wi-Fi setup ready: Improv BLE + fallback AP %s", s_setup_ssid);
}

struct HttpReply {
    static constexpr size_t capacity = 3072;
    char *body = nullptr;
    size_t length = 0;
    HttpReply() {
        body = static_cast<char *>(heap_caps_calloc(1, capacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        if(body == nullptr) body = static_cast<char *>(std::calloc(1, capacity));
    }
    ~HttpReply() { std::free(body); }
    HttpReply(const HttpReply &) = delete;
    HttpReply &operator=(const HttpReply &) = delete;
};

static esp_err_t capture_http_body(esp_http_client_event_t *event)
{
    if(event->event_id == HTTP_EVENT_ON_DATA && event->user_data != nullptr && event->data != nullptr) {
        auto *reply = static_cast<HttpReply *>(event->user_data);
        const size_t available = HttpReply::capacity - 1 - reply->length;
        const size_t copy_size = static_cast<size_t>(event->data_len) < available
                                     ? static_cast<size_t>(event->data_len)
                                     : available;
        std::memcpy(reply->body + reply->length, event->data, copy_size);
        reply->length += copy_size;
        reply->body[reply->length] = '\0';
    }
    return ESP_OK;
}

static bool https_request(const char *url, esp_http_client_method_t method,
                          const char *body, const char *token, HttpReply *reply)
{
    if(reply == nullptr || reply->body == nullptr) { s_http_status = 0; return false; }
    esp_http_client_config_t config = {};
    config.url = url;
    config.method = method;
    config.timeout_ms = 9000;
    config.event_handler = capture_http_body;
    config.user_data = reply;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if(client == nullptr) return false;
    esp_http_client_set_header(client, "Accept", "application/json");
    if(method == HTTP_METHOD_POST) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        if(body != nullptr) esp_http_client_set_post_field(client, body, std::strlen(body));
    }
    if(token != nullptr && token[0] != '\0') {
        char authorization[128];
        std::snprintf(authorization, sizeof(authorization), "Bearer %s", token);
        esp_http_client_set_header(client, "Authorization", authorization);
    }
    const esp_err_t result = esp_http_client_perform(client);
    const int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    s_http_status = result == ESP_OK ? status : 0;
    if(result != ESP_OK || status < 200 || status >= 300) {
        ESP_LOGW(TAG, "Cloud request failed: transport=%s HTTP=%d", esp_err_to_name(result), status);
    }
    return result == ESP_OK && status >= 200 && status < 300;
}

static bool sync_watch_account(const char *token)
{
    HttpReply reply;
    if(!https_request((std::string(HAVIT_API_BASE) + "/syncWatch").c_str(), HTTP_METHOD_GET, nullptr, token, &reply)) return false;
    cJSON *root = cJSON_Parse(reply.body);
    if(root == nullptr) return false;
    cJSON *profile = cJSON_GetObjectItemCaseSensitive(root, "profile");
    cJSON *name = cJSON_GetObjectItemCaseSensitive(profile, "displayName");
    cJSON *tasks = cJSON_GetObjectItemCaseSensitive(root, "tasks");
    if(cJSON_IsString(name) && name->valuestring != nullptr) {
        std::snprintf(s_display_name, sizeof(s_display_name), "%s", name->valuestring);
    }
    s_synced_task_count = cJSON_IsArray(tasks) ? static_cast<uint32_t>(cJSON_GetArraySize(tasks)) : 0;
    cJSON_Delete(root);
    return true;
}

static void log_ui_heap(const char *stage)
{
    constexpr uint32_t INTERNAL_CAPS = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
    constexpr uint32_t PSRAM_CAPS = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
    const size_t internal_free = heap_caps_get_free_size(INTERNAL_CAPS);
    const size_t internal_total = heap_caps_get_total_size(INTERNAL_CAPS);
    const size_t internal_largest = heap_caps_get_largest_free_block(INTERNAL_CAPS);
    const size_t psram_free = heap_caps_get_free_size(PSRAM_CAPS);
    const size_t psram_total = heap_caps_get_total_size(PSRAM_CAPS);
    const size_t psram_largest = heap_caps_get_largest_free_block(PSRAM_CAPS);
    const size_t internal_gap = internal_free > internal_largest ? internal_free - internal_largest : 0;
    const size_t psram_gap = psram_free > psram_largest ? psram_free - psram_largest : 0;
    ESP_LOGI(TAG, "UI heap[%s]: internal=%lu/%lu largest=%lu gap=%lu; psram=%lu/%lu largest=%lu gap=%lu",
             stage,
             static_cast<unsigned long>(internal_free), static_cast<unsigned long>(internal_total),
             static_cast<unsigned long>(internal_largest), static_cast<unsigned long>(internal_gap),
             static_cast<unsigned long>(psram_free), static_cast<unsigned long>(psram_total),
             static_cast<unsigned long>(psram_largest), static_cast<unsigned long>(psram_gap));
}

static void begin_ui_measurement(void)
{
    const int64_t now = esp_timer_get_time();
    s_ui_navigation_started_us = now;
    s_ui_render_started_us = 0;
    s_ui_render_ready_us = 0;
    s_ui_build_duration_us = 0;
    s_final_display_flush_queued = false;
    s_ui_measurement_pending = true;
    ui_ring_overlay_reset_stats();
    if(s_action_touch_us > 0 && now - s_action_touch_us < 1000000) {
        ESP_LOGI(TAG, "UI touch-to-navigation start: %llums",
                 static_cast<long long>((now - s_action_touch_us) / 1000));
    }
    s_action_touch_us = 0;
    log_ui_heap("navigation-start");
}

static void render_async(void *)
{
    begin_ui_measurement();
    if(s_app.state.screen == HAVIT_SCREEN_HOME && !s_wifi_setup_view && !s_pairing_view &&
       is_prototype_view(s_app_view)) {
        show_prototype_view(s_app_view);
        return;
    }
    lv_obj_t *next = lv_obj_create(nullptr);
    finish_render_and_load(next, true);
}

static void refresh_connection_widgets(void)
{
    if(s_connection_widgets_dirty.exchange(false)) {
        const bool connected = s_wifi_connected.load();
        if(s_home_wifi_label != nullptr) {
            lv_label_set_text(s_home_wifi_label, connected ? "Wi-Fi conectado" : "Wi-Fi sin conexión");
            lv_obj_set_style_text_color(s_home_wifi_label, lv_color_hex(connected ? 0x60D394 : WHITE), 0);
        }
        if(s_wifi_status_label != nullptr) {
            lv_label_set_text(s_wifi_status_label, connected ? "Wi-Fi: conectado" : "Wi-Fi: sin conexión");
            lv_obj_set_style_text_color(s_wifi_status_label, lv_color_hex(connected ? 0x60D394 : WHITE), 0);
        }
        if(s_wifi_ssid_label != nullptr) {
            lv_label_set_text(s_wifi_ssid_label, s_network_name[0] ? s_network_name : "Sin red conectada");
        }
        if(s_wifi_ip_label != nullptr) {
            lv_label_set_text(s_wifi_ip_label, s_network_ip[0] ? s_network_ip : "Sin dirección IP");
        }
        if(s_wifi_ble_status_label != nullptr) {
            lv_label_set_text(s_wifi_ble_status_label, s_ble_available ?
                              "Bluetooth: disponible para configurar" : "Bluetooth de configuración: apagado");
        }
        if(s_wifi_ble_button_label != nullptr) {
            lv_label_set_text(s_wifi_ble_button_label, s_ble_available ? "Apagar Bluetooth" : "Activar Bluetooth");
        }
        if(s_wifi_toggle_button_label != nullptr) {
            lv_label_set_text(s_wifi_toggle_button_label, connected ? "Desconectar Wi-Fi" : "Reconectar Wi-Fi");
        }
        if(s_pairing_status_label != nullptr) {
            lv_label_set_text(s_pairing_status_label, connected ? "Wi-Fi conectado" : "Wi-Fi sin conexión");
            lv_obj_set_style_text_color(s_pairing_status_label, lv_color_hex(connected ? 0x60D394 : 0xFFD080), 0);
        }
        if(s_pairing_network_label != nullptr) {
            lv_label_set_text(s_pairing_network_label, connected ? s_network_name :
                              (s_ble_available ? "Bluetooth disponible" : "Bluetooth apagado"));
        }
    }

    if(s_pairing_widgets_dirty.exchange(false)) {
        char code[sizeof(s_pairing_code)];
        char message[sizeof(s_pairing_message)];
        portENTER_CRITICAL(&s_text_lock);
        std::snprintf(code, sizeof(code), "%s", s_pairing_code);
        std::snprintf(message, sizeof(message), "%s", s_pairing_message);
        portEXIT_CRITICAL(&s_text_lock);
        if(s_pairing_code_label != nullptr) {
            lv_label_set_text(s_pairing_code_label, code[0] && s_wifi_connected ? code : "···· ····");
        }
        if(s_pairing_message_label != nullptr) lv_label_set_text(s_pairing_message_label, message);
        if(s_pairing_button_label != nullptr) {
            lv_label_set_text(s_pairing_button_label, s_pairing_running ? "Solicitar nuevo PIN" : "Reintentar / pedir PIN");
        }
    }
}

static void set_pairing_message(const char *format, ...)
{
    char text[sizeof(s_pairing_message)];
    va_list args;
    va_start(args, format);
    std::vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    portENTER_CRITICAL(&s_text_lock);
    std::snprintf(s_pairing_message, sizeof(s_pairing_message), "%s", text);
    portEXIT_CRITICAL(&s_text_lock);
    s_pairing_widgets_dirty = true;
}

static bool start_pairing_request(void)
{
    char request_body[256];
    std::snprintf(request_body, sizeof(request_body),
                  "{\"deviceId\":\"%s\",\"model\":\"Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0\"}", s_device_id);
    HttpReply reply;
    if(!https_request((std::string(HAVIT_API_BASE) + "/startWatchPairing").c_str(), HTTP_METHOD_POST, request_body, nullptr, &reply)) return false;
    cJSON *root = cJSON_Parse(reply.body);
    if(root == nullptr) return false;
    cJSON *id = cJSON_GetObjectItemCaseSensitive(root, "pairingId");
    cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");
    cJSON *token = cJSON_GetObjectItemCaseSensitive(root, "pollToken");
    bool valid = cJSON_IsString(id) && cJSON_IsString(code) && cJSON_IsString(token) &&
                 id->valuestring != nullptr && code->valuestring != nullptr && token->valuestring != nullptr;
    if(valid && !s_pairing_cancelled) {
        std::snprintf(s_pairing_id, sizeof(s_pairing_id), "%s", id->valuestring);
        std::snprintf(s_pairing_token, sizeof(s_pairing_token), "%s", token->valuestring);
        const char *raw_code = code->valuestring;
        portENTER_CRITICAL(&s_text_lock);
        if(std::strlen(raw_code) == 8) {
            std::snprintf(s_pairing_code, sizeof(s_pairing_code), "%.4s %.4s", raw_code, raw_code + 4);
        } else {
            std::snprintf(s_pairing_code, sizeof(s_pairing_code), "%s", raw_code);
        }
        portEXIT_CRITICAL(&s_text_lock);
        s_pairing_widgets_dirty = true;
    }
    cJSON_Delete(root);
    return valid;
}

static void pairing_task(void *)
{
    if(s_device_token[0] != '\0') {
        set_pairing_message("Sincronizando tu cuenta…");
        if(sync_watch_account(s_device_token)) {
            if(s_pairing_cancelled) return;
            ESP_LOGI(TAG, "Saved account synchronized; opening welcome screen");
            s_account_ready = true;
            s_ui_dirty = true;
            return;
        }
        if(s_http_status != 401 && s_http_status != 403) {
            set_pairing_message("Cuenta guardada; servidor sin respuesta. Pulsa Reintentar.");
            return;
        }
        s_device_token[0] = '\0';
        nvs_handle_t store;
        if(nvs_open("havit", NVS_READWRITE, &store) == ESP_OK) {
            (void)nvs_erase_key(store, "device_token");
            (void)nvs_commit(store);
            nvs_close(store);
        }
    }

    portENTER_CRITICAL(&s_text_lock);
    s_pairing_code[0] = '\0';
    portEXIT_CRITICAL(&s_text_lock);
    s_pairing_widgets_dirty = true;
    if(!start_pairing_request()) {
        if(s_http_status > 0) set_pairing_message("Wi-Fi conectado. Error del servidor HTTP %d. Reintentar.", s_http_status.load());
        else set_pairing_message("Wi-Fi conectado; servidor sin respuesta. Reintentar.");
        return;
    }
    if(s_pairing_cancelled) return;
    ESP_LOGI(TAG, "Pairing PIN created; displaying code on HAVIT");
    set_pairing_message("PIN listo. Escríbelo en la web. Dura 10 min.");

    char url[256];
    std::snprintf(url, sizeof(url), "%s/pollWatchPairing?pairingId=%s", HAVIT_API_BASE, s_pairing_id);
    for(int attempt = 0; attempt < 120 && !s_pairing_cancelled; ++attempt) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        if(s_pairing_cancelled) break;
        HttpReply reply;
        if(!https_request(url, HTTP_METHOD_GET, nullptr, s_pairing_token, &reply)) {
            set_pairing_message("Sin respuesta del servidor. Reintentando…");
            continue;
        }
        if(s_pairing_cancelled) return;
        cJSON *root = cJSON_Parse(reply.body);
        cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
        const char *status_text = cJSON_IsString(status) ? status->valuestring : "";
        const bool is_paired = std::strcmp(status_text, "paired") == 0;
        const bool is_expired = std::strcmp(status_text, "expired") == 0;
        const bool is_revoked = std::strcmp(status_text, "revoked") == 0;
        if(is_paired) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "displayName");
            const char *display_name = cJSON_IsString(name) && name->valuestring != nullptr
                                           ? name->valuestring
                                           : "HAVIT";
            std::snprintf(s_display_name, sizeof(s_display_name), "%s", display_name);
            if(root != nullptr) cJSON_Delete(root);
            std::snprintf(s_device_token, sizeof(s_device_token), "%s", s_pairing_token);
            nvs_handle_t store;
            if(nvs_open("havit", NVS_READWRITE, &store) == ESP_OK) {
                ESP_ERROR_CHECK(nvs_set_str(store, "device_token", s_device_token));
                ESP_ERROR_CHECK(nvs_set_str(store, "device_id", s_device_id));
                ESP_ERROR_CHECK(nvs_set_u8(store, "onboarded", 1));
                ESP_ERROR_CHECK(nvs_commit(store));
                nvs_close(store);
            }
            (void)sync_watch_account(s_device_token);
            ESP_LOGI(TAG, "Account linked; opening welcome screen");
            s_account_ready = true;
            s_ui_dirty = true;
            return;
        }
        if(root != nullptr) cJSON_Delete(root);
        if(is_expired || is_revoked) {
            set_pairing_message("PIN vencido. Vuelve y solicita uno nuevo.");
            return;
        }
    }
    if(!s_pairing_cancelled) set_pairing_message("El PIN venció. Vuelve y solicita uno nuevo.");
}

static void start_pairing_task(void *)
{
    pairing_task(nullptr);
    s_pairing_running = false;
    s_pairing_widgets_dirty = true;
    vTaskDelete(nullptr);
}

static void init_image(lv_image_dsc_t *image, const uint8_t *data, uint16_t width,
                       uint16_t height, uint32_t data_size)
{
    image->header.magic = LV_IMAGE_HEADER_MAGIC;
    image->header.cf = LV_COLOR_FORMAT_RGB565A8;
    image->header.w = width;
    image->header.h = height;
    image->header.stride = width * 2;
    image->data_size = data_size;
    image->data = data;
}

static void add_image(lv_obj_t *parent, const lv_image_dsc_t *source,
                      int x, int y, int width, int height)
{
    lv_obj_t *image = lv_image_create(parent);
    lv_image_set_src(image, source);
    lv_obj_set_pos(image, x, y);
    lv_obj_set_size(image, width, height);
    lv_obj_clear_flag(image, LV_OBJ_FLAG_CLICKABLE);
}

static lv_obj_t *add_label(lv_obj_t *parent, const char *text, int y,
                           const lv_font_t *font, uint32_t color, uint8_t opacity)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, SCREEN_SIZE);
    lv_obj_set_pos(label, 0, y);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_opa(label, opacity, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
    return label;
}

static lv_obj_t *add_plain_label(lv_obj_t *parent, const char *text, int x, int y,
                                 const lv_font_t *font, uint32_t color, uint8_t opacity)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_opa(label, opacity, 0);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
    return label;
}

static lv_obj_t *add_card(lv_obj_t *parent, int x, int y, int width, int height,
                          uint32_t color, int radius)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, width, height);
    lv_obj_set_style_bg_color(card, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, radius, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    return card;
}

static void add_circle_outline(lv_obj_t *screen)
{
    lv_obj_t *outline = lv_obj_create(screen);
    lv_obj_remove_style_all(outline);
    lv_obj_set_pos(outline, 0, 0);
    lv_obj_set_size(outline, SCREEN_SIZE, SCREEN_SIZE);
    lv_obj_set_style_radius(outline, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(outline, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(outline, 1, 0);
    lv_obj_set_style_border_color(outline, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_opa(outline, 16, 0);
    lv_obj_clear_flag(outline, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(outline, LV_OBJ_FLAG_CLICKABLE);
}

static void add_eye(lv_obj_t *parent, int x, int y, int width, int height,
                    int rotation, int pivot_x, int pivot_y)
{
    lv_obj_t *eye = add_card(parent, x, y, width, height, 0x1A1A24, LV_RADIUS_CIRCLE);
    lv_obj_set_style_transform_pivot_x(eye, pivot_x, 0);
    lv_obj_set_style_transform_pivot_y(eye, pivot_y, 0);
    lv_obj_set_style_transform_rotation(eye, rotation, 0);
}

static void animate_eye_blink(void *eye, int32_t scale)
{
    if(!s_intro_blink_active.load()) return;
    lv_obj_set_style_transform_scale_y(static_cast<lv_obj_t *>(eye), scale, 0);
}

static void start_intro_blink(lv_obj_t *left_eye, lv_obj_t *right_eye)
{
    s_intro_blink_active = true;
    for(lv_obj_t *eye : {left_eye, right_eye}) {
        lv_anim_t blink;
        lv_anim_init(&blink);
        lv_anim_set_var(&blink, eye);
        lv_anim_set_exec_cb(&blink, animate_eye_blink);
        lv_anim_set_values(&blink, LV_SCALE_NONE, 32);
        lv_anim_set_duration(&blink, INTRO_BLINK_MS);
        lv_anim_set_reverse_duration(&blink, INTRO_BLINK_MS);
        lv_anim_set_repeat_delay(&blink, INTRO_BLINK_INTERVAL_MS);
        lv_anim_set_reverse_delay(&blink, INTRO_BLINK_INTERVAL_MS);
        lv_anim_set_repeat_count(&blink, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&blink);
    }
}

static void add_action_area(lv_obj_t *screen, int x, int y, int width, int height,
                            UiAction action);

static void create_screen_one(lv_obj_t *screen)
{
    add_circle_outline(screen);
    lv_obj_t *left_eye = add_card(screen, 211, 71, 37, 110, 0x1A1A24, LV_RADIUS_CIRCLE);
    lv_obj_set_style_transform_pivot_x(left_eye, 19, 0);
    lv_obj_set_style_transform_pivot_y(left_eye, 55, 0);
    lv_obj_set_style_transform_rotation(left_eye, 50, 0);
    lv_obj_t *right_eye = add_card(screen, 112, 71, 37, 110, 0x1A1A24, LV_RADIUS_CIRCLE);
    lv_obj_set_style_transform_pivot_x(right_eye, 19, 0);
    lv_obj_set_style_transform_pivot_y(right_eye, 55, 0);
    lv_obj_set_style_transform_rotation(right_eye, 1750, 0);
    start_intro_blink(left_eye, right_eye);
    add_label(screen, "Hola, Soy Havit.", 211, s_font_22, WHITE, 184);
    add_label(screen, "Vamos a crecer juntos", 239, s_font_15, MUTED, 64);
    add_image(screen, &s_ring1, 10, 10, 340, 340);
    add_card(screen, 153, 278, 54, 54, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    add_image(screen, &s_arrow_welcome, 167, 298, 25, 14);
    add_action_area(screen, 125, 266, 110, 78, UiAction::ContinueOnboarding);
}

static void create_screen_two(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_label(screen, "Inicia Sesión", 143, s_font_22, WHITE, 184);
    add_label(screen, "Tu próximo paso empieza aquí", 171, s_font_15, MUTED, 64);
    add_image(screen, &s_ring2, 10, 10, 340, 340);

    add_image(screen, &s_separator, 150, 209, 30, 84);
    /* Position these two text labels over the right-hand side of the welcome group. */
    add_plain_label(screen, "Welcome", 185, 224, s_font_20, WHITE, 128);
    add_plain_label(screen, "Back", 185, 248, s_font_20, WHITE, 128);
    add_plain_label(screen, "ROM", 238, 248, s_font_20, ACCENT, LV_OPA_COVER);

    add_card(screen, 82, 218, 64, 64, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    add_image(screen, &s_google, 91, 226, 47, 48);
    add_action_area(screen, 73, 205, 220, 92, UiAction::PairDevice);
}

static void create_screen_three(lv_obj_t *screen)
{
    add_eye(screen, 203, 38, 27, 81, 50, 14, 41);
    add_eye(screen, 130, 38, 27, 81, 1750, 14, 41);
    add_label(screen, "Bienvenido de nuevo.", 154, s_font_22, WHITE, 184);
    add_label(screen, s_display_name[0] ? s_display_name : "HAVIT", 182, s_font_22, ACCENT, LV_OPA_COVER);
    add_image(screen, &s_pill, 87, 242, 180, 51);
    add_plain_label(screen, "Continuar", 98, 253, s_font_24, WHITE, 184);
    add_card(screen, 218, 244, 45, 45, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    add_image(screen, &s_arrow, 228, 260, 25, 14);
    add_action_area(screen, 84, 239, 184, 61, UiAction::ContinueHome);
}

static void add_ui_button(lv_obj_t *screen, const char *text, int x, int y, int width,
                          UiAction action, uint32_t color = CIRCLE_BUTTON)
{
    add_card(screen, x, y, width, 42, color, LV_RADIUS_CIRCLE);
    lv_obj_t *label = add_plain_label(screen, text, x + 5, y + 10, s_font_15,
                                      color == CIRCLE_BUTTON ? UI_BACKGROUND : WHITE, LV_OPA_COVER);
    lv_obj_set_width(label, width - 10);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    add_action_area(screen, x, y, width, 42, action);
}

static void open_text_keyboard(lv_event_t *event)
{
    auto *textarea = static_cast<lv_obj_t *>(lv_event_get_target(event));
    auto *screen = lv_obj_get_screen(textarea);
    lv_obj_t *keyboard = nullptr;
    for(uint32_t i = 0; i < lv_obj_get_child_count(screen); ++i) {
        lv_obj_t *child = lv_obj_get_child(screen, i);
        if(lv_obj_get_user_data(child) == screen) keyboard = child;
    }
    if(keyboard == nullptr) {
        keyboard = lv_keyboard_create(screen);
        lv_obj_set_size(keyboard, 340, 158);
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -8);
        lv_obj_set_style_radius(keyboard, 12, 0);
        lv_obj_add_event_cb(keyboard, [](lv_event_t *e) {
            const lv_event_code_t code = lv_event_get_code(e);
            if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
                lv_obj_add_flag(lv_event_get_target_obj(e), LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_state(static_cast<lv_obj_t *>(lv_keyboard_get_textarea(lv_event_get_target_obj(e))), LV_STATE_FOCUSED);
            }
        }, LV_EVENT_ALL, nullptr);
        lv_obj_set_user_data(keyboard, screen);
    }
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(keyboard, textarea);
    lv_obj_move_foreground(keyboard);
}

static lv_obj_t *add_text_input(lv_obj_t *screen, const char *caption, const char *placeholder,
                                int x, int y, int width, int height, bool one_line = true)
{
    add_plain_label(screen, caption, x, y - 22, s_font_15, WHITE, LV_OPA_COVER);
    lv_obj_t *input = lv_textarea_create(screen);
    lv_obj_set_pos(input, x, y);
    lv_obj_set_size(input, width, height);
    lv_textarea_set_one_line(input, one_line);
    lv_textarea_set_max_length(input, one_line ? 47 : 95);
    lv_textarea_set_placeholder_text(input, placeholder);
    lv_obj_set_style_bg_color(input, lv_color_hex(0x73979A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(input, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(input, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(input, s_font_15, LV_PART_MAIN);
    lv_obj_set_style_border_width(input, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(input, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(input, 12, LV_PART_MAIN);
    lv_obj_add_event_cb(input, open_text_keyboard, LV_EVENT_CLICKED, nullptr);
    return input;
}

static void create_today_screen(lv_obj_t *screen)
{
    const uint32_t total = s_synced_task_count + s_local_tasks.count;
    const time_t now = time(nullptr);
    struct tm local_now{};
    char date[32] = "Fecha no configurada";
    if(now > 1700000000 && localtime_r(&now, &local_now)) {
        std::strftime(date, sizeof(date), "%d %b", &local_now);
    }
    add_card(screen, 121, 121, 118, 118, 0x73979A, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Hoy", 150, 144, s_font_22, WHITE, LV_OPA_COVER);
    add_plain_label(screen, date, 139, 174, s_font_15, WHITE, LV_OPA_COVER);
    char count[40];
    std::snprintf(count, sizeof(count), "%lu tarea%s", static_cast<unsigned long>(total), total == 1 ? "" : "s");
    add_plain_label(screen, count, 128, 198, s_font_20, WHITE, LV_OPA_COVER);

    add_card(screen, 157, 43, 46, 46, 0xD0E4BF, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Wi-Fi", 155, 58, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 142, 35, 76, 62, UiAction::OpenConnections);
    add_card(screen, 48, 151, 58, 58, 0xD0E4BF, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Lista", 51, 172, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 42, 143, 72, 74, UiAction::OpenTaskList);
    add_card(screen, 254, 151, 58, 58, 0xD0E4BF, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Agenda", 250, 172, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 246, 143, 76, 74, UiAction::OpenAgendaDay);
    add_card(screen, 151, 264, 58, 58, 0xD0E4BF, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "+", 169, 270, s_font_24, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 143, 256, 74, 74, UiAction::OpenQuickForm);
    add_plain_label(screen, "Agregar tarea", 116, 326, s_font_15, WHITE, 210);
    if(s_quick_running || s_quick_expired)
        add_ui_button(screen, s_quick_expired ? "Tiempo terminado" : "Tarea en curso", 100, 94, 160, UiAction::OpenQuickTimer, 0x5250F3);
}

static void create_account_settings(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_plain_label(screen, "Mi cuenta", 112, 36, s_font_22, WHITE, LV_OPA_COVER);
    add_action_area(screen, 24, 32, 45, 36, UiAction::GoBack);
    add_plain_label(screen, s_display_name[0] ? s_display_name : "Dispositivo sin vincular",
                    42, 104, s_font_20, 0xD0E4BF, LV_OPA_COVER);
    s_home_wifi_label = add_plain_label(screen, s_wifi_connected ? "Wi-Fi conectado" : "Wi-Fi sin conexión",
                                        42, 145, s_font_15, s_wifi_connected ? 0x60D394 : 0xFFD080,
                                        LV_OPA_COVER);
    lv_obj_set_width(s_home_wifi_label, 276);
    lv_obj_set_style_text_align(s_home_wifi_label, LV_TEXT_ALIGN_CENTER, 0);
    add_ui_button(screen, "Conexiones", 76, 198, 208, UiAction::OpenConnections, 0x73979A);
    add_ui_button(screen, "Volver a Hoy", 90, 255, 180, UiAction::GoBack, 0xD0E4BF);
}

static bool parse_task_date(const char *value, int *year, int *month, int *day)
{
    char tail = '\0';
    return value != nullptr &&
           std::sscanf(value, "%4d-%2d-%2d%c", year, month, day, &tail) == 3 &&
           *month >= 1 && *month <= 12 && *day >= 1 && *day <= 31;
}

static int calendar_days_in_month(int year, int month)
{
    static constexpr int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(month < 1 || month > 12) return 0;
    if(month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) return 29;
    return days[month - 1];
}

static void initialize_calendar(void)
{
    if(s_calendar_initialized) return;
    const time_t now = time(nullptr);
    struct tm local_now{};
    if(now > 1700000000 && localtime_r(&now, &local_now) != nullptr) {
        s_calendar_year = static_cast<uint16_t>(local_now.tm_year + 1900);
        s_calendar_month = static_cast<uint8_t>(local_now.tm_mon + 1);
        s_calendar_selected_day = static_cast<uint8_t>(local_now.tm_mday);
        s_calendar_initialized = true;
    }
}

static uint32_t count_tasks_on_date(const char *date, bool completed_only = false)
{
    uint32_t count = 0;
    for(uint32_t i = 0; i < s_local_tasks.count; ++i) {
        const LocalTask &task = s_local_tasks.tasks[i];
        if(task.due_date[0] == '\0' || std::strcmp(task.due_date, date) != 0) continue;
        if(completed_only && !task.completed) continue;
        ++count;
    }
    return count;
}

static void select_calendar_day_event(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    s_ui_feedback_started_us = esp_timer_get_time();
    s_ui_feedback_pending = true;
    const uint8_t day = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    if(day == 0 || day > calendar_days_in_month(s_calendar_year, s_calendar_month)) return;
    s_calendar_selected_day = day;
    if(s_app_view == AppView::AgendaDay && s_calendar_month_label != nullptr) update_calendar_view();
    else s_ui_dirty = true;
}

static void toggle_task_row_event(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    s_ui_feedback_started_us = esp_timer_get_time();
    s_ui_feedback_pending = true;
    const uintptr_t index = reinterpret_cast<uintptr_t>(lv_event_get_user_data(event));
    if(index >= s_local_tasks.count) return;
    s_local_tasks.tasks[index].completed = !s_local_tasks.tasks[index].completed;
    persist_local_tasks();
    if(s_app_view == AppView::TaskList && s_task_list_widgets.rows != nullptr) update_task_list_view();
    else s_ui_dirty = true;
}

static void update_task_list_view(void)
{
    if(s_task_list_widgets.rows == nullptr || !lv_obj_is_valid(s_task_list_widgets.rows)) return;
    uint32_t pending_count = 0;
    uint32_t completed_count = 0;
    uint8_t visible[LOCAL_TASK_CAPACITY]{};
    uint32_t visible_count = 0;
    for(uint32_t i = 0; i < s_local_tasks.count; ++i) {
        const LocalTask &task = s_local_tasks.tasks[i];
        if(task.completed) ++completed_count;
        else ++pending_count;
        if(static_cast<bool>(task.completed) != s_task_completed_filter) continue;
        if(s_task_date_filter[0] && std::strcmp(task.due_date, s_task_date_filter) != 0) continue;
        visible[visible_count++] = static_cast<uint8_t>(i);
    }
    char counts[40];
    std::snprintf(counts, sizeof(counts), "%lu por hacer  ·  %lu listas",
                  static_cast<unsigned long>(pending_count), static_cast<unsigned long>(completed_count));
    lv_label_set_text(s_task_list_widgets.count, counts);

    const uint32_t page_count = visible_count ? (visible_count + TASK_PAGE_SIZE - 1) / TASK_PAGE_SIZE : 1;
    s_task_page_count = page_count;
    if(s_task_list_page >= page_count) s_task_list_page = static_cast<uint16_t>(page_count - 1);
    const uint32_t first = static_cast<uint32_t>(s_task_list_page) * TASK_PAGE_SIZE;
    const uint32_t end = std::min<uint32_t>(first + TASK_PAGE_SIZE, visible_count);

    lv_label_set_text(s_task_list_widgets.pending_label, s_task_completed_filter ? "Hechas" : "Pendientes");
    lv_label_set_text(s_task_list_widgets.completed_label, s_task_completed_filter ? "Por hacer" : "Completadas");
    lv_obj_set_style_bg_color(s_task_list_widgets.pending_card,
        lv_color_hex(s_task_completed_filter ? 0x73979A : 0xD0E4BF), 0);
    lv_obj_set_style_bg_color(s_task_list_widgets.completed_card,
        lv_color_hex(s_task_completed_filter ? 0xD0E4BF : 0x73979A), 0);
    if(s_task_date_filter[0]) {
        char date_filter[40];
        std::snprintf(date_filter, sizeof(date_filter), "Día %s  ×", s_task_date_filter + 8);
        lv_label_set_text(s_task_list_widgets.date_label, date_filter);
        lv_obj_clear_flag(s_task_list_widgets.date_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_task_list_widgets.date_clear, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_task_list_widgets.date_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_task_list_widgets.date_clear, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_clean(s_task_list_widgets.rows);
    if(visible_count == 0) {
        const char *empty = s_task_date_filter[0] ? "No hay tareas para este día." :
                            (s_task_completed_filter ? "Aún no has completado tareas." : "Tu lista está despejada.");
        lv_obj_t *empty_label = add_plain_label(s_task_list_widgets.rows, empty, 45, 177, s_font_15, 0xD0E4BF, LV_OPA_COVER);
        lv_obj_set_width(empty_label, 270);
        lv_obj_set_style_text_align(empty_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_t *create_card = add_card(s_task_list_widgets.rows, 82, 218, 196, 42, 0x73979A, LV_RADIUS_CIRCLE);
        lv_obj_t *create_label = add_plain_label(s_task_list_widgets.rows, "Crear tarea", 87, 228, s_font_15, WHITE, LV_OPA_COVER);
        lv_obj_set_width(create_label, 186);
        lv_obj_set_style_text_align(create_label, LV_TEXT_ALIGN_CENTER, 0);
        add_action_area(s_task_list_widgets.rows, 82, 218, 196, 42, UiAction::OpenAddMenu);
        (void)create_card;
    } else {
        for(uint32_t i = first; i < end; ++i) {
            const uint32_t task_index = visible[i];
            const LocalTask &task = s_local_tasks.tasks[task_index];
            const int y = (s_task_date_filter[0] ? 151 : 137) + static_cast<int>(i - first) * 54;
            const uint32_t row_color = task.completed ? 0x26383A : 0x1E2C32;
            add_card(s_task_list_widgets.rows, 39, y, 282, 40, row_color, 13);
            add_card(s_task_list_widgets.rows, 47, y + 9, 21, 21, task.completed ? 0xD0E4BF : 0x74B2CD, LV_RADIUS_CIRCLE);
            char title[48];
            std::snprintf(title, sizeof(title), "%.27s", task.title);
            lv_obj_t *title_label = add_plain_label(s_task_list_widgets.rows, title, 77, y + 3, s_font_15,
                task.completed ? 0xA7B5B6 : WHITE, LV_OPA_COVER);
            lv_label_set_long_mode(title_label, LV_LABEL_LONG_DOT);
            lv_obj_set_width(title_label, 226);
            char meta[40];
            int year = 0, month = 0, day = 0;
            if(parse_task_date(task.due_date, &year, &month, &day)) {
                std::snprintf(meta, sizeof(meta), "%.12s  ·  %.16s", task.due_date, task.category);
            } else {
                std::snprintf(meta, sizeof(meta), "%.18s  ·  Sin fecha", task.category);
            }
            add_plain_label(s_task_list_widgets.rows, meta, 78, y + 21, s_font_15, 0x91A4A6, LV_OPA_COVER);
            lv_obj_t *tap = lv_obj_create(s_task_list_widgets.rows);
            lv_obj_remove_style_all(tap);
            lv_obj_set_pos(tap, 39, y);
            lv_obj_set_size(tap, 282, 40);
            lv_obj_add_flag(tap, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_color(tap, lv_color_hex(WHITE), LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(tap, LV_OPA_20, LV_STATE_PRESSED);
            lv_obj_add_event_cb(tap, toggle_task_row_event, LV_EVENT_CLICKED,
                                reinterpret_cast<void *>(static_cast<uintptr_t>(task_index)));
        }
    }
    char page[20];
    std::snprintf(page, sizeof(page), "%u / %u", static_cast<unsigned>(s_task_list_page + 1),
                  static_cast<unsigned>(page_count));
    lv_label_set_text(s_task_list_widgets.page, page);
    if(page_count > 1 && s_task_list_page > 0) lv_obj_clear_flag(s_task_list_widgets.previous, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_task_list_widgets.previous, LV_OBJ_FLAG_HIDDEN);
    if(page_count > 1 && s_task_list_page + 1 < page_count) lv_obj_clear_flag(s_task_list_widgets.next, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_task_list_widgets.next, LV_OBJ_FLAG_HIDDEN);
}

static void create_task_list(lv_obj_t *screen)
{
    s_task_list_widgets = {};
    add_circle_outline(screen);
    add_plain_label(screen, "Mis tareas", 96, 36, s_font_22, WHITE, LV_OPA_COVER);
    s_task_list_widgets.count = add_plain_label(screen, "", 50, 69, s_font_15, 0x91A4A6, LV_OPA_COVER);
    lv_obj_set_width(s_task_list_widgets.count, 260);
    lv_obj_set_style_text_align(s_task_list_widgets.count, LV_TEXT_ALIGN_CENTER, 0);

    s_task_list_widgets.pending_card = add_card(screen, 43, 96, 126, 42, 0xD0E4BF, LV_RADIUS_CIRCLE);
    s_task_list_widgets.pending_label = add_plain_label(screen, "", 48, 108, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    lv_obj_set_width(s_task_list_widgets.pending_label, 116);
    lv_obj_set_style_text_align(s_task_list_widgets.pending_label, LV_TEXT_ALIGN_CENTER, 0);
    add_action_area(screen, 43, 96, 126, 42, UiAction::ToggleTaskFilter);
    s_task_list_widgets.completed_card = add_card(screen, 191, 96, 126, 42, 0x73979A, LV_RADIUS_CIRCLE);
    s_task_list_widgets.completed_label = add_plain_label(screen, "", 196, 108, s_font_15, WHITE, LV_OPA_COVER);
    lv_obj_set_width(s_task_list_widgets.completed_label, 116);
    lv_obj_set_style_text_align(s_task_list_widgets.completed_label, LV_TEXT_ALIGN_CENTER, 0);
    add_action_area(screen, 191, 96, 126, 42, UiAction::ToggleTaskFilter);

    s_task_list_widgets.date_label = add_plain_label(screen, "", 114, 129, s_font_15, 0xD0E4BF, LV_OPA_COVER);
    s_task_list_widgets.date_clear = lv_obj_create(screen);
    lv_obj_remove_style_all(s_task_list_widgets.date_clear);
    lv_obj_set_pos(s_task_list_widgets.date_clear, 100, 126);
    lv_obj_set_size(s_task_list_widgets.date_clear, 160, 26);
    lv_obj_add_flag(s_task_list_widgets.date_clear, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_task_list_widgets.date_clear, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(s_task_list_widgets.date_clear, [](lv_event_t *event) {
        if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
        s_ui_feedback_started_us = esp_timer_get_time();
        s_ui_feedback_pending = true;
        s_task_date_filter[0] = '\0';
        s_task_list_page = 0;
        update_task_list_view();
    }, LV_EVENT_CLICKED, nullptr);

    s_task_list_widgets.rows = lv_obj_create(screen);
    lv_obj_remove_style_all(s_task_list_widgets.rows);
    lv_obj_set_pos(s_task_list_widgets.rows, 0, 0);
    lv_obj_set_size(s_task_list_widgets.rows, SCREEN_SIZE, SCREEN_SIZE);
    lv_obj_clear_flag(s_task_list_widgets.rows, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_task_list_widgets.rows, LV_OBJ_FLAG_CLICKABLE);

    s_task_list_widgets.previous = lv_obj_create(screen);
    lv_obj_remove_style_all(s_task_list_widgets.previous);
    lv_obj_set_pos(s_task_list_widgets.previous, 78, 271);
    lv_obj_set_size(s_task_list_widgets.previous, 34, 32);
    add_card(s_task_list_widgets.previous, 4, 3, 26, 26, 0x26383A, LV_RADIUS_CIRCLE);
    add_plain_label(s_task_list_widgets.previous, "‹", 10, 0, s_font_20, WHITE, LV_OPA_COVER);
    add_action_area(s_task_list_widgets.previous, 0, 0, 34, 32, UiAction::PreviousTaskPage);
    lv_obj_add_flag(s_task_list_widgets.previous, LV_OBJ_FLAG_HIDDEN);
    s_task_list_widgets.page = add_plain_label(screen, "1 / 1", 137, 279, s_font_15, 0xA7B5B6, LV_OPA_COVER);
    lv_obj_set_width(s_task_list_widgets.page, 86);
    lv_obj_set_style_text_align(s_task_list_widgets.page, LV_TEXT_ALIGN_CENTER, 0);
    s_task_list_widgets.next = lv_obj_create(screen);
    lv_obj_remove_style_all(s_task_list_widgets.next);
    lv_obj_set_pos(s_task_list_widgets.next, 248, 271);
    lv_obj_set_size(s_task_list_widgets.next, 34, 32);
    add_card(s_task_list_widgets.next, 4, 3, 26, 26, 0x26383A, LV_RADIUS_CIRCLE);
    add_plain_label(s_task_list_widgets.next, "›", 10, 0, s_font_20, WHITE, LV_OPA_COVER);
    add_action_area(s_task_list_widgets.next, 0, 0, 34, 32, UiAction::NextTaskPage);
    lv_obj_add_flag(s_task_list_widgets.next, LV_OBJ_FLAG_HIDDEN);

    add_ui_button(screen, "Calendario", 42, 311, 128, UiAction::OpenCalendar, 0x73979A);
    add_ui_button(screen, "＋ Agregar", 190, 311, 128, UiAction::OpenAddMenu, 0xD0E4BF);
    add_action_area(screen, 24, 32, 45, 36, UiAction::GoBack);
    update_task_list_view();
}

static void update_calendar_view(void)
{
    static constexpr const char *months[] = {
        "enero", "febrero", "marzo", "abril", "mayo", "junio",
        "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"
    };
    if(s_calendar_month_label == nullptr || !lv_obj_is_valid(s_calendar_month_label)) return;
    char month_year[40];
    std::snprintf(month_year, sizeof(month_year), "%s %u", months[s_calendar_month - 1],
                  static_cast<unsigned>(s_calendar_year));
    lv_label_set_text(s_calendar_month_label, month_year);
    constexpr int grid_left = 47;
    constexpr int cell_width = 38;
    struct tm first_day{};
    first_day.tm_year = static_cast<int>(s_calendar_year) - 1900;
    first_day.tm_mon = static_cast<int>(s_calendar_month) - 1;
    first_day.tm_mday = 1;
    const time_t first_epoch = mktime(&first_day);
    localtime_r(&first_epoch, &first_day);
    const int first_column = (first_day.tm_wday + 6) % 7;
    const int days = calendar_days_in_month(s_calendar_year, s_calendar_month);
    for(int day = 1; day <= days; ++day) {
        const int cell = first_column + day - 1;
        const int column = cell % 7;
        const int row = cell / 7;
        const int x = grid_left + column * cell_width + 1;
        const int y = 120 + row * 25;
        char date[32];
        std::snprintf(date, sizeof(date), "%04u-%02u-%02d", static_cast<unsigned>(s_calendar_year),
                      static_cast<unsigned>(s_calendar_month), day);
        const bool has_tasks = count_tasks_on_date(date) > 0;
        CalendarDayWidgets &widgets = s_calendar_day_widgets[day - 1];
        const bool selected = day == s_calendar_selected_day;
        lv_obj_set_pos(widgets.button, x, y);
        lv_obj_clear_flag(widgets.button, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(widgets.label, "%d", day);
        lv_obj_set_style_bg_color(widgets.button, lv_color_hex(selected ? 0xD0E4BF : 0x000000), 0);
        lv_obj_set_style_bg_opa(widgets.button, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(widgets.label,
            lv_color_hex(selected ? UI_BACKGROUND : (has_tasks ? 0x74B2CD : WHITE)), 0);
        if(has_tasks && !selected) lv_obj_clear_flag(widgets.marker, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(widgets.marker, LV_OBJ_FLAG_HIDDEN);
    }
    for(int day = days; day < 31; ++day)
        lv_obj_add_flag(s_calendar_day_widgets[day].button, LV_OBJ_FLAG_HIDDEN);

    char selected_date[32] = "";
    std::snprintf(selected_date, sizeof(selected_date), "%04u-%02u-%02u",
                  static_cast<unsigned>(s_calendar_year), static_cast<unsigned>(s_calendar_month),
                  static_cast<unsigned>(s_calendar_selected_day));
    const uint32_t selected_count = count_tasks_on_date(selected_date);
    char selected_title[48];
    std::snprintf(selected_title, sizeof(selected_title), "%02u %s  ·  %lu tarea%s",
                  static_cast<unsigned>(s_calendar_selected_day), months[s_calendar_month - 1],
                  static_cast<unsigned long>(selected_count), selected_count == 1 ? "" : "s");
    lv_label_set_text(s_calendar_selected_label, selected_title);
    if(selected_count) {
        bool found = false;
        for(uint32_t i = 0, shown = 0; i < s_local_tasks.count && shown < 1; ++i) {
            const LocalTask &task = s_local_tasks.tasks[i];
            if(std::strcmp(task.due_date, selected_date) != 0) continue;
            char task_line[36];
            std::snprintf(task_line, sizeof(task_line), "%s %.27s", task.completed ? "✓" : "·", task.title);
            lv_label_set_text(s_calendar_task_label, task_line);
            lv_obj_set_style_text_color(s_calendar_task_label,
                lv_color_hex(task.completed ? 0x91A4A6 : WHITE), 0);
            ++shown;
            found = true;
        }
        if(!found) lv_label_set_text(s_calendar_task_label, "Sin entregas programadas");
    } else {
        lv_label_set_text(s_calendar_task_label, "Sin entregas programadas");
    }
}

static void create_calendar_view(lv_obj_t *screen)
{
    static constexpr const char *weekdays[] = {"L", "M", "X", "J", "V", "S", "D"};
    s_calendar_month_label = nullptr;
    s_calendar_selected_label = nullptr;
    s_calendar_task_label = nullptr;
    for(auto &day : s_calendar_day_widgets) day = {};
    add_circle_outline(screen);
    add_plain_label(screen, "Calendario", 112, 36, s_font_22, WHITE, LV_OPA_COVER);
    add_action_area(screen, 24, 32, 45, 36, UiAction::GoBack);
    initialize_calendar();
    if(!s_calendar_initialized) {
        add_plain_label(screen, "Conéctate para cargar la fecha del reloj.", 45, 157,
                        s_font_15, 0xD0E4BF, LV_OPA_COVER);
        add_ui_button(screen, "Volver a Hoy", 82, 216, 196, UiAction::GoBack, 0x73979A);
        return;
    }

    s_calendar_month_label = add_plain_label(screen, "", 104, 72, s_font_20, 0xD0E4BF, LV_OPA_COVER);
    lv_obj_set_width(s_calendar_month_label, 152);
    lv_obj_set_style_text_align(s_calendar_month_label, LV_TEXT_ALIGN_CENTER, 0);
    add_action_area(screen, 48, 67, 42, 36, UiAction::PreviousCalendarMonth);
    add_plain_label(screen, "‹", 58, 69, s_font_20, WHITE, LV_OPA_COVER);
    add_action_area(screen, 270, 67, 42, 36, UiAction::NextCalendarMonth);
    add_plain_label(screen, "›", 282, 69, s_font_20, WHITE, LV_OPA_COVER);

    constexpr int grid_left = 47;
    constexpr int cell_width = 38;
    for(int column = 0; column < 7; ++column) {
        lv_obj_t *weekday = add_plain_label(screen, weekdays[column], grid_left + column * cell_width,
                                             103, s_font_15, 0x91A4A6, LV_OPA_COVER);
        lv_obj_set_width(weekday, cell_width);
        lv_obj_set_style_text_align(weekday, LV_TEXT_ALIGN_CENTER, 0);
    }
    for(uint32_t day = 0; day < 31; ++day) {
        CalendarDayWidgets &widgets = s_calendar_day_widgets[day];
        widgets.button = lv_obj_create(screen);
        lv_obj_remove_style_all(widgets.button);
        lv_obj_set_size(widgets.button, 36, 24);
        lv_obj_set_style_radius(widgets.button, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(widgets.button, LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_font(widgets.button, s_font_15, 0);
        lv_obj_add_flag(widgets.button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(widgets.button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(widgets.button, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(widgets.button, select_calendar_day_event, LV_EVENT_CLICKED,
                            reinterpret_cast<void *>(static_cast<uintptr_t>(day + 1)));
        widgets.label = lv_label_create(widgets.button);
        lv_label_set_text_fmt(widgets.label, "%u", static_cast<unsigned>(day + 1));
        lv_obj_set_size(widgets.label, 36, 18);
        lv_obj_set_pos(widgets.label, 0, 0);
        lv_obj_set_style_text_font(widgets.label, s_font_15, 0);
        lv_obj_set_style_text_align(widgets.label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_clear_flag(widgets.label, LV_OBJ_FLAG_CLICKABLE);
        widgets.marker = add_card(widgets.button, 16, 19, 4, 4, 0x74B2CD, LV_RADIUS_CIRCLE);
        lv_obj_clear_flag(widgets.marker, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(widgets.marker, LV_OBJ_FLAG_HIDDEN);
    }
    s_calendar_selected_label = add_plain_label(screen, "", 50, 274, s_font_15, 0xD0E4BF, LV_OPA_COVER);
    s_calendar_task_label = add_plain_label(screen, "", 56, 293, s_font_15, WHITE, LV_OPA_COVER);
    add_ui_button(screen, "Ver tareas del día", 77, 316, 206,
                  UiAction::OpenTasksForSelectedDay, 0x73979A);
    update_calendar_view();
}

static void quick_duration_changed(lv_event_t *event)
{
    auto *dropdown = static_cast<lv_obj_t *>(lv_event_get_target(event));
    static constexpr uint32_t durations[] = {300, 600, 900, 1200, 1800, 2700, 3600};
    const uint32_t selected = lv_dropdown_get_selected(dropdown);
    if(selected < sizeof(durations) / sizeof(durations[0])) s_quick_duration_seconds = durations[selected];
}

static void create_quick_form(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_plain_label(screen, "Tarea rápida", 78, 38, s_font_22, WHITE, LV_OPA_COVER);
    add_ui_button(screen, "Atrás", 26, 30, 84, UiAction::GoBack);
    s_quick_name_input = add_text_input(screen, "Nombre", "Escribe tu tarea", 42, 112, 276, 46);
    if(s_quick_name[0]) lv_textarea_set_text(s_quick_name_input, s_quick_name);
    add_plain_label(screen, "Duración", 42, 186, s_font_15, WHITE, LV_OPA_COVER);
    s_quick_duration_select = lv_dropdown_create(screen);
    lv_dropdown_set_options(s_quick_duration_select, "5 min\n10 min\n15 min\n20 min\n30 min\n45 min\n60 min");
    lv_obj_set_pos(s_quick_duration_select, 126, 174);
    lv_obj_set_size(s_quick_duration_select, 192, 42);
    lv_obj_set_style_text_font(s_quick_duration_select, s_font_15, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_quick_duration_select, lv_color_hex(0x73979A), LV_PART_MAIN);
    lv_obj_add_event_cb(s_quick_duration_select, quick_duration_changed, LV_EVENT_VALUE_CHANGED, nullptr);
    static constexpr uint32_t duration_values[] = {300, 600, 900, 1200, 1800, 2700, 3600};
    for(uint32_t i = 0; i < 7; ++i) if(duration_values[i] == s_quick_duration_seconds) lv_dropdown_set_selected(s_quick_duration_select, i);
    add_ui_button(screen, "Empezar", 91, 258, 178, UiAction::QuickStart, 0x73979A);
    add_ui_button(screen, "Cancelar", 108, 312, 144, UiAction::GoBack);
    s_keyboard = lv_keyboard_create(screen);
    lv_obj_set_size(s_keyboard, 340, 158);
    lv_obj_align(s_keyboard, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(s_keyboard, [](lv_event_t *e) {
        if(lv_event_get_code(e) == LV_EVENT_READY || lv_event_get_code(e) == LV_EVENT_CANCEL) {
            lv_obj_add_flag(lv_event_get_target_obj(e), LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_ALL, nullptr);
}

static void quick_timer_tick(lv_timer_t *)
{
    if(!s_quick_running || s_quick_paused) return;
    const int64_t remaining = s_quick_deadline_us - esp_timer_get_time();
    if(remaining <= 0) {
        s_quick_running = false;
        s_quick_expired = true;
        if(s_app_view == AppView::QuickTimer) s_app_view = AppView::QuickComplete;
        s_ui_dirty = true;
        return;
    }
    if(s_app_view == AppView::QuickTimer && s_active_prototype_view == AppView::QuickTimer &&
       lv_screen_active() == s_prototype_views.root && s_timer_label != nullptr &&
       lv_obj_is_valid(s_timer_label)) {
        const int64_t seconds = (remaining + 999999) / 1000000;
        char value[16];
        std::snprintf(value, sizeof(value), "%02u:%02u:%02u",
                      static_cast<unsigned>(seconds / 3600),
                      static_cast<unsigned>((seconds / 60) % 60),
                      static_cast<unsigned>(seconds % 60));
        if(std::strcmp(lv_label_get_text(s_timer_label), value) != 0) {
            lv_label_set_text(s_timer_label, value);
        }
    }
}

static void create_quick_timer(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_plain_label(screen, s_quick_name[0] ? s_quick_name : "Tarea rápida", 40, 88, s_font_20, WHITE, LV_OPA_COVER);
    const int64_t remaining = s_quick_paused ? s_quick_paused_remaining_us : std::max<int64_t>(0, s_quick_deadline_us - esp_timer_get_time());
    const int64_t seconds = (remaining + 999999) / 1000000;
    char value[16];
    std::snprintf(value, sizeof(value), "%02u:%02u", static_cast<unsigned>(seconds / 60), static_cast<unsigned>(seconds % 60));
    s_timer_label = add_label(screen, s_quick_expired ? "Tiempo terminado" : value, 154, s_font_24, WHITE, LV_OPA_COVER);
    if(s_quick_expired) {
        add_ui_button(screen, "Guardar tarea", 78, 220, 204, UiAction::QuickComplete, 0x73979A);
        add_ui_button(screen, "Descartar", 108, 274, 144, UiAction::QuickCancel);
    } else {
        add_ui_button(screen, s_quick_paused ? "Reanudar" : "Pausar", 80, 220, 200, UiAction::QuickPauseResume, 0x73979A);
        add_ui_button(screen, "Cancelar", 108, 274, 144, UiAction::QuickCancel);
    }
    add_ui_button(screen, "Conexiones", 108, 324, 144, UiAction::OpenConnections);
    if(s_quick_timer == nullptr) s_quick_timer = lv_timer_create(quick_timer_tick, 250, nullptr);
}

static void create_project_form(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_plain_label(screen, "Nuevo proyecto", 54, 35, s_font_20, WHITE, LV_OPA_COVER);
    add_ui_button(screen, "Atrás", 24, 28, 84, UiAction::GoBack);
    s_project_name_input = add_text_input(screen, "Nombre del proyecto", "Escribe un nombre", 42, 103, 276, 42);
    if(s_project_name[0]) lv_textarea_set_text(s_project_name_input, s_project_name);
    s_project_subject_input = add_text_input(screen, "Materia", "Ej. Humanidades", 42, 166, 276, 42);
    if(s_project_subject[0]) lv_textarea_set_text(s_project_subject_input, s_project_subject);
    add_ui_button(screen, "Continuar", 91, 244, 178, UiAction::OpenProjectDetails, 0x73979A);
    add_ui_button(screen, "Conexiones", 108, 302, 144, UiAction::OpenConnections);
    s_keyboard = lv_keyboard_create(screen);
    lv_obj_set_size(s_keyboard, 340, 158);
    lv_obj_align(s_keyboard, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(s_keyboard, [](lv_event_t *e) {
        if(lv_event_get_code(e) == LV_EVENT_READY || lv_event_get_code(e) == LV_EVENT_CANCEL) lv_obj_add_flag(lv_event_get_target_obj(e), LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_ALL, nullptr);
}

static bool valid_project_date(const char *value)
{
    int year = 0, month = 0, day = 0;
    char tail = '\0';
    if(value == nullptr || std::sscanf(value, "%4d-%2d-%2d%c", &year, &month, &day, &tail) != 3) return false;
    if(year < 2024 || month < 1 || month > 12) return false;
    static constexpr int days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max_day = days_per_month[month - 1];
    if(month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) max_day = 29;
    return day >= 1 && day <= max_day;
}

static void create_project_details(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_plain_label(screen, "Detalles del proyecto", 53, 35, s_font_20, WHITE, LV_OPA_COVER);
    add_ui_button(screen, "Atrás", 24, 28, 84, UiAction::GoBack);
    s_project_due_input = add_text_input(screen, "Fecha de entrega", "AAAA-MM-DD", 42, 106, 276, 42);
    if(s_project_due[0]) lv_textarea_set_text(s_project_due_input, s_project_due);
    s_project_description_input = add_text_input(screen, "Descripción", "Describe el proyecto", 42, 170, 276, 48, false);
    if(s_project_description[0]) lv_textarea_set_text(s_project_description_input, s_project_description);
    add_plain_label(screen, "La fecha usa el formato AAAA-MM-DD.", 40, 227, s_font_15, WHITE, 185);
    add_ui_button(screen, "Guardar en este reloj", 71, 260, 218, UiAction::SaveProject, 0x73979A);
    add_ui_button(screen, "Conexiones", 108, 310, 144, UiAction::OpenConnections);
    s_keyboard = lv_keyboard_create(screen);
    lv_obj_set_size(s_keyboard, 340, 158);
    lv_obj_align(s_keyboard, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(s_keyboard, [](lv_event_t *e) {
        if(lv_event_get_code(e) == LV_EVENT_READY || lv_event_get_code(e) == LV_EVENT_CANCEL) lv_obj_add_flag(lv_event_get_target_obj(e), LV_OBJ_FLAG_HIDDEN);
    }, LV_EVENT_ALL, nullptr);
}

static void create_project_saved(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_plain_label(screen, "Proyecto guardado", 64, 37, s_font_20, WHITE, LV_OPA_COVER);
    add_plain_label(screen, s_project_name, 42, 120, s_font_20, ACCENT, LV_OPA_COVER);
    add_plain_label(screen, "Guardado localmente en este reloj.", 35, 164, s_font_15, WHITE, 210);
    add_plain_label(screen, "Conéctate para sincronizarlo con tu cuenta.", 28, 190, s_font_15, WHITE, 190);
    add_ui_button(screen, "Ver tareas", 91, 252, 178, UiAction::OpenTaskList, 0x73979A);
    add_ui_button(screen, "Hoy", 108, 307, 144, UiAction::SaveProjectConfirm);
}

static void create_quick_complete(lv_obj_t *screen)
{
    add_circle_outline(screen);
    add_plain_label(screen, "Tiempo terminado", 74, 105, s_font_20, WHITE, LV_OPA_COVER);
    add_plain_label(screen, s_quick_name, 44, 146, s_font_20, ACCENT, 250);
    add_ui_button(screen, "Guardar tarea", 78, 220, 204, UiAction::QuickComplete, 0x73979A);
    add_ui_button(screen, "Descartar", 108, 274, 144, UiAction::QuickCancel);
}

static void create_agenda(lv_obj_t *screen, bool week)
{
    add_circle_outline(screen);
    add_plain_label(screen, week ? "Esta semana" : "Agenda de hoy", 50, 36, s_font_20, WHITE, LV_OPA_COVER);
    const time_t now = time(nullptr);
    const bool has_clock = now > 1700000000;
    if(!has_clock) {
        add_plain_label(screen, "La fecha del reloj se configura con Wi-Fi.", 38, 136, s_font_15, WHITE, 220);
    }
    struct tm today{};
    if(has_clock && localtime_r(&now, &today)) {
        today.tm_hour = 0; today.tm_min = 0; today.tm_sec = 0;
    }
    time_t period_start = has_clock ? mktime(&today) : 0;
    if(week && has_clock) period_start -= static_cast<time_t>((today.tm_wday + 6) % 7) * 86400;
    uint32_t row = 0;
    for(uint32_t i = 0; i < s_local_tasks.count && row < 4; ++i) {
        const LocalTask &task = s_local_tasks.tasks[i];
        if(task.completed) continue;
        if(task.due_date[0] && has_clock) {
            int year = 0, month = 0, day = 0;
            if(std::sscanf(task.due_date, "%4d-%2d-%2d", &year, &month, &day) != 3) continue;
            struct tm due{};
            due.tm_year = year - 1900; due.tm_mon = month - 1; due.tm_mday = day;
            const int64_t day_offset = (static_cast<int64_t>(mktime(&due)) - period_start) / 86400;
            if(day_offset < 0 || (week ? day_offset > 6 : day_offset != 0)) continue;
        }
        char line[72];
        if(task.due_date[0]) std::snprintf(line, sizeof(line), "%.10s · %.47s", task.due_date, task.title);
        else std::snprintf(line, sizeof(line), "%.47s · %.19s", task.title, task.category);
        add_plain_label(screen, line, 45, 92 + static_cast<int>(row) * 42, s_font_15, WHITE, LV_OPA_COVER);
        ++row;
    }
    if(row == 0) add_plain_label(screen, "No hay entregas locales para mostrar.", 48, 144, s_font_15, WHITE, 200);
    add_ui_button(screen, week ? "Ver día" : "Ver semana", 54, 275, 120, week ? UiAction::OpenAgendaDay : UiAction::OpenAgendaWeek, 0x73979A);
    add_ui_button(screen, "Volver", 187, 275, 120, UiAction::GoBack);
}

static void adopt_generated_screen(lv_obj_t *screen, lv_obj_t *(*create_screen)(void))
{
    lv_obj_t *generated = create_screen();
    if(generated == nullptr) return;
    /* Routing is owned by the app state machine; keep the Figma layers visual-only. */
    lv_obj_clear_flag(generated, LV_OBJ_FLAG_CLICKABLE);
    for(uint32_t i = 0; i < lv_obj_get_child_count(generated); ++i) {
        lv_obj_t *child = lv_obj_get_child(generated, i);
        lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
        for(uint32_t j = 0; j < lv_obj_get_child_count(child); ++j) {
            lv_obj_clear_flag(lv_obj_get_child(child, j), LV_OBJ_FLAG_CLICKABLE);
        }
    }
    while(lv_obj_get_child_count(generated) > 0) {
        lv_obj_set_parent(lv_obj_get_child(generated, 0), screen);
    }
    lv_obj_delete(generated);
}

static lv_obj_t *find_generated_label(lv_obj_t *root, const char *text)
{
    if(lv_obj_check_type(root, &lv_label_class) &&
       std::strcmp(lv_label_get_text(root), text) == 0) return root;
    for(uint32_t i = 0; i < lv_obj_get_child_count(root); ++i) {
        if(lv_obj_t *found = find_generated_label(lv_obj_get_child(root, i), text)) return found;
    }
    return nullptr;
}

static void set_generated_label(lv_obj_t *root, const char *text, const char *replacement)
{
    if(lv_obj_t *label = find_generated_label(root, text)) lv_label_set_text(label, replacement);
}

static uint32_t collect_week_task_indices(uint8_t *indices, uint32_t capacity)
{
    const time_t now = time(nullptr);
    struct tm today{};
    if(now <= 1700000000 || localtime_r(&now, &today) == nullptr) return 0;
    today.tm_hour = today.tm_min = today.tm_sec = 0;
    time_t monday = mktime(&today) - static_cast<time_t>((today.tm_wday + 6) % 7) * 86400;
    uint32_t count = 0;
    for(int day_offset = 0; day_offset < 7 && count < capacity; ++day_offset) {
        const time_t date_epoch = monday + static_cast<time_t>(day_offset) * 86400;
        struct tm date_parts{};
        if(localtime_r(&date_epoch, &date_parts) == nullptr) continue;
        char date[12];
        std::strftime(date, sizeof(date), "%Y-%m-%d", &date_parts);
        for(uint32_t i = 0; i < s_local_tasks.count && count < capacity; ++i) {
            const LocalTask &task = s_local_tasks.tasks[i];
            if(task.completed || std::strcmp(task.due_date, date) != 0) continue;
            indices[count++] = static_cast<uint8_t>(i);
        }
    }
    return count;
}

static void set_local_agenda_labels(lv_obj_t *root, const uint8_t *indices,
                                    uint32_t count, uint32_t *index)
{
    static constexpr const char *sample_names[] = {
        "Seminario 1", "Web Design", "Practicum 2", "Mobile Design", "Practicum 1"
    };
    if(lv_obj_check_type(root, &lv_label_class)) {
        const char *current = lv_label_get_text(root);
        bool sample = false;
        for(const char *name : sample_names) if(std::strcmp(current, name) == 0) sample = true;
        if(sample) {
            if(*index < count) {
                lv_label_set_text(root, s_local_tasks.tasks[indices[*index]].title);
                lv_obj_set_width(root, 70);
                lv_obj_set_height(root, 16);
                lv_label_set_long_mode(root, LV_LABEL_LONG_DOT);
                ++(*index);
            } else {
                lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    for(uint32_t i = 0; i < lv_obj_get_child_count(root); ++i) {
        set_local_agenda_labels(lv_obj_get_child(root, i), indices, count, index);
    }
}

static void current_date_label(char *destination, size_t capacity)
{
    static constexpr const char *months[] = {
        "enero", "febrero", "marzo", "abril", "mayo", "junio",
        "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"
    };
    const time_t now = time(nullptr);
    struct tm local_now{};
    if(now <= 1700000000 || localtime_r(&now, &local_now) == nullptr) {
        std::snprintf(destination, capacity, "Sin sincronizar");
        return;
    }
    std::snprintf(destination, capacity, "%d de %s", local_now.tm_mday, months[local_now.tm_mon]);
}

static void current_time_label(char *destination, size_t capacity)
{
    const time_t now = time(nullptr);
    struct tm local_now{};
    if(now <= 1700000000 || localtime_r(&now, &local_now) == nullptr) {
        std::snprintf(destination, capacity, "--:--");
        return;
    }
    std::strftime(destination, capacity, "%I:%M %p", &local_now);
}

static void current_week_label(char *destination, size_t capacity)
{
    static constexpr const char *months[] = {
        "ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic"
    };
    const time_t now = time(nullptr);
    struct tm today{};
    if(now <= 1700000000 || localtime_r(&now, &today) == nullptr) {
        std::snprintf(destination, capacity, "Semana sin sincronizar");
        return;
    }
    today.tm_hour = today.tm_min = today.tm_sec = 0;
    time_t monday = mktime(&today) - static_cast<time_t>((today.tm_wday + 6) % 7) * 86400;
    const time_t sunday = monday + 6 * 86400;
    struct tm first{};
    struct tm last{};
    localtime_r(&monday, &first);
    localtime_r(&sunday, &last);
    if(first.tm_mon == last.tm_mon) {
        std::snprintf(destination, capacity, "%d-%d de %s", first.tm_mday, last.tm_mday,
                      months[first.tm_mon]);
    } else {
        std::snprintf(destination, capacity, "%d %s - %d %s", first.tm_mday,
                      months[first.tm_mon], last.tm_mday, months[last.tm_mon]);
    }
}

static lv_obj_t *add_generated_text_input(lv_obj_t *screen, const char *initial,
                                           int x, int y, int width, int height,
                                           uint32_t max_length = 47, bool one_line = true)
{
    lv_obj_t *input = lv_textarea_create(screen);
    lv_obj_set_pos(input, x, y);
    lv_obj_set_size(input, width, height);
    lv_textarea_set_one_line(input, one_line);
    lv_textarea_set_max_length(input, max_length);
    lv_textarea_set_text(input, initial != nullptr ? initial : "");
    lv_obj_set_style_bg_opa(input, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(input, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(input, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(input, s_font_15, LV_PART_MAIN);
    lv_obj_set_style_pad_all(input, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(input, open_text_keyboard, LV_EVENT_CLICKED, nullptr);
    return input;
}

static void create_generated_quick_form(lv_obj_t *screen)
{
    adopt_generated_screen(screen, add_task_p1_b_create);
    s_quick_name_input = add_generated_text_input(screen, s_quick_name, 46, 171, 220, 23);
    add_action_area(screen, 164, 90, 33, 33, UiAction::GoBack);
    add_action_area(screen, 116, 282, 129, 40, UiAction::QuickStart);

    /* Keep the Figma timer controls visible and add an unobtrusive native selector. */
    lv_obj_t *duration = lv_dropdown_create(screen);
    lv_dropdown_set_options(duration, "5 min\n10 min\n15 min\n20 min\n30 min\n45 min\n60 min");
    lv_obj_set_pos(duration, 149, 211);
    lv_obj_set_size(duration, 87, 46);
    lv_obj_set_style_bg_color(duration, lv_color_hex(0x73979A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(duration, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(duration, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(duration, 10, LV_PART_MAIN);
    lv_obj_set_style_text_color(duration, lv_color_hex(WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(duration, s_font_15, LV_PART_MAIN);
    lv_obj_set_style_text_align(duration, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_event_cb(duration, quick_duration_changed, LV_EVENT_VALUE_CHANGED, nullptr);
    s_quick_duration_select = duration;
    static constexpr uint32_t duration_values[] = {300, 600, 900, 1200, 1800, 2700, 3600};
    for(uint32_t i = 0; i < 7; ++i) {
        if(duration_values[i] == s_quick_duration_seconds) lv_dropdown_set_selected(duration, i);
    }
}

static void create_generated_timer(lv_obj_t *screen)
{
    adopt_generated_screen(screen, add_task_p1_c_create);
    s_quick_timer_title_label = find_generated_label(screen, "Tarea Rápida");
    if(s_quick_timer_title_label != nullptr) {
        lv_label_set_text(s_quick_timer_title_label,
                          s_quick_paused ? "En pausa" : (s_quick_name[0] ? s_quick_name : "Tarea Rápida"));
    }
    if(lv_obj_t *subject = find_generated_label(screen, "Humanidades")) {
        lv_label_set_text(subject, "Temporizador");
    }
    s_timer_label = find_generated_label(screen, "00:20:00");
    if(s_timer_label != nullptr) {
        const int64_t remaining = s_quick_paused ? s_quick_paused_remaining_us :
            std::max<int64_t>(0, s_quick_deadline_us - esp_timer_get_time());
        const uint32_t seconds = static_cast<uint32_t>((remaining + 999999) / 1000000);
        char value[16];
        std::snprintf(value, sizeof(value), "%02u:%02u:%02u",
                      static_cast<unsigned int>(seconds / 3600),
                      static_cast<unsigned int>((seconds / 60) % 60),
                      static_cast<unsigned int>(seconds % 60));
        lv_label_set_text(s_timer_label, value);
    }
    add_action_area(screen, 102, 234, 62, 62, UiAction::ShowQuickComplete);
    add_action_area(screen, 197, 237, 59, 59, UiAction::QuickCancel);
    add_action_area(screen, 68, 150, 224, 64, UiAction::QuickPauseResume);
}

static void create_project_inputs(lv_obj_t *screen, bool details)
{
    if(!details) {
        adopt_generated_screen(screen, add_task_p1_create);
        s_project_name_input = add_generated_text_input(screen, s_project_name, 46, 174, 204, 21);
        s_project_subject_input = add_generated_text_input(screen, s_project_subject, 46, 226, 204, 21, 19);
        add_action_area(screen, 164, 90, 33, 33, UiAction::GoBack);
        add_action_area(screen, 162, 296, 36, 37, UiAction::OpenProjectDetails);
        return;
    }

    adopt_generated_screen(screen, add_task_p2_create);
    s_project_due_input = add_generated_text_input(screen, s_project_due, 212, 114, 82, 20, 23);
    lv_textarea_set_placeholder_text(s_project_due_input, "AAAA-MM-DD");
    s_project_description_input = add_generated_text_input(screen, s_project_description, 51, 188, 202, 48, 95, false);
    add_action_area(screen, 198, 64, 36, 37, UiAction::GoBack);
    add_action_area(screen, 162, 296, 36, 37, UiAction::OpenProjectSchedule);
}

static void create_project_schedule(lv_obj_t *screen)
{
    adopt_generated_screen(screen, add_task_p3_create);
    lv_obj_t *start = add_generated_text_input(screen, s_project_start, 87, 145, 92, 21, 23);
    lv_obj_t *due = add_generated_text_input(screen, s_project_due, 87, 227, 92, 21, 23);
    lv_textarea_set_placeholder_text(start, "AAAA-MM-DD");
    lv_textarea_set_placeholder_text(due, "AAAA-MM-DD");
    s_project_due_input = due;
    s_project_start_input = start;
    if(s_project_dates_invalid) {
        lv_obj_t *error = add_plain_label(screen, "Usa AAAA-MM-DD y deja la entrega después del inicio.",
                                          39, 270, font_instrument_sans_regular_10,
                                          0xFF7777, LV_OPA_COVER);
        lv_obj_set_width(error, 280);
        lv_obj_set_style_text_align(error, LV_TEXT_ALIGN_CENTER, 0);
    }
    add_action_area(screen, 162, 29, 36, 37, UiAction::GoBack);
    add_action_area(screen, 162, 296, 36, 37, UiAction::GenerateProjectTimeline);
}

static lv_obj_t *add_notification_dropdown(lv_obj_t *screen, const char *options,
                                            uint8_t selected, int x, int y, int width)
{
    lv_obj_t *dropdown = lv_dropdown_create(screen);
    lv_dropdown_set_options(dropdown, options);
    lv_dropdown_set_selected(dropdown, selected);
    lv_obj_set_pos(dropdown, x, y);
    lv_obj_set_size(dropdown, width, 24);
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x77B7DB), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dropdown, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(dropdown, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(dropdown, 6, LV_PART_MAIN);
    lv_obj_set_style_text_color(dropdown, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(dropdown, font_instrument_sans_regular_12, LV_PART_MAIN);
    lv_obj_set_style_pad_left(dropdown, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_right(dropdown, 14, LV_PART_MAIN);
    return dropdown;
}

static void create_project_notifications(lv_obj_t *screen)
{
    adopt_generated_screen(screen, desglose_create);
    set_generated_label(screen, "Agregar Tarea", "Avisos del proyecto");
    s_project_notice_sound = add_notification_dropdown(screen,
        "Sin sonido\nSuave\nFuerte", s_notice_sound, 163, 120, 62);
    s_project_notice_light = add_notification_dropdown(screen,
        "Azul\nVerde\nÁmbar\nApagada", s_notice_light, 169, 162, 71);
    s_project_notice_tone = add_notification_dropdown(screen,
        "Tono 1\nTono 2\nTono 3", s_notice_tone, 143, 201, 62);
    add_action_area(screen, 160, 286, 42, 42, UiAction::SaveProject);
}

static bool project_milestone_date(uint8_t milestone, char *destination, size_t capacity)
{
    int start_year = 0, start_month = 0, start_day = 0;
    int due_year = 0, due_month = 0, due_day = 0;
    if(std::sscanf(s_project_start, "%4d-%2d-%2d", &start_year, &start_month, &start_day) != 3 ||
       std::sscanf(s_project_due, "%4d-%2d-%2d", &due_year, &due_month, &due_day) != 3) return false;
    struct tm start{};
    struct tm due{};
    start.tm_year = start_year - 1900; start.tm_mon = start_month - 1; start.tm_mday = start_day;
    due.tm_year = due_year - 1900; due.tm_mon = due_month - 1; due.tm_mday = due_day;
    const time_t start_epoch = mktime(&start);
    const time_t due_epoch = mktime(&due);
    if(due_epoch <= start_epoch) return false;
    const int64_t span_days = (static_cast<int64_t>(due_epoch) - start_epoch) / 86400;
    const time_t milestone_epoch = start_epoch + (span_days * milestone / 4) * 86400;
    struct tm milestone_tm{};
    if(localtime_r(&milestone_epoch, &milestone_tm) == nullptr) return false;
    static constexpr const char *months[] = {
        "ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic"
    };
    std::snprintf(destination, capacity, "%02d %s - 23:59", milestone_tm.tm_mday,
                  months[milestone_tm.tm_mon]);
    return true;
}

static void create_project_breakdown(lv_obj_t *screen)
{
    adopt_generated_screen(screen, desglose_b_create);
    char date[32];
    static constexpr const char *date_templates[] = {
        "18 Oct - 23:59", "23 Oct - 23:59", "31 Oct - 23:59", "03 Nov - 23:59"
    };
    for(uint8_t milestone = 1; milestone <= 4; ++milestone) {
        if(project_milestone_date(milestone, date, sizeof(date))) {
            set_generated_label(screen, date_templates[milestone - 1], date);
        }
    }
    set_generated_label(screen, "Entregar 5 fuentes académicas iniciales", "Hito 1 · Preparación");
    set_generated_label(screen, "Lista final de 15 fuentes y casos de estudio \ncompletada.", "Hito 2 · Avance");
    set_generated_label(screen, "Lectura y anotación de todas las fuentes finalizada.", "Hito 3 · Revisión");
    set_generated_label(screen, "Esquema de investigación", s_project_name);
    add_action_area(screen, 58, 47, 58, 58, UiAction::OpenNotifications);
}

static void update_project_heading(lv_obj_t *screen)
{
    char heading[80];
    std::snprintf(heading, sizeof(heading), "Línea de tiempo\n%.48s",
                  s_project_name[0] ? s_project_name : "Nuevo proyecto");
    if(lv_obj_t *title = find_generated_label(screen, "Línea de tiempo\nFinal Research Paper")) {
        lv_label_set_text(title, heading);
    }
    if(lv_obj_t *subject = find_generated_label(screen, "Seminario de Investigación de Diseño")) {
        lv_label_set_text(subject, s_project_subject[0] ? s_project_subject : "Proyecto HAVIT");
    }
}

static void project_calendar_day_event(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    s_ui_feedback_started_us = esp_timer_get_time();
    s_ui_feedback_pending = true;
    const uint8_t day = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    if(day == 0 || day > calendar_days_in_month(s_project_calendar_year, s_project_calendar_month)) return;
    s_project_calendar_selected_day = day;
    std::snprintf(s_project_due, sizeof(s_project_due), "%04u-%02u-%02u",
                  static_cast<unsigned>(s_project_calendar_year),
                  static_cast<unsigned>(s_project_calendar_month),
                  static_cast<unsigned>(day));
    update_project_calendar_view();
}

static bool project_calendar_can_shift(int direction)
{
    (void)direction;
    return true;
}

static void update_project_calendar_view(void)
{
    static constexpr const char *months[] = {
        "enero", "febrero", "marzo", "abril", "mayo", "junio",
        "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"
    };
    if(s_project_calendar_month_label == nullptr || !lv_obj_is_valid(s_project_calendar_month_label)) return;
    lv_label_set_text(s_project_calendar_month_label, months[s_project_calendar_month - 1]);
    char year[8];
    std::snprintf(year, sizeof(year), "%u", static_cast<unsigned>(s_project_calendar_year));
    lv_label_set_text(s_project_calendar_year_label, year);
    struct tm first{};
    first.tm_year = static_cast<int>(s_project_calendar_year) - 1900;
    first.tm_mon = static_cast<int>(s_project_calendar_month) - 1;
    first.tm_mday = 1;
    const time_t first_epoch = mktime(&first);
    localtime_r(&first_epoch, &first);
    const int first_column = (first.tm_wday + 6) % 7;
    const int days = calendar_days_in_month(s_project_calendar_year, s_project_calendar_month);
    static constexpr uint32_t colors[] = {0x8798B8, 0x77B7DB, 0xA99DCB, 0x8798B8, 0x77B7DB, 0xA99DCB, 0xFFFFFF};
    for(int day = 1; day <= days; ++day) {
        const int cell = first_column + day - 1;
        const int column = cell % 7;
        const int row = cell / 7;
        const int x = 79 + column * 29;
        const int y = 145 + row * 23;
        ProjectCalendarDayWidgets &widgets = s_project_calendar_day_widgets[day - 1];
        const bool selected = day == s_project_calendar_selected_day;
        lv_obj_set_pos(widgets.button, x, y);
        lv_obj_clear_flag(widgets.button, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(widgets.label, "%d", day);
        lv_obj_set_style_bg_color(widgets.button, lv_color_hex(selected ? 0xD0E4BF : 0x000000), 0);
        lv_obj_set_style_bg_opa(widgets.button, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(widgets.label, lv_color_hex(selected ? 0x000000 : colors[column]), 0);
    }
    for(int day = days; day < 31; ++day) {
        lv_obj_add_flag(s_project_calendar_day_widgets[day].button, LV_OBJ_FLAG_HIDDEN);
    }
}

static void create_project_calendar(lv_obj_t *screen)
{
    s_project_calendar_month_label = nullptr;
    s_project_calendar_year_label = nullptr;
    for(auto &day : s_project_calendar_day_widgets) day = {};
    adopt_generated_screen(screen, add_task_p5_create);
    update_project_heading(screen);
    s_project_calendar_month_label = find_generated_label(screen, "octubre");
    s_project_calendar_year_label = find_generated_label(screen, "2026");
    if(s_project_calendar_month_label == nullptr || s_project_calendar_year_label == nullptr) return;

    lv_obj_t *grid_background = lv_obj_create(screen);
    lv_obj_remove_style_all(grid_background);
    lv_obj_set_pos(grid_background, 78, 143);
    lv_obj_set_size(grid_background, 205, 145);
    lv_obj_set_style_bg_color(grid_background, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(grid_background, LV_OPA_COVER, 0);
    lv_obj_clear_flag(grid_background, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(grid_background, LV_OBJ_FLAG_CLICKABLE);
    for(uint32_t day = 0; day < 31; ++day) {
        ProjectCalendarDayWidgets &widgets = s_project_calendar_day_widgets[day];
        widgets.button = lv_obj_create(screen);
        lv_obj_remove_style_all(widgets.button);
        lv_obj_set_size(widgets.button, 27, 22);
        lv_obj_set_style_radius(widgets.button, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(widgets.button, LV_OPA_TRANSP, 0);
        lv_obj_add_flag(widgets.button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(widgets.button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(widgets.button, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(widgets.button, project_calendar_day_event, LV_EVENT_CLICKED,
                            reinterpret_cast<void *>(static_cast<uintptr_t>(day + 1)));
        widgets.label = lv_label_create(widgets.button);
        lv_label_set_text_fmt(widgets.label, "%u", static_cast<unsigned>(day + 1));
        lv_obj_set_pos(widgets.label, 0, 0);
        lv_obj_set_size(widgets.label, 27, 22);
        lv_obj_set_style_text_font(widgets.label, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_align(widgets.label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_clear_flag(widgets.label, LV_OBJ_FLAG_CLICKABLE);
    }

    lv_obj_t *previous = lv_label_create(screen);
    lv_label_set_text(previous, "‹");
    lv_obj_set_pos(previous, 39, 174);
    lv_obj_set_size(previous, 26, 28);
    lv_obj_set_style_text_font(previous, font_instrument_sans_regular_20, 0);
    lv_obj_set_style_text_color(previous, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(previous, LV_TEXT_ALIGN_CENTER, 0);
    add_action_area(screen, 39, 170, 30, 36, UiAction::PreviousProjectMonth);

    add_action_area(screen, 286, 171, 44, 42, UiAction::NextProjectMonth);
    add_action_area(screen, 162, 296, 36, 37, UiAction::OpenProjectBreakdown);
    update_project_calendar_view();
}

static void welcome_timeout(lv_timer_t *timer)
{
    if(timer != nullptr) lv_timer_del(timer);
    s_welcome_timer = nullptr;
    if(s_app.state.screen == HAVIT_SCREEN_WELCOME_BACK) {
        const havit_app_result_t result = havit_app_dispatch(&s_app, HAVIT_EVENT_CONTINUE, nullptr);
        if(result == HAVIT_APP_OK) s_ui_dirty = true;
    }
}

static void render_app_view(lv_obj_t *screen)
{
    s_quick_name_input = nullptr;
    s_quick_duration_select = nullptr;
    s_keyboard = nullptr;
    s_project_name_input = nullptr;
    s_project_subject_input = nullptr;
    s_project_description_input = nullptr;
    s_project_due_input = nullptr;
    s_project_start_input = nullptr;
    s_timer_label = nullptr;
    s_quick_timer_title_label = nullptr;
    s_project_notice_sound = nullptr;
    s_project_notice_light = nullptr;
    s_project_notice_tone = nullptr;
    switch(s_app_view) {
        case AppView::Today:
            adopt_generated_screen(screen, today_c_create);
            {
                s_prototype_views.today_count_label = find_generated_label(screen, "0 tareas");
                if(s_prototype_views.today_count_label == nullptr) {
                    s_prototype_views.today_count_label = find_generated_label(screen, "1 tarea");
                }
                s_prototype_views.today_date_label = find_generated_label(screen, "10 de octubre");
                if(s_prototype_views.today_date_label == nullptr) {
                    s_prototype_views.today_date_label = find_generated_label(screen, "18 de octubre");
                }
                const uint32_t count = s_synced_task_count + s_local_tasks.count;
                char count_text[24];
                char date_text[40];
                std::snprintf(count_text, sizeof(count_text), "%lu tarea%s",
                              static_cast<unsigned long>(count), count == 1 ? "" : "s");
                current_date_label(date_text, sizeof(date_text));
                set_generated_label(screen, "0 tareas", count_text);
                set_generated_label(screen, "1 tarea", count_text);
                set_generated_label(screen, "10 de octubre", date_text);
                set_generated_label(screen, "18 de octubre", date_text);
            }
            add_action_area(screen, 140, 34, 80, 72, UiAction::OpenSettings);
            add_action_area(screen, 20, 140, 95, 82, UiAction::OpenTaskList);
            add_action_area(screen, 245, 140, 95, 82, UiAction::OpenAgendaDay);
            add_action_area(screen, 140, 250, 80, 82, UiAction::OpenAddMenu);
            if(s_quick_running || s_quick_expired) {
                add_action_area(screen, 116, 116, 129, 129, UiAction::OpenQuickTimer);
            }
            break;
        case AppView::AccountSettings: create_account_settings(screen); break;
        case AppView::AddMenu:
            adopt_generated_screen(screen, today_b_create);
            add_action_area(screen, 20, 130, 105, 90, UiAction::OpenProjectForm);
            add_action_area(screen, 235, 130, 105, 90, UiAction::OpenQuickForm);
            add_action_area(screen, 145, 35, 70, 54, UiAction::GoBack);
            break;
        case AppView::TaskList: create_task_list(screen); break;
        case AppView::QuickForm: create_generated_quick_form(screen); break;
        case AppView::QuickTimer: create_generated_timer(screen); break;
        case AppView::QuickComplete:
            adopt_generated_screen(screen, add_task_p1_d_create);
            if(lv_obj_t *name = find_generated_label(screen, "Humanidades")) {
                lv_label_set_text(name, s_quick_name[0] ? s_quick_name : "Tarea rápida");
            }
            add_action_area(screen, 149, 149, 62, 62, UiAction::QuickComplete);
            break;
        case AppView::ProjectSaved: create_project_saved(screen); break;
        case AppView::ProjectForm: create_project_inputs(screen, false); break;
        case AppView::ProjectDetails: create_project_inputs(screen, true); break;
        case AppView::ProjectSchedule: create_project_schedule(screen); break;
        case AppView::ProjectGenerate:
            adopt_generated_screen(screen, add_task_p4_create);
            add_action_area(screen, 162, 29, 36, 37, UiAction::GoBack);
            add_action_area(screen, 57, 142, 246, 76, UiAction::GenerateProjectTimeline);
            break;
        case AppView::ProjectTimeline:
            adopt_generated_screen(screen, add_task_p5_e_create);
            update_project_heading(screen);
            add_action_area(screen, 162, 296, 36, 37, UiAction::OpenProjectCalendar);
            break;
        case AppView::ProjectCalendar:
            create_project_calendar(screen);
            break;
        case AppView::ProjectBreakdown:
            create_project_breakdown(screen);
            break;
        case AppView::ProjectNotifications:
            create_project_notifications(screen);
            break;
        case AppView::AgendaDay:
            create_calendar_view(screen);
            break;
        case AppView::AgendaWeek:
            adopt_generated_screen(screen, esta_semana_create);
            {
                char range_text[40];
                current_week_label(range_text, sizeof(range_text));
                set_generated_label(screen, "12-18 de octubre", range_text);
                uint8_t week_indices[LOCAL_TASK_CAPACITY]{};
                const uint32_t week_count = collect_week_task_indices(week_indices, LOCAL_TASK_CAPACITY);
                uint32_t label_index = 0;
                set_local_agenda_labels(screen, week_indices, week_count, &label_index);
            }
            add_action_area(screen, 150, 205, 58, 50, UiAction::OpenAgendaDay);
            add_action_area(screen, 162, 296, 36, 37, UiAction::OpenTaskList);
            break;
    }
}

static lv_obj_t *find_design_ring(lv_obj_t *root)
{
    if(lv_obj_get_width(root) == 350 && lv_obj_get_height(root) == 350) return root;
    const uint32_t count = lv_obj_get_child_count(root);
    for(uint32_t i = 0; i < count; ++i) {
        if(lv_obj_t *found = find_design_ring(lv_obj_get_child(root, i))) return found;
    }
    return nullptr;
}

static void remove_design_ring(lv_obj_t *root)
{
    if(lv_obj_t *ring = find_design_ring(root)) lv_obj_delete(ring);
}

static bool is_prototype_view(AppView view)
{
    return view == AppView::Today || view == AppView::QuickForm || view == AppView::QuickTimer;
}

static void clear_active_view_refs()
{
    s_quick_name_input = nullptr;
    s_quick_duration_select = nullptr;
    s_keyboard = nullptr;
    s_timer_label = nullptr;
    s_quick_timer_title_label = nullptr;
    s_project_name_input = nullptr;
    s_project_subject_input = nullptr;
    s_project_description_input = nullptr;
    s_project_due_input = nullptr;
    s_project_start_input = nullptr;
    s_project_notice_sound = nullptr;
    s_project_notice_light = nullptr;
    s_project_notice_tone = nullptr;
}

static void restore_cached_view_refs(AppView view)
{
    clear_active_view_refs();
    if(view == AppView::QuickForm) {
        s_quick_name_input = s_prototype_views.quick_name_input;
        s_quick_duration_select = s_prototype_views.quick_duration_select;
        s_keyboard = s_prototype_views.quick_keyboard;
    } else if(view == AppView::QuickTimer) {
        s_timer_label = s_prototype_views.timer_label;
        s_quick_timer_title_label = s_prototype_views.timer_title_label;
    }
}

static void save_active_cached_view_refs(void)
{
    if(s_prototype_views.root == nullptr) return;
    if(s_active_prototype_view == AppView::QuickForm) {
        s_prototype_views.quick_name_input = s_quick_name_input;
        s_prototype_views.quick_duration_select = s_quick_duration_select;
        s_prototype_views.quick_keyboard = s_keyboard;
        if(s_keyboard != nullptr) lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
    } else if(s_active_prototype_view == AppView::QuickTimer) {
        s_prototype_views.timer_label = s_timer_label;
        s_prototype_views.timer_title_label = s_quick_timer_title_label;
    }
}

static void refresh_cached_today(void)
{
    if(s_prototype_views.today == nullptr) return;
    const uint32_t count = s_synced_task_count + s_local_tasks.count;
    char count_text[24];
    char date_text[40];
    std::snprintf(count_text, sizeof(count_text), "%lu tarea%s",
                  static_cast<unsigned long>(count), count == 1 ? "" : "s");
    current_date_label(date_text, sizeof(date_text));
    if(s_prototype_views.today_count_label != nullptr &&
       std::strcmp(lv_label_get_text(s_prototype_views.today_count_label), count_text) != 0) {
        lv_label_set_text(s_prototype_views.today_count_label, count_text);
    }
    if(s_prototype_views.today_date_label != nullptr &&
       std::strcmp(lv_label_get_text(s_prototype_views.today_date_label), date_text) != 0) {
        lv_label_set_text(s_prototype_views.today_date_label, date_text);
    }
}

static lv_obj_t *create_prototype_page(AppView view)
{
    lv_obj_t *page = lv_obj_create(s_prototype_views.root);
    lv_obj_remove_style_all(page);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_size(page, SCREEN_SIZE, SCREEN_SIZE);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);

    const AppView previous_view = s_app_view;
    s_app_view = view;
    render_app_view(page);
    s_app_view = previous_view;
    remove_design_ring(page);

    if(view == AppView::Today) {
        s_prototype_views.today = page;
    } else if(view == AppView::QuickForm) {
        s_prototype_views.quick_form = page;
        s_prototype_views.quick_name_input = s_quick_name_input;
        s_prototype_views.quick_duration_select = s_quick_duration_select;
        s_prototype_views.quick_keyboard = s_keyboard;
    } else if(view == AppView::QuickTimer) {
        s_prototype_views.quick_timer = page;
        s_prototype_views.timer_label = s_timer_label;
        s_prototype_views.timer_title_label = s_quick_timer_title_label;
    }
    return page;
}

static void refresh_cached_timer(void)
{
    if(s_prototype_views.timer_title_label != nullptr) {
        lv_label_set_text(s_prototype_views.timer_title_label,
                          s_quick_paused ? "En pausa" : (s_quick_name[0] ? s_quick_name : "Tarea Rápida"));
    }
    if(s_prototype_views.timer_label != nullptr) {
        const int64_t remaining = s_quick_paused ? s_quick_paused_remaining_us :
            std::max<int64_t>(0, s_quick_deadline_us - esp_timer_get_time());
        const uint32_t seconds = static_cast<uint32_t>((remaining + 999999) / 1000000);
        char value[16];
        std::snprintf(value, sizeof(value), "%02u:%02u:%02u",
                      static_cast<unsigned>(seconds / 3600),
                      static_cast<unsigned>((seconds / 60) % 60),
                      static_cast<unsigned>(seconds % 60));
        if(std::strcmp(lv_label_get_text(s_prototype_views.timer_label), value) != 0) {
            lv_label_set_text(s_prototype_views.timer_label, value);
        }
    }
}

static void show_prototype_view(AppView view, lv_obj_t *initial_root)
{
    const int64_t view_started = esp_timer_get_time();
    save_active_cached_view_refs();
    if(s_prototype_views.root == nullptr) {
        s_prototype_views.root = initial_root != nullptr ? initial_root : lv_obj_create(nullptr);
        lv_obj_set_size(s_prototype_views.root, SCREEN_SIZE, SCREEN_SIZE);
        lv_obj_set_style_bg_color(s_prototype_views.root, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(s_prototype_views.root, LV_OPA_COVER, 0);
        lv_obj_clear_flag(s_prototype_views.root, LV_OBJ_FLAG_SCROLLABLE);

        s_prototype_views.ring = lv_obj_create(s_prototype_views.root);
        lv_obj_remove_style_all(s_prototype_views.ring);
        lv_obj_set_size(s_prototype_views.ring, 350, 350);
        lv_obj_align(s_prototype_views.ring, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(s_prototype_views.ring, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(s_prototype_views.ring, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(s_prototype_views.ring, 0, 0);
        lv_obj_clear_flag(s_prototype_views.ring, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(s_prototype_views.ring, LV_OBJ_FLAG_SCROLLABLE);
        ui_ring_overlay(s_prototype_views.root);
    }

    lv_obj_t **page_slot = nullptr;
    if(view == AppView::Today) page_slot = &s_prototype_views.today;
    else if(view == AppView::QuickForm) page_slot = &s_prototype_views.quick_form;
    else if(view == AppView::QuickTimer) page_slot = &s_prototype_views.quick_timer;
    if(page_slot == nullptr) return;

    const bool cache_hit = *page_slot != nullptr;
    lv_obj_t *page = cache_hit ? *page_slot : create_prototype_page(view);
    s_ui_build_duration_us = cache_hit ? 0 : static_cast<uint32_t>(esp_timer_get_time() - view_started);

    if(s_prototype_views.today != nullptr) lv_obj_add_flag(s_prototype_views.today, LV_OBJ_FLAG_HIDDEN);
    if(s_prototype_views.quick_form != nullptr) lv_obj_add_flag(s_prototype_views.quick_form, LV_OBJ_FLAG_HIDDEN);
    if(s_prototype_views.quick_timer != nullptr) lv_obj_add_flag(s_prototype_views.quick_timer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(s_prototype_views.ring);
    s_active_prototype_view = view;
    restore_cached_view_refs(view);

    if(view == AppView::Today) refresh_cached_today();
    if(view == AppView::QuickForm && s_quick_form_reset_requested && s_quick_name_input != nullptr) {
        lv_textarea_set_text(s_quick_name_input, "");
        s_quick_form_reset_requested = false;
    }
    if(view == AppView::QuickTimer) refresh_cached_timer();

    if(lv_screen_active() != s_prototype_views.root) {
        /* Delete the transient screen on the way in; keep the prototype root cached. */
        lv_screen_load_anim(s_prototype_views.root, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);
    }
    lv_obj_invalidate(s_prototype_views.root);
    log_ui_heap(cache_hit ? "prototype-cache-hit" : "prototype-cache-miss");
    ESP_LOGI(TAG, "UI prototype view: view=%u cache=%s build=%luus page_children=%lu",
             static_cast<unsigned>(view), cache_hit ? "hit" : "miss",
             static_cast<unsigned long>(s_ui_build_duration_us),
             static_cast<unsigned long>(lv_obj_get_child_count(page)));
}

static lv_obj_t *add_setup_button(lv_obj_t *screen, const char *text, int y, UiAction action)
{
    add_card(screen, 60, y, 240, 35, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    lv_obj_t *label = add_label(screen, text, y + 8, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    lv_obj_set_width(label, 240);
    lv_obj_set_x(label, 60);
    add_action_area(screen, 60, y, 240, 35, action);
    return label;
}

static void create_wifi_setup_screen(lv_obj_t *screen)
{
    add_circle_outline(screen);
    if(s_manual_wifi_view) {
        add_label(screen, "Wi-Fi de configuración", 63, s_font_20, WHITE, LV_OPA_COVER);
        add_label(screen, s_setup_ssid, 112, s_font_15, ACCENT, LV_OPA_COVER);
        char password_line[40];
        std::snprintf(password_line, sizeof(password_line), "Clave: %s", s_setup_password);
        add_label(screen, password_line, 145, s_font_15, WHITE, LV_OPA_COVER);
        add_label(screen, "Únete a esta red y abre\nhttp://192.168.4.1", 178, s_font_15, WHITE, 184);
        add_setup_button(screen, s_ble_available ? "Red de configuración activa" : "Activar red de configuración", 236, UiAction::ToggleBluetooth);
        add_setup_button(screen, "Volver", 281, UiAction::ShowWifiSetup);
        return;
    }
    if(s_forget_confirm) {
        add_label(screen, "¿Olvidar redes Wi-Fi?", 95, s_font_20, WHITE, LV_OPA_COVER);
        add_label(screen, "Tendrás que configurar tu red de nuevo.\nTu cuenta seguirá guardada.", 145, s_font_15, WHITE, 184);
        add_setup_button(screen, "Sí, olvidar redes", 220, UiAction::ConfirmForgetWifi);
        add_setup_button(screen, "Cancelar", 266, UiAction::ShowWifiSetup);
        return;
    }
    add_label(screen, "Conexiones", 38, s_font_20, WHITE, LV_OPA_COVER);
    s_wifi_status_label = add_label(screen, s_wifi_connected ? "Wi-Fi: conectado" : "Wi-Fi: sin conexión", 70, s_font_15,
                                    s_wifi_connected ? 0x60D394 : WHITE, LV_OPA_COVER);
    s_wifi_ssid_label = add_label(screen, s_network_name[0] ? s_network_name : "Sin red conectada", 94, s_font_15, WHITE, 184);
    s_wifi_ip_label = add_label(screen, s_network_ip[0] ? s_network_ip : "Sin dirección IP", 116, s_font_15, WHITE, 160);
    s_wifi_ble_status_label = add_label(screen, s_ble_available ? "Bluetooth: disponible para configurar" : "Bluetooth de configuración: apagado",
                                        139, s_font_15, WHITE, 184);
    s_wifi_ble_button_label = add_setup_button(screen, s_ble_available ? "Apagar Bluetooth" : "Activar Bluetooth", 160,
                                               UiAction::ToggleBluetooth);
    s_wifi_toggle_button_label = add_setup_button(screen, s_wifi_connected ? "Desconectar Wi-Fi" : "Reconectar Wi-Fi", 200,
                                                  UiAction::ToggleWifi);
    add_setup_button(screen, "Olvidar redes Wi-Fi", 240, UiAction::ForgetWifi);
    add_card(screen, 60, 280, 150, 35, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Wi-Fi manual", 80, 288, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 60, 280, 150, 35, UiAction::ShowManualWifi);
    add_card(screen, 218, 280, 82, 35, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Volver", 235, 288, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 218, 280, 82, 35, UiAction::CloseOverlay);
}

static void create_pairing_screen(lv_obj_t *screen)
{
    char code[sizeof(s_pairing_code)];
    char message[sizeof(s_pairing_message)];
    portENTER_CRITICAL(&s_text_lock);
    std::snprintf(code, sizeof(code), "%s", s_pairing_code);
    std::snprintf(message, sizeof(message), "%s", s_pairing_message);
    portEXIT_CRITICAL(&s_text_lock);
    add_circle_outline(screen);
    add_label(screen, "Vincula tu HAVIT", 46, s_font_22, WHITE, LV_OPA_COVER);
    s_pairing_status_label = add_label(screen, s_wifi_connected ? "Wi-Fi conectado" : "Wi-Fi sin conexión", 83, s_font_15,
                                       s_wifi_connected ? 0x60D394 : 0xFFD080, LV_OPA_COVER);
    s_pairing_network_label = add_label(screen, s_wifi_connected ? s_network_name : (s_ble_available ? "Bluetooth disponible" : "Bluetooth apagado"),
                                        106, s_font_15, WHITE, 184);
    s_pairing_code_label = add_label(screen, code[0] && s_wifi_connected ? code : "···· ····", 140, s_font_24, ACCENT, LV_OPA_COVER);
    s_pairing_message_label = add_label(screen, message, 182, s_font_15, WHITE, 220);
    lv_obj_set_width(s_pairing_message_label, 280);
    lv_obj_set_x(s_pairing_message_label, 40);
    s_pairing_button_label = add_setup_button(screen, s_pairing_running ? "Solicitar nuevo PIN" : "Reintentar / pedir PIN", 242,
                                              UiAction::RetryPairing);
    add_card(screen, 60, 286, 150, 35, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Conexiones", 87, 294, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 60, 286, 150, 35, UiAction::ShowWifiSetup);
    add_card(screen, 218, 286, 82, 35, CIRCLE_BUTTON, LV_RADIUS_CIRCLE);
    add_plain_label(screen, "Volver", 235, 294, s_font_15, UI_BACKGROUND, LV_OPA_COVER);
    add_action_area(screen, 218, 286, 82, 35, UiAction::CloseOverlay);
}

static void render_screen(lv_obj_t *screen, havit_screen_t screen_id)
{
    s_home_wifi_label = nullptr;
    s_wifi_status_label = nullptr;
    s_wifi_ssid_label = nullptr;
    s_wifi_ip_label = nullptr;
    s_wifi_ble_status_label = nullptr;
    s_wifi_ble_button_label = nullptr;
    s_wifi_toggle_button_label = nullptr;
    s_pairing_status_label = nullptr;
    s_pairing_network_label = nullptr;
    s_pairing_code_label = nullptr;
    s_pairing_message_label = nullptr;
    s_pairing_button_label = nullptr;
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_BACKGROUND), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    if(s_wifi_setup_view) {
        create_wifi_setup_screen(screen);
        return;
    }
    if(s_pairing_view) {
        create_pairing_screen(screen);
        return;
    }

    switch(screen_id) {
        case HAVIT_SCREEN_ONBOARDING:
            adopt_generated_screen(screen, intro_4_create);
            add_action_area(screen, 142, 202, 76, 72, UiAction::ContinueOnboarding);
            break;
        case HAVIT_SCREEN_SIGN_IN:
            adopt_generated_screen(screen, log_in_create);
            add_action_area(screen, 143, 208, 74, 74, UiAction::PairDevice);
            break;
        case HAVIT_SCREEN_WELCOME_BACK:
            adopt_generated_screen(screen, welcome_back_create);
            set_generated_label(screen, "Leo!", s_display_name[0] ? s_display_name : "HAVIT");
            add_action_area(screen, 18, 24, 324, 312, UiAction::ContinueHome);
            if(s_welcome_timer == nullptr) s_welcome_timer = lv_timer_create(welcome_timeout, 800, nullptr);
            break;
        case HAVIT_SCREEN_HOME: render_app_view(screen); break;
    }
}

static void finish_render_and_load(lv_obj_t *screen, bool load_screen)
{
    if(s_app.state.screen == HAVIT_SCREEN_HOME && !s_wifi_setup_view && !s_pairing_view &&
       is_prototype_view(s_app_view)) {
        show_prototype_view(s_app_view, load_screen ? nullptr : screen);
        return;
    }
    if(lv_screen_active() == s_prototype_views.root) save_active_cached_view_refs();
    const int64_t started = esp_timer_get_time();
    render_screen(screen, s_app.state.screen);
    const uint32_t built_ms = static_cast<uint32_t>((esp_timer_get_time() - started) / 1000);
    s_ui_build_duration_us = static_cast<uint32_t>(esp_timer_get_time() - started);
    ESP_LOGI(TAG, "UI build: route=%u view=%u root_children=%lu time=%lums heap_internal=%lu heap_psram=%lu",
             static_cast<unsigned>(s_app.state.screen), static_cast<unsigned>(s_app_view),
             static_cast<unsigned long>(lv_obj_get_child_count(screen)),
             static_cast<unsigned long>(built_ms),
             static_cast<unsigned long>(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)),
             static_cast<unsigned long>(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)));
    log_ui_heap("after-screen-build");
    if(load_screen) {
        const bool preserve_prototype_root = lv_screen_active() == s_prototype_views.root;
        lv_screen_load_anim(screen, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, !preserve_prototype_root);
    }
}

static void transition_to_current_screen(void)
{
    s_intro_blink_active = false;
    begin_ui_measurement();
    if(s_app.state.screen == HAVIT_SCREEN_HOME && !s_wifi_setup_view && !s_pairing_view &&
       is_prototype_view(s_app_view)) {
        show_prototype_view(s_app_view);
        return;
    }
    lv_obj_t *next_screen = lv_obj_create(nullptr);
    finish_render_and_load(next_screen, true);
}

static void start_pairing_task(void *);

static void action_event(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    const auto action = static_cast<UiAction>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    s_action_touch_us = s_touch_detected_us;
    s_touch_detected_us = 0;
    s_ui_feedback_started_us = esp_timer_get_time();
    s_ui_feedback_pending = true;
    ESP_LOGI(TAG, "UI action %u clicked on screen %u", static_cast<unsigned>(action),
             static_cast<unsigned>(s_app.state.screen));
    havit_app_result_t result = HAVIT_APP_OK;

    switch(action) {
        case UiAction::ContinueOnboarding:
            result = havit_app_dispatch(&s_app, HAVIT_EVENT_CONTINUE, nullptr);
            break;
        case UiAction::PairDevice:
        case UiAction::RetryPairing:
            s_wifi_setup_view = false;
            s_pairing_view = true;
            s_pairing_requested = true;
            s_pairing_cancelled = s_pairing_running.load();
            portENTER_CRITICAL(&s_text_lock);
            s_pairing_code[0] = '\0';
            portEXIT_CRITICAL(&s_text_lock);
            s_pairing_widgets_dirty = true;
            set_pairing_message(s_wifi_connected ? "Solicitando PIN…" : "En la web: Buscar HAVIT por Bluetooth.\nConfigura tu Wi-Fi de 2.4 GHz.");
            s_ui_dirty = true;
            return;
        case UiAction::ShowManualWifi:
            s_manual_wifi_view = true;
            s_ui_dirty = true;
            return;
        case UiAction::EnableBluetooth:
            s_network_command = 1;
            return;
        case UiAction::DisableBluetooth:
            s_network_command = 2;
            return;
        case UiAction::ToggleBluetooth:
            s_network_command = s_ble_available ? 2 : 1;
            return;
        case UiAction::ToggleWifi:
            s_network_command = s_wifi_connected ? 4 : 5;
            return;
        case UiAction::ForgetWifi:
            s_forget_confirm = true;
            s_ui_dirty = true;
            return;
        case UiAction::ConfirmForgetWifi:
            s_forget_confirm = false;
            s_pairing_cancelled = true;
            s_network_command = 3;
            s_ui_dirty = true;
            return;
        case UiAction::ContinueHome:
            if(s_welcome_timer != nullptr) {
                lv_timer_del(s_welcome_timer);
                s_welcome_timer = nullptr;
            }
            if(s_app.state.auth_state == HAVIT_AUTH_AUTHENTICATED) {
                s_app.state.screen = HAVIT_SCREEN_HOME;
            } else {
                result = havit_app_dispatch(&s_app, HAVIT_EVENT_CONTINUE, nullptr);
            }
            break;
        case UiAction::ShowWifiSetup:
            s_manual_wifi_view = false;
            s_forget_confirm = false;
            s_resume_pairing = s_pairing_view || s_resume_pairing;
            s_pairing_view = false;
            s_wifi_setup_view = true;
            transition_to_current_screen();
            return;
        case UiAction::CloseOverlay:
            if(s_wifi_setup_view && s_resume_pairing) {
                s_pairing_view = true;
                s_resume_pairing = false;
            } else {
                s_pairing_cancelled = true;
                s_pairing_requested = false;
                s_pairing_view = false;
            }
            s_wifi_setup_view = false;
            transition_to_current_screen();
            return;
        case UiAction::OpenAddMenu: s_app_view = AppView::AddMenu; break;
        case UiAction::OpenSettings:
            s_previous_app_view = s_app_view;
            s_app_view = AppView::AccountSettings;
            break;
        case UiAction::OpenConnections:
            s_manual_wifi_view = false;
            s_forget_confirm = false;
            s_resume_pairing = false;
            s_previous_app_view = s_app_view;
            s_wifi_setup_view = true;
            render_async(nullptr);
            return;
        case UiAction::OpenTaskList:
            s_task_date_filter[0] = '\0';
            s_task_completed_filter = false;
            s_task_list_page = 0;
            s_app_view = AppView::TaskList;
            break;
        case UiAction::OpenAgendaDay:
        case UiAction::OpenCalendar:
            initialize_calendar();
            s_app_view = AppView::AgendaDay;
            break;
        case UiAction::OpenAgendaWeek: s_app_view = AppView::AgendaWeek; break;
        case UiAction::OpenTasksForSelectedDay:
            initialize_calendar();
            if(s_calendar_initialized) {
                std::snprintf(s_task_date_filter, sizeof(s_task_date_filter), "%04u-%02u-%02u",
                              static_cast<unsigned>(s_calendar_year),
                              static_cast<unsigned>(s_calendar_month),
                              static_cast<unsigned>(s_calendar_selected_day));
            } else {
                s_task_date_filter[0] = '\0';
            }
            s_task_completed_filter = false;
            s_task_list_page = 0;
            s_app_view = AppView::TaskList;
            break;
        case UiAction::ClearTaskDateFilter:
            s_task_date_filter[0] = '\0';
            s_task_list_page = 0;
            break;
        case UiAction::ToggleTaskFilter:
            s_task_completed_filter = !s_task_completed_filter;
            s_task_list_page = 0;
            break;
        case UiAction::PreviousCalendarMonth:
            initialize_calendar();
            if(s_calendar_initialized) {
                if(s_calendar_month == 1) { --s_calendar_year; s_calendar_month = 12; }
                else --s_calendar_month;
                s_calendar_selected_day = 1;
            }
            break;
        case UiAction::NextCalendarMonth:
            initialize_calendar();
            if(s_calendar_initialized) {
                if(s_calendar_month == 12) { ++s_calendar_year; s_calendar_month = 1; }
                else ++s_calendar_month;
                s_calendar_selected_day = 1;
            }
            break;
        case UiAction::OpenQuickForm:
            s_previous_app_view = s_app_view;
            s_app_view = AppView::QuickForm;
            break;
        case UiAction::OpenQuickTimer: s_app_view = AppView::QuickTimer; break;
        case UiAction::OpenProjectForm:
            s_previous_app_view = s_app_view;
            s_app_view = AppView::ProjectForm;
            break;
        case UiAction::OpenProjectDetails:
            if(s_project_name_input) std::snprintf(s_project_name, sizeof(s_project_name), "%s", lv_textarea_get_text(s_project_name_input));
            if(s_project_subject_input) std::snprintf(s_project_subject, sizeof(s_project_subject), "%s", lv_textarea_get_text(s_project_subject_input));
            if(s_project_name[0] == '\0') { s_ui_dirty = true; return; }
            s_app_view = AppView::ProjectDetails;
            break;
        case UiAction::OpenProjectSchedule:
            if(s_project_due_input) std::snprintf(s_project_due, sizeof(s_project_due), "%s", lv_textarea_get_text(s_project_due_input));
            if(s_project_description_input) std::snprintf(s_project_description, sizeof(s_project_description), "%s", lv_textarea_get_text(s_project_description_input));
            s_app_view = AppView::ProjectSchedule;
            break;
        case UiAction::GenerateProjectTimeline:
            if(s_project_start_input) std::snprintf(s_project_start, sizeof(s_project_start), "%s", lv_textarea_get_text(s_project_start_input));
            if(s_project_due_input) std::snprintf(s_project_due, sizeof(s_project_due), "%s", lv_textarea_get_text(s_project_due_input));
            if(s_app_view == AppView::ProjectSchedule) {
                if(!valid_project_date(s_project_start) || !valid_project_date(s_project_due) ||
                   std::strcmp(s_project_start, s_project_due) >= 0) {
                    s_project_dates_invalid = true;
                    s_ui_dirty = true;
                    return;
                }
                s_project_dates_invalid = false;
                s_app_view = AppView::ProjectGenerate;
            } else {
                /* No remote planning endpoint is connected yet. Build the local
                   date-based plan immediately instead of showing a fake loader. */
                s_app_view = AppView::ProjectTimeline;
            }
            break;
        case UiAction::OpenProjectCalendar:
            {
                int year = 0, month = 0, day = 0;
                if(std::sscanf(s_project_due, "%4d-%2d-%2d", &year, &month, &day) == 3 &&
                   month >= 1 && month <= 12 && day >= 1 && day <= 31) {
                    s_project_calendar_year = static_cast<uint16_t>(year);
                    s_project_calendar_month = static_cast<uint8_t>(month);
                    s_project_calendar_selected_day = static_cast<uint8_t>(day);
                } else {
                    time_t now = time(nullptr);
                    struct tm local_now{};
                    if(localtime_r(&now, &local_now) != nullptr) {
                        s_project_calendar_year = static_cast<uint16_t>(local_now.tm_year + 1900);
                        s_project_calendar_month = static_cast<uint8_t>(local_now.tm_mon + 1);
                        s_project_calendar_selected_day = 0;
                    }
                }
            }
            s_app_view = AppView::ProjectCalendar;
            break;
        case UiAction::OpenProjectBreakdown:
            s_app_view = AppView::ProjectBreakdown;
            break;
        case UiAction::NextProjectMonth:
            if(project_calendar_can_shift(1)) {
                if(s_project_calendar_month == 12) { ++s_project_calendar_year; s_project_calendar_month = 1; }
                else ++s_project_calendar_month;
                s_project_calendar_selected_day = 0;
            }
            break;
        case UiAction::PreviousProjectMonth:
            if(project_calendar_can_shift(-1)) {
                if(s_project_calendar_month == 1) { --s_project_calendar_year; s_project_calendar_month = 12; }
                else --s_project_calendar_month;
                s_project_calendar_selected_day = 0;
            }
            break;
        case UiAction::ShowQuickComplete:
            s_quick_running = false;
            s_quick_expired = true;
            s_app_view = AppView::QuickComplete;
            break;
        case UiAction::QuickStart: {
            if(s_quick_name_input) std::snprintf(s_quick_name, sizeof(s_quick_name), "%s", lv_textarea_get_text(s_quick_name_input));
            if(s_quick_name[0] == '\0') { s_ui_dirty = true; return; }
            s_quick_deadline_us = esp_timer_get_time() + static_cast<int64_t>(s_quick_duration_seconds) * 1000000;
            s_quick_paused = false;
            s_quick_expired = false;
            s_quick_running = true;
            s_app_view = AppView::QuickTimer;
            break;
        }
        case UiAction::QuickPauseResume:
            if(s_quick_paused) {
                s_quick_deadline_us = esp_timer_get_time() + s_quick_paused_remaining_us;
                s_quick_paused = false;
                s_quick_running = true;
            } else {
                s_quick_paused_remaining_us = std::max<int64_t>(0, s_quick_deadline_us - esp_timer_get_time());
                s_quick_paused = true;
                s_quick_running = false;
            }
            s_app_view = AppView::QuickTimer;
            break;
        case UiAction::QuickComplete:
            (void)add_local_task(s_quick_name, "Rápida");
            s_quick_running = false;
            s_quick_expired = false;
            s_quick_name[0] = '\0';
            s_quick_form_reset_requested = true;
            s_app_view = AppView::TaskList;
            break;
        case UiAction::QuickCancel:
            s_quick_running = false;
            s_quick_paused = false;
            s_quick_expired = false;
            s_quick_name[0] = '\0';
            s_quick_form_reset_requested = true;
            s_app_view = s_previous_app_view;
            break;
        case UiAction::SaveProject:
            if(s_project_start_input) std::snprintf(s_project_start, sizeof(s_project_start), "%s", lv_textarea_get_text(s_project_start_input));
            if(s_project_due_input) std::snprintf(s_project_due, sizeof(s_project_due), "%s", lv_textarea_get_text(s_project_due_input));
            if(s_project_description_input) std::snprintf(s_project_description, sizeof(s_project_description), "%s", lv_textarea_get_text(s_project_description_input));
            if(s_project_notice_sound) s_notice_sound = static_cast<uint8_t>(lv_dropdown_get_selected(s_project_notice_sound));
            if(s_project_notice_light) s_notice_light = static_cast<uint8_t>(lv_dropdown_get_selected(s_project_notice_light));
            if(s_project_notice_tone) s_notice_tone = static_cast<uint8_t>(lv_dropdown_get_selected(s_project_notice_tone));
            if(s_project_name[0] == '\0' || !valid_project_date(s_project_due)) { s_ui_dirty = true; return; }
            if(!add_local_task(s_project_name, s_project_subject, s_project_due)) { s_ui_dirty = true; return; }
            (void)nvs_set_u8(s_havit_store_handle, "notice_sound", s_notice_sound);
            (void)nvs_set_u8(s_havit_store_handle, "notice_light", s_notice_light);
            (void)nvs_set_u8(s_havit_store_handle, "notice_tone", s_notice_tone);
            (void)nvs_commit(s_havit_store_handle);
            s_project_saved = true;
            s_app_view = AppView::ProjectSaved;
            break;
        case UiAction::SaveProjectConfirm:
            s_app_view = AppView::Today;
            s_project_name[0] = s_project_subject[0] = s_project_description[0] = s_project_due[0] = s_project_start[0] = '\0';
            break;
        case UiAction::PreviousTaskPage:
            if(s_task_list_page) --s_task_list_page;
            break;
        case UiAction::NextTaskPage:
            if(static_cast<uint32_t>(s_task_list_page + 1) < s_task_page_count) ++s_task_list_page;
            break;
        case UiAction::ToggleTask:
            if(s_local_tasks.count) {
                const uint32_t first = static_cast<uint32_t>(s_task_list_page) * 4;
                const int y = lv_obj_get_y(static_cast<lv_obj_t *>(lv_event_get_target(event)));
                const uint32_t index = first + (y >= 96 ? static_cast<uint32_t>((y - 96) / 46) : 0);
                if(index < s_local_tasks.count) {
                    s_local_tasks.tasks[index].completed = !s_local_tasks.tasks[index].completed;
                    persist_local_tasks();
                }
            }
            break;
        case UiAction::GoBack:
            if(s_app_view == AppView::AccountSettings) s_app_view = AppView::Today;
            else if(s_app_view == AppView::ProjectDetails) s_app_view = AppView::ProjectForm;
            else if(s_app_view == AppView::ProjectSchedule) s_app_view = AppView::ProjectDetails;
            else if(s_app_view == AppView::ProjectGenerate) s_app_view = AppView::ProjectSchedule;
            else if(s_app_view == AppView::ProjectCalendar) s_app_view = AppView::ProjectTimeline;
            else if(s_app_view == AppView::ProjectBreakdown) s_app_view = AppView::ProjectCalendar;
            else if(s_app_view == AppView::AgendaWeek) s_app_view = AppView::AgendaDay;
            else if(s_app_view == AppView::AgendaDay) s_app_view = AppView::Today;
            else if(s_app_view == AppView::QuickTimer) s_app_view = s_previous_app_view;
            else if(s_app_view == AppView::QuickForm || s_app_view == AppView::ProjectForm) s_app_view = s_previous_app_view;
            else s_app_view = AppView::Today;
            break;
        case UiAction::OpenNotifications:
            s_app_view = AppView::ProjectNotifications;
            break;
    }

    if(result != HAVIT_APP_OK) {
        ESP_LOGE(TAG, "Screen action %u failed with app result %d",
                 static_cast<unsigned>(action), static_cast<int>(result));
        return;
    }
    if(s_app_view == AppView::TaskList &&
       (action == UiAction::ToggleTaskFilter || action == UiAction::ClearTaskDateFilter ||
        action == UiAction::PreviousTaskPage || action == UiAction::NextTaskPage ||
        action == UiAction::ToggleTask)) {
        begin_ui_measurement();
        const int64_t started = esp_timer_get_time();
        update_task_list_view();
        s_ui_build_duration_us = static_cast<uint32_t>(esp_timer_get_time() - started);
        ESP_LOGI(TAG, "UI partial update: task list %llums",
                 static_cast<long long>((esp_timer_get_time() - started) / 1000));
        return;
    }
    if(s_app_view == AppView::AgendaDay &&
       (action == UiAction::PreviousCalendarMonth || action == UiAction::NextCalendarMonth)) {
        begin_ui_measurement();
        const int64_t started = esp_timer_get_time();
        update_calendar_view();
        s_ui_build_duration_us = static_cast<uint32_t>(esp_timer_get_time() - started);
        ESP_LOGI(TAG, "UI partial update: calendar %llums",
                 static_cast<long long>((esp_timer_get_time() - started) / 1000));
        return;
    }
    if(s_app_view == AppView::ProjectCalendar &&
       (action == UiAction::PreviousProjectMonth || action == UiAction::NextProjectMonth)) {
        begin_ui_measurement();
        const int64_t started = esp_timer_get_time();
        update_project_calendar_view();
        s_ui_build_duration_us = static_cast<uint32_t>(esp_timer_get_time() - started);
        ESP_LOGI(TAG, "UI partial update: project calendar %llums",
                 static_cast<long long>((esp_timer_get_time() - started) / 1000));
        return;
    }
    transition_to_current_screen();
}

static void add_action_area(lv_obj_t *screen, int x, int y, int width, int height,
                            UiAction action)
{
    lv_obj_t *area = lv_obj_create(screen);
    lv_obj_remove_style_all(area);
    lv_obj_set_pos(area, x, y);
    lv_obj_set_size(area, width, height);
    lv_obj_set_style_bg_color(area, lv_color_hex(WHITE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(area, LV_OPA_20, LV_STATE_PRESSED);
    lv_obj_set_style_radius(area, LV_RADIUS_CIRCLE, LV_STATE_PRESSED);
    lv_obj_add_flag(area, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(area, action_event, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(action)));
}

static void display_render_event(lv_event_t *event)
{
    if(!s_ui_measurement_pending) return;
    const int64_t now = esp_timer_get_time();
    if(lv_event_get_code(event) == LV_EVENT_RENDER_START) s_ui_render_started_us = now;
    else if(lv_event_get_code(event) == LV_EVENT_RENDER_READY) s_ui_render_ready_us = now;
}

static IRAM_ATTR void mark_display_completion(void)
{
    s_display_completed_us_low = static_cast<uint32_t>(esp_timer_get_time());
    s_display_completion_pending = true;
    s_final_display_flush_in_flight = false;
}

static IRAM_ATTR void finish_display_flush_submission(void)
{
    const uint32_t outstanding = s_display_flushes_in_flight.fetch_sub(1) - 1;
    if(outstanding == 0 && s_final_display_flush_queued.exchange(false)) {
        mark_display_completion();
    }
}

static void report_display_completion(void)
{
    if(!s_display_completion_pending.exchange(false) ||
       !s_ui_measurement_pending.exchange(false)) return;

    const uint32_t completed_us = s_display_completed_us_low.load();
    const uint32_t total_us = completed_us - static_cast<uint32_t>(s_ui_navigation_started_us);
    const uint32_t render_ms = s_ui_render_ready_us > 0 ?
        static_cast<uint32_t>((s_ui_render_ready_us - s_ui_navigation_started_us) / 1000) : UINT32_MAX;
    ui_ring_overlay_stats_t ring{};
    ui_ring_overlay_get_stats(&ring);
    ESP_LOGI(TAG, "UI navigation complete: render_ready=%lums dma_complete=%lums build=%luus ring_calls=%lu ring_last=%luus ring_max=%luus ring_avg=%luus",
             static_cast<unsigned long>(render_ms),
             static_cast<unsigned long>(total_us / 1000),
             static_cast<unsigned long>(s_ui_build_duration_us),
             static_cast<unsigned long>(ring.calls),
             static_cast<unsigned long>(ring.last_us),
             static_cast<unsigned long>(ring.max_us),
             static_cast<unsigned long>(ring.calls ? ring.total_us / ring.calls : 0));
    log_ui_heap("navigation-complete");
}

static void flush_display(lv_display_t *display, const lv_area_t *area, uint8_t *pixels)
{
    if(s_ui_feedback_pending) {
        const uint32_t elapsed_ms = static_cast<uint32_t>((esp_timer_get_time() - s_ui_feedback_started_us) / 1000);
        ESP_LOGI(TAG, "UI first display flush after click (not full update): %lums",
                 static_cast<unsigned long>(elapsed_ms));
        s_ui_feedback_pending = false;
    }
    auto *lcd = static_cast<LCD *>(lv_display_get_user_data(display));
    const int width = area->x2 - area->x1 + 1;
    const int height = area->y2 - area->y1 + 1;
    /* ST77916 QSPI expects RGB565 high byte first; LVGL stores it native-endian. */
    lv_draw_sw_rgb565_swap(pixels, static_cast<uint32_t>(width * height));
    // Keep the LVGL DMA buffer owned by the display until the QSPI completion callback.
    const bool is_last_flush = lv_display_flush_is_last(display);
    s_display_flushes_in_flight.fetch_add(1);
    if(is_last_flush) {
        s_final_display_flush_in_flight = true;
        s_final_display_flush_queued = true;
    }
    if(!lcd->drawBitmap(area->x1, area->y1, width, height, pixels, 0)) {
        ESP_LOGE(TAG, "LCD transfer failed for area (%ld,%ld)-(%ld,%ld)",
                 area->x1, area->y1, area->x2, area->y2);
        finish_display_flush_submission();
        lv_display_flush_ready(display);
    }
}

static IRAM_ATTR bool lcd_draw_complete(void *user_data)
{
    finish_display_flush_submission();
    lv_display_flush_ready(static_cast<lv_display_t *>(user_data));
    return false;
}

static void read_touch(lv_indev_t *, lv_indev_data_t *data)
{
    static TouchPoint last_point;
    static int64_t last_press_us = 0;
    static bool was_pressed = false;
    const int64_t now = esp_timer_get_time();
    // CST816S may NACK while asleep: read only after its touch interrupt.
    if(s_touch != nullptr && (!s_touch->isInterruptEnabled() || s_touch_pending.exchange(false))) {
        TouchPoint point;
        if(s_touch->readPoints(&point, 1, 0) > 0) {
            last_point = point;
            last_press_us = now;
            ESP_LOGI(TAG, "Touch at x=%d y=%d", point.x, point.y);
        } else {
            last_press_us = 0;
        }
    }
    data->point.x = last_point.x;
    data->point.y = last_point.y;
    const bool pressed = last_press_us && now - last_press_us < 120000;
    if(pressed && !was_pressed) s_touch_detected_us = now;
    was_pressed = pressed;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void network_control_task(void *arg)
{
    const auto command = reinterpret_cast<uintptr_t>(arg);
    if(command == 1) wifi_cfg_start_provisioning();
    else if(command == 2) wifi_cfg_stop_provisioning();
    else if(command == 4) (void)wifi_cfg_disconnect();
    else if(command == 5) (void)wifi_cfg_connect(nullptr);
    else if(command == 3) {
        (void)wifi_cfg_disconnect();
        for(int i = 0; i < 20 && wifi_cfg_is_connected(); ++i) vTaskDelay(pdMS_TO_TICKS(50));
        wifi_cfg_stop_provisioning();
        wifi_network_t networks[16];
        size_t count = 0;
        if(wifi_cfg_list_networks(networks, 16, &count) == ESP_OK) {
            for(size_t i = 0; i < count; ++i) (void)wifi_cfg_remove_network(networks[i].ssid);
        }
        wifi_cfg_start_provisioning();
    }
    s_network_running = false;
    s_connection_widgets_dirty = true;
    vTaskDelete(nullptr);
}

static void service_connection_state()
{
    static int64_t next_status_us = 0;
    const int64_t now = esp_timer_get_time();
    if(now >= next_status_us) {
        next_status_us = now + 1000000;
        const bool connected = wifi_cfg_is_connected();
        if(connected != s_wifi_connected) {
            s_wifi_connected = connected;
            if(connected) s_pairing_requested = true;
            else s_pairing_cancelled = true;
            s_connection_widgets_dirty = true;
        }
        wifi_status_t status{};
        if(wifi_cfg_get_status(&status) == ESP_OK) {
            if(std::strcmp(s_network_name, connected ? status.ssid : "") ||
               std::strcmp(s_network_ip, connected ? status.ip : "")) s_connection_widgets_dirty = true;
            std::snprintf(s_network_name, sizeof(s_network_name), "%s", connected ? status.ssid : "");
            std::snprintf(s_network_ip, sizeof(s_network_ip), "%s", connected ? status.ip : "");
        }
    }
    if(s_account_ready.exchange(false)) {
        const bool show_initial_welcome = s_initial_account_sync_pending && s_device_token[0];
        s_initial_account_sync_pending = false;
        if(show_initial_welcome) {
            s_app.state.screen = HAVIT_SCREEN_SIGN_IN;
            (void)havit_app_dispatch(&s_app, HAVIT_EVENT_LOGIN_STARTED, nullptr);
            (void)havit_app_dispatch(&s_app, HAVIT_EVENT_LOGIN_SUCCEEDED, s_display_name[0] ? s_display_name : "HAVIT");
            s_pairing_view = false;
            s_resume_pairing = false;
            s_wifi_setup_view = false;
            s_pairing_requested = false;
            s_ui_dirty = true;
        } else if(s_app.state.screen == HAVIT_SCREEN_HOME && s_app_view == AppView::Today) {
            const uint32_t count = s_synced_task_count + s_local_tasks.count;
            char count_text[24];
            std::snprintf(count_text, sizeof(count_text), "%lu tarea%s",
                          static_cast<unsigned long>(count), count == 1 ? "" : "s");
            set_generated_label(lv_screen_active(), "0 tareas", count_text);
            set_generated_label(lv_screen_active(), "1 tarea", count_text);
        }
    }
    if(s_pairing_requested && s_wifi_connected && !s_pairing_running) {
        s_pairing_requested = false;
        s_pairing_cancelled = false;
        s_pairing_running = true;
        if(!s_wifi_setup_view && !s_pairing_view) {
            s_pairing_view = true;
            s_ui_dirty = true;
        }
        s_pairing_widgets_dirty = true;
        set_pairing_message(s_device_token[0] ? "Sincronizando tu cuenta…" : "Creando tu PIN…");
        if(xTaskCreate(start_pairing_task, "havit_pair", 8192, nullptr, 4, nullptr) != pdPASS) {
            s_pairing_running = false;
            s_pairing_widgets_dirty = true;
            set_pairing_message("No hay memoria para solicitar el PIN. Reintentar.");
        }
    }
    if(!s_network_running) {
        const int command = s_network_command.exchange(0);
        if(command) {
            s_network_running = true;
            if(xTaskCreate(network_control_task, "havit_network", 6144,
                           reinterpret_cast<void *>(static_cast<uintptr_t>(command)), 4, nullptr) != pdPASS) {
                s_network_running = false;
                set_pairing_message("No se pudo cambiar la conexión. Reintentar.");
            }
        }
    }
    if(s_ui_dirty.exchange(false)) render_async(nullptr);
    refresh_connection_widgets();
}

static void lvgl_task(void *)
{
    while(true) {
        service_connection_state();
        uint32_t wait_ms = lv_timer_handler();
        report_display_completion();
        if(wait_ms == LV_NO_TIMER_READY || wait_ms > 10) wait_ms = 10;
        vTaskDelay(pdMS_TO_TICKS(wait_ms));
    }
}

static void init_ui_assets(void)
{
    init_image(&s_ring1, ring1_data, 340, 340, ring1_end - ring1_data);
    init_image(&s_ring2, ring2_data, 340, 340, ring2_end - ring2_data);
    init_image(&s_ring3, ring3_data, 340, 340, ring3_end - ring3_data);
    init_image(&s_google, google_data, 47, 48, google_end - google_data);
    init_image(&s_separator, separator_data, 30, 84, separator_end - separator_data);
    init_image(&s_pill, pill_data, 180, 51, pill_end - pill_data);
    init_image(&s_arrow, arrow_data, 29, 17, arrow_end - arrow_data);
    init_image(&s_arrow_welcome, arrow_welcome_data, 29, 17,
               arrow_welcome_end - arrow_welcome_data);

    s_font_15 = LV_FONT_DEFAULT;
    s_font_20 = &lv_font_montserrat_20;
    s_font_22 = &lv_font_montserrat_20;
    s_font_24 = &lv_font_montserrat_24;
}

} // namespace

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting Havit OS on Waveshare ESP32-S3-Touch-LCD-1.85, 360x360");

    esp_err_t nvs_result = nvs_flash_init();
    if(nvs_result == ESP_ERR_NVS_NO_FREE_PAGES || nvs_result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_result);

    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("havit", NVS_READWRITE, &nvs_handle));
    s_havit_store_handle = nvs_handle;
    (void)nvs_get_u8(nvs_handle, "notice_sound", &s_notice_sound);
    (void)nvs_get_u8(nvs_handle, "notice_light", &s_notice_light);
    (void)nvs_get_u8(nvs_handle, "notice_tone", &s_notice_tone);
    load_local_tasks();
    size_t stored_token_size = sizeof(s_device_token);
    (void)nvs_get_str(nvs_handle, "device_token", s_device_token, &stored_token_size);
    start_wifi_services(nvs_handle);
    const havit_store_t store = havit_store_nvs_create(nvs_handle);
    ESP_ERROR_CHECK(havit_app_init(&s_app, store) == HAVIT_APP_OK ? ESP_OK : ESP_FAIL);

    static Board board;
    ESP_ERROR_CHECK(board.init() ? ESP_OK : ESP_FAIL);
    const bool board_ready = board.begin();
    s_lcd = board.getLCD();
    s_touch = board_ready ? board.getTouch() : nullptr;
    if(!board_ready) {
        ESP_LOGE(TAG, "Touch initialization failed; continuing in display-only mode");
    }
    if(s_touch != nullptr && s_touch->isInterruptEnabled()) {
        s_touch->attachInterruptCallback([](void *) -> bool { s_touch_pending = true; return false; });
    }
    ESP_ERROR_CHECK(s_lcd != nullptr ? ESP_OK : ESP_FAIL);

    lv_init();
    lv_display_t *display = lv_display_create(s_lcd->getFrameWidth(), s_lcd->getFrameHeight());
    ESP_ERROR_CHECK(display != nullptr ? ESP_OK : ESP_FAIL);

    /* QSPI max transfer size is limited, so render in DMA-capable internal-RAM strips. */
    /* BLE provisioning uses internal RAM; keep the DMA strips small enough for it. */
    // Keep two strips small enough for internal DMA RAM alongside Wi-Fi and BLE.
    // The asynchronous QSPI transfer overlaps rendering without requiring large buffers.
    constexpr uint32_t BUFFER_LINES = 16;
    const uint32_t buffer_bytes = s_lcd->getFrameWidth() * BUFFER_LINES * sizeof(lv_color_t);
    void *buffer_1 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    void *buffer_2 = heap_caps_malloc(buffer_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_ERROR_CHECK(buffer_1 != nullptr && buffer_2 != nullptr ? ESP_OK : ESP_ERR_NO_MEM);

    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, buffer_1, buffer_2, buffer_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(display, s_lcd);
    lv_display_set_flush_cb(display, flush_display);
    lv_display_add_event_cb(display, display_render_event, LV_EVENT_RENDER_START, nullptr);
    lv_display_add_event_cb(display, display_render_event, LV_EVENT_RENDER_READY, nullptr);
    ESP_ERROR_CHECK(s_lcd->attachDrawBitmapFinishCallback(lcd_draw_complete, display) ? ESP_OK : ESP_FAIL);

    lv_indev_t *pointer = lv_indev_create();
    lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(pointer, read_touch);

    const esp_timer_create_args_t tick_args = {
        .callback = [](void *) { lv_tick_inc(2); },
        .name = "havit_lvgl_tick"
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 2000));

    lv_fs_memfs_init();
    ui_init_gen("");
    havit_ui_fonts_init();
    init_ui_assets();
    // Events accumulated while the display was initializing are reflected in the state
    // consumed below; don't immediately rebuild the just-created screen for those events.
    s_ui_dirty.exchange(false);
    finish_render_and_load(lv_screen_active(), false);
    ESP_LOGI(TAG, "Havit screens ready; onboarding_complete=%s",
             s_app.state.onboarding_complete ? "true" : "false");
    ESP_ERROR_CHECK(xTaskCreate(lvgl_task, "havit_lvgl", 8192, nullptr, 5, nullptr) == pdPASS
                        ? ESP_OK : ESP_ERR_NO_MEM);
    s_lvgl_ready = true;
    if(s_wifi_connected) s_pairing_requested = true;
}
