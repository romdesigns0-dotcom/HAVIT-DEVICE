#include "havit_ui.h"
#include "havit_quick_task_assets.h"

#include "esp_log.h"
#include "lvgl.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "widgets/keyboard/lv_keyboard.h"
#include "widgets/line/lv_line.h"
#include "widgets/roller/lv_roller.h"
#include "widgets/textarea/lv_textarea.h"

extern const lv_font_t havit_instrument_sans_12;
extern const lv_font_t havit_instrument_sans_14;
extern const lv_font_t havit_instrument_sans_15;
extern const lv_font_t havit_instrument_sans_16;
extern const lv_font_t havit_instrument_sans_22;
extern const lv_font_t havit_instrument_sans_20;
extern const lv_font_t havit_instrument_sans_24;
extern const lv_font_t havit_instrument_sans_28;
extern const lv_font_t havit_instrument_sans_32;
extern const lv_font_t havit_instrument_sans_40;
extern const lv_font_t havit_instrument_sans_48;

#define QUICK_GREEN 0xD0E6BD
#define QUICK_TEAL  0x73979A
#define QUICK_BLUE  0x77B7DB
#define QUICK_WHITE 0xFFFFFF

static const char *TAG = "havit_quick_task";
static lv_obj_t *s_quick_view;
static lv_obj_t *s_timer_label;
static lv_obj_t *s_running_title;
static lv_obj_t *s_task_name;
static lv_obj_t *s_keyboard;
static lv_obj_t *s_minutes_roller;
static lv_obj_t *s_seconds_roller;
static lv_timer_t *s_countdown;
static uint32_t s_remaining_seconds;

static const lv_font_t *font_for_size(uint8_t size)
{
    switch(size) {
        case 12: return &havit_instrument_sans_12;
        case 14: return &havit_instrument_sans_14;
        case 15: return &havit_instrument_sans_15;
        case 16: return &havit_instrument_sans_16;
        case 20: return &havit_instrument_sans_20;
        case 22: return &havit_instrument_sans_22;
        case 24: return &havit_instrument_sans_24;
        case 28: return &havit_instrument_sans_28;
        case 32: return &havit_instrument_sans_32;
        case 48: return &havit_instrument_sans_48;
        default: return &havit_instrument_sans_40;
    }
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text,
                            int16_t x, int16_t y, int16_t width,
                            uint8_t size, lv_color_t color,
                            lv_text_align_t align)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_remove_style_all(label);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, width);
    lv_obj_set_height(label, LV_SIZE_CONTENT);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label, font_for_size(size), 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_align(label, align, 0);
    lv_obj_set_style_text_line_space(label, 0, 0);
    lv_obj_set_pos(label, x, y);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return label;
}

static lv_obj_t *make_image(lv_obj_t *parent, const lv_image_dsc_t *source,
                            int16_t x, int16_t y)
{
    lv_obj_t *image = lv_image_create(parent);
    lv_image_set_src(image, source);
    lv_obj_set_pos(image, x, y);
    lv_obj_clear_flag(image, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return image;
}

static lv_obj_t *make_circle(lv_obj_t *parent, int16_t x, int16_t y,
                             int16_t size, lv_color_t fill)
{
    lv_obj_t *circle = lv_obj_create(parent);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, size, size);
    lv_obj_set_pos(circle, x, y);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, fill, 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return circle;
}

static lv_obj_t *make_textarea(lv_obj_t *parent, int16_t x, int16_t y,
                               int16_t width, int16_t height, uint16_t max_length)
{
    lv_obj_t *textarea = lv_textarea_create(parent);
    lv_obj_remove_style_all(textarea);
    lv_obj_set_size(textarea, width, height);
    lv_obj_set_pos(textarea, x, y);
    lv_obj_set_style_bg_opa(textarea, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(textarea, 0, 0);
    lv_obj_set_style_pad_all(textarea, 0, 0);
    lv_obj_set_style_text_font(textarea, &havit_instrument_sans_16, 0);
    lv_obj_set_style_text_color(textarea, lv_color_hex(QUICK_WHITE), 0);
    lv_textarea_set_one_line(textarea, true);
    lv_textarea_set_max_length(textarea, max_length);
    lv_textarea_set_placeholder_text(textarea, "NOMBRE");
    lv_obj_set_style_text_color(textarea, lv_color_hex(QUICK_WHITE), LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_set_style_text_opa(textarea, LV_OPA_COVER, LV_PART_TEXTAREA_PLACEHOLDER);
    lv_textarea_set_cursor_click_pos(textarea, true);
    lv_obj_clear_flag(textarea, LV_OBJ_FLAG_SCROLLABLE);
    return textarea;
}

static void keyboard_event(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(s_keyboard, NULL);
    }
}

static lv_obj_t *make_duration_roller(lv_obj_t *parent, int16_t x,
                                      const char *options, uint16_t selected,
                                      uint8_t font_size)
{
    lv_obj_t *roller = lv_roller_create(parent);
    lv_roller_set_options(roller, options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(roller, 1);
    lv_roller_set_selected(roller, selected, LV_ANIM_OFF);
    lv_obj_set_size(roller, 75, 35);
    lv_obj_set_pos(roller, x, 220);
    lv_obj_set_style_bg_color(roller, lv_color_hex(QUICK_TEAL), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(roller, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(roller, 0, LV_PART_MAIN);
    lv_obj_set_style_text_font(roller, font_for_size(font_size), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller, lv_color_hex(QUICK_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_align(roller, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller, LV_OPA_TRANSP, LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller, lv_color_hex(QUICK_WHITE), LV_PART_SELECTED);
    lv_obj_set_style_text_font(roller, font_for_size(font_size), LV_PART_SELECTED);
    lv_obj_set_style_text_align(roller, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED);
    lv_obj_set_style_border_width(roller, 0, LV_PART_SELECTED);
    lv_obj_set_style_pad_all(roller, 0, LV_PART_MAIN);
    return roller;
}

static void format_remaining_time(void)
{
    if(s_timer_label == NULL) return;
    uint32_t hours = s_remaining_seconds / 3600;
    uint32_t minutes = (s_remaining_seconds / 60) % 60;
    uint32_t seconds = s_remaining_seconds % 60;
    char text[16];
    lv_snprintf(text, sizeof(text), "%02lu:%02lu:%02lu",
                (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds);
    lv_label_set_text(s_timer_label, text);
}

static void cancel_countdown(void)
{
    if(s_countdown != NULL) {
        lv_timer_delete(s_countdown);
        s_countdown = NULL;
    }
}

static void countdown_tick(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if(s_remaining_seconds > 0) --s_remaining_seconds;
    format_remaining_time();
    if(s_remaining_seconds == 0) {
        cancel_countdown();
        havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_QUICK_DONE);
    }
}

static void build_quick_form(lv_obj_t *parent)
{
    lv_obj_t *title = make_label(parent, "Tarea Rápida", 70, 51, 220, 24,
                                 lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_CENTER);
    /* The shared label helper wraps by default; keep this heading on one line. */
    lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);
    make_image(parent, &havit_quick_task_close_background, 164, 90);
    make_image(parent, &havit_quick_task_close, 166, 92);
    make_label(parent, "Nombre de la tarea:", 55, 144, 220, 16,
               lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_LEFT);
    s_task_name = make_textarea(parent, 55, 168, 190, 24, 40);
    make_image(parent, &havit_quick_task_line, 55, 192);
    make_image(parent, &havit_quick_task_edit, 255, 148);
    make_image(parent, &havit_quick_task_voice, 281, 148);

    make_image(parent, &havit_quick_task_clock, 84, 228);
    static const char minute_options[] = "00\n05\n10\n15\n20\n25\n30\n45\n60";
    static const char second_options[] = "00\n05\n10\n15\n20\n25\n30\n35\n40\n45\n50\n55";
    s_minutes_roller = make_duration_roller(parent, 109, minute_options, 0, 22);
    s_seconds_roller = make_duration_roller(parent, 201, second_options, 0, 22);
    make_image(parent, &havit_quick_task_separator_dot, 190, 226);

    lv_obj_t *start = lv_obj_create(parent);
    lv_obj_remove_style_all(start);
    lv_obj_set_size(start, 129, 40);
    lv_obj_set_pos(start, 116, 282);
    lv_obj_set_style_radius(start, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(start, lv_color_hex(QUICK_TEAL), 0);
    lv_obj_set_style_bg_opa(start, LV_OPA_COVER, 0);
    lv_obj_clear_flag(start, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    make_label(parent, "EMPEZAR", 116, 293, 129, 16,
               lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_CENTER);

    s_keyboard = lv_keyboard_create(parent);
    lv_obj_set_size(s_keyboard, 360, 140);
    lv_obj_set_pos(s_keyboard, 0, 220);
    lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_font(s_keyboard, &havit_instrument_sans_16, LV_PART_ITEMS);
    lv_obj_add_event_cb(s_keyboard, keyboard_event, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(s_keyboard, keyboard_event, LV_EVENT_CANCEL, NULL);
}

static void build_running(lv_obj_t *parent)
{
    make_label(parent, "Tarea Rápida", 115, 47, 130, 14,
               lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_CENTER);
    s_running_title = make_label(parent, "Humanidades", 105, 112, 200, 24,
                                 lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_LEFT);
    s_timer_label = make_label(parent, "00:20:20", 71, 155, 217, 48,
                               lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_CENTER);

    make_image(parent, &havit_quick_task_timer_check, 102, 234);
    make_circle(parent, 197, 237, 59, lv_color_hex(QUICK_GREEN));
    make_image(parent, &havit_quick_task_timer_cancel, 212, 252);
}

static void build_done(lv_obj_t *parent)
{
    make_label(parent, "Humanidades", 77, 97, 206, 32,
               lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_LEFT);
    make_image(parent, &havit_quick_task_done_check, 149, 149);
    make_label(parent, "Completado", 90, 226, 180, 20,
               lv_color_hex(QUICK_WHITE), LV_TEXT_ALIGN_CENTER);
}

void havit_ui_build_quick_task_views(lv_obj_t *quick, lv_obj_t *running,
                                     lv_obj_t *done)
{
    s_quick_view = quick;
    build_quick_form(quick);
    build_running(running);
    build_done(done);
    ESP_LOGI(TAG, "Quick Task flow ready: Figma 464:314 -> 464:342 -> 464:357");
}

void havit_ui_quick_task_reset(void)
{
    if(s_minutes_roller != NULL)
        lv_roller_set_selected(s_minutes_roller, 0, LV_ANIM_OFF);
    if(s_seconds_roller != NULL)
        lv_roller_set_selected(s_seconds_roller, 0, LV_ANIM_OFF);
    ESP_LOGI(TAG, "Quick Task duration reset to 00:00");
}

bool havit_ui_quick_task_handle_touch(havit_ui_screen_t screen,
                                       int32_t x, int32_t y)
{
    if(screen == HAVIT_UI_SCREEN_TASK_QUICK) {
        if(x >= 162 && x <= 200 && y >= 89 && y <= 123) {
            if(s_keyboard != NULL && !lv_obj_has_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
                lv_keyboard_set_textarea(s_keyboard, NULL);
            }
            ESP_LOGI(TAG, "Quick Task close tapped; returning to Add");
            havit_ui_navigate_to(HAVIT_UI_SCREEN_ADD);
            return true;
        }
        if(s_keyboard != NULL && !lv_obj_has_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN))
            return true;
        if(x >= 46 && x <= 296 && y >= 165 && y <= 198) {
            lv_keyboard_set_textarea(s_keyboard, s_task_name);
            lv_obj_clear_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(s_keyboard);
            return true;
        }
        if(x >= 105 && x <= 256 && y >= 279 && y <= 329) {
            uint16_t minutes = 0, seconds = 0;
            char minute_text[8] = {0};
            char second_text[8] = {0};
            lv_roller_get_selected_str(s_minutes_roller, minute_text, sizeof(minute_text));
            lv_roller_get_selected_str(s_seconds_roller, second_text, sizeof(second_text));
            minutes = (uint16_t)atoi(minute_text);
            seconds = (uint16_t)atoi(second_text);
            s_remaining_seconds = (uint32_t)minutes * 60U + seconds;
            if(s_remaining_seconds == 0) {
                ESP_LOGW(TAG, "Choose a non-zero duration before starting Quick Task");
                return true;
            }

            const char *name = lv_textarea_get_text(s_task_name);
            if(name == NULL || name[0] == '\0') name = "Humanidades";
            /* Copy before changing the destination view and its label. */
            lv_label_set_text(s_running_title, name);
            format_remaining_time();
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_QUICK_RUNNING);
            cancel_countdown();
            s_countdown = lv_timer_create(countdown_tick, 1000, NULL);
            if(s_countdown == NULL) ESP_LOGE(TAG, "Could not start quick-task timer");
            ESP_LOGI(TAG, "Quick Task start tapped: %s, %lu seconds",
                     name, (unsigned long)s_remaining_seconds);
            return true;
        }
        return false;
    }
    if(screen == HAVIT_UI_SCREEN_TASK_QUICK_RUNNING) {
        if(x >= 94 && x <= 173 && y >= 226 && y <= 304) {
            cancel_countdown();
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_QUICK_DONE);
            return true;
        }
        if(x >= 190 && x <= 266 && y >= 226 && y <= 304) {
            cancel_countdown();
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_QUICK);
            return true;
        }
        return true;
    }
    if(screen == HAVIT_UI_SCREEN_TASK_QUICK_DONE) {
        /* The completed screen is a single exit target: any touch returns to Today. */
        havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY);
        return true;
    }
    return false;
}
