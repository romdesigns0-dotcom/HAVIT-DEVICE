#include "havit_ui.h"
#include "havit_default_background.h"
#include "havit_team_assets.h"

#include "esp_log.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Figma 1X1, Teams flow: 464:201 -> 464:483 -> 464:493 ->
 * 464:1211 -> 464:617. The first frame is a 360x1091 scroll view. */
extern const lv_font_t havit_instrument_sans_5;
extern const lv_font_t havit_instrument_sans_7;
extern const lv_font_t havit_instrument_sans_10;
extern const lv_font_t havit_instrument_sans_11;
extern const lv_font_t havit_instrument_sans_13;
extern const lv_font_t havit_instrument_sans_14;
extern const lv_font_t havit_instrument_sans_15;
extern const lv_font_t havit_instrument_sans_8;
extern const lv_font_t havit_instrument_sans_12;
extern const lv_font_t havit_instrument_sans_16;
extern const lv_font_t havit_instrument_sans_20;
extern const lv_font_t havit_instrument_sans_22;
extern const lv_font_t havit_instrument_sans_24;

#define WHITE 0xFFFFFF
#define BLACK 0x000000
#define TEAL 0x73979A
#define BLUE 0x77B7DB
#define MINT 0xD0E7BD
#define GREEN 0xA9D8B5
#define PURPLE 0x9594C8

static const char *TAG = "havit_team";
static lv_obj_t *s_create_scroll;
static lv_draw_buf_t *s_form_cache;
static lv_image_dsc_t s_form_cache_image;
static lv_obj_t *s_spinner;
static lv_timer_t *s_generate_timer;

static const lv_font_t *font_at(int size)
{
    switch(size) {
        case 5: return &havit_instrument_sans_5;
        case 7: return &havit_instrument_sans_7;
        case 10: return &havit_instrument_sans_10;
        case 11: return &havit_instrument_sans_11;
        case 13: return &havit_instrument_sans_13;
        case 14: return &havit_instrument_sans_14;
        case 15: return &havit_instrument_sans_15;
        case 8: return &havit_instrument_sans_8;
        case 12: return &havit_instrument_sans_12;
        case 16: return &havit_instrument_sans_16;
        case 20: return &havit_instrument_sans_20;
        case 22: return &havit_instrument_sans_22;
        default: return &havit_instrument_sans_24;
    }
}

static lv_obj_t *rect(lv_obj_t *parent, int x, int y, int w, int h,
                      uint32_t color, int radius)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

static lv_obj_t *image(lv_obj_t *parent, const lv_image_dsc_t *asset, int x, int y)
{
    lv_obj_t *obj = lv_image_create(parent);
    lv_image_set_src(obj, asset);
    lv_obj_set_pos(obj, x, y);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

static lv_obj_t *outline(lv_obj_t *parent, int x, int y, int w, int h,
                         uint32_t color, int radius, int border)
{
    lv_obj_t *obj = rect(parent, x, y, w, h, BLACK, radius);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, border, 0);
    return obj;
}

static lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y,
                       int w, int h, int size, uint32_t color,
                       lv_text_align_t align)
{
    lv_obj_t *obj = lv_label_create(parent);
    lv_obj_remove_style_all(obj);
    lv_label_set_text(obj, text);
    /* Match Figma line boxes without clipping ascenders or accents. In this
     * flow multiline text has explicit newlines; never wrap a single line. */
    const lv_font_t *font = font_at(size);
    lv_point_t text_size;
    lv_text_get_size(&text_size, text, font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    if(text_size.x > w)
        ESP_LOGW(TAG, "Text exceeds Figma slot: %s (%ld > %d px)", text, (long)text_size.x, w);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(obj, w);
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    const int lines = text[0] && strchr(text, '\n') ? 2 : 1;
    const int line_height = h / lines;
    lv_obj_set_style_text_line_space(obj, line_height - font->line_height, 0);
    lv_obj_set_pos(obj, x, y + (line_height - font->line_height) / 2);
    lv_obj_set_style_text_font(obj, font_at(size), 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(obj, align, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

static void build_member(lv_obj_t *content, int y, const lv_image_dsc_t *avatar,
                         const char *email, bool initial)
{
    image(content, avatar, 47, y + 2);
    if(initial) label(content, "R", 55, y + 4, 20, 28, 24, BLACK, LV_TEXT_ALIGN_LEFT);
    rect(content, 88, y, 219, 36, TEAL, 7);
    lv_obj_t *email_label = label(content, email, 98, y + 4, 200, 28, 16, WHITE, LV_TEXT_ALIGN_LEFT);
    lv_obj_set_style_text_opa(email_label, 135, 0);
}

static void build_text_field(lv_obj_t *content, int y, const char *title,
                             int title_width)
{
    label(content, title, 57, y, title_width, 28, 16, WHITE, LV_TEXT_ALIGN_LEFT);
    label(content, "NOMBRE", 57, y + 24, 192, 28, 16, WHITE, LV_TEXT_ALIGN_LEFT);
    image(content, &havit_team_imgVector5, 57, y + 48);
    image(content, &havit_team_imgBitcoinIconsEditFilled, 257, y + 4);
    image(content, &havit_team_imgIcBaselineKeyboardVoice, 283, y + 4);
}

static void build_date(lv_obj_t *content, int y, int clock_y, int separator_y, const char *date)
{
    image(content, &havit_team_imgAntDesignCalendarFilled, 83, y + 6);
    rect(content, 110, y, 167, 35, TEAL, 10);
    label(content, date, 121, y + 4, 156, 27, 22, WHITE, LV_TEXT_ALIGN_LEFT);
    rect(content, 110, y + 39, 75, 35, TEAL, 10);
    rect(content, 202, y + 39, 75, 35, TEAL, 10);
    label(content, "20", 110, y + 43, 75, 28, 22, WHITE, LV_TEXT_ALIGN_CENTER);
    label(content, "20", 202, y + 43, 75, 28, 22, WHITE, LV_TEXT_ALIGN_CENTER);
    image(content, &havit_team_imgGroup58, 191, separator_y);
    image(content, &havit_team_imgGroup56, 85, clock_y);
}

static lv_obj_t *build_create_content(lv_obj_t *scroll)
{
    lv_obj_t *content = lv_obj_create(scroll);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, 360, 1091);
    lv_obj_set_pos(content, 0, 0);
    lv_obj_set_style_bg_color(content, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(content, LV_OBJ_FLAG_EVENT_BUBBLE);

    label(content, "Crear Equipos", 102, 32, 180, 29, 24, WHITE, LV_TEXT_ALIGN_LEFT);
    image(content, &havit_team_imgEllipse28, 164, 71);
    image(content, &havit_team_imgHugeiconsCancel01, 166, 73);
    label(content, "Miembros del equipo", 99, 118, 194, 28, 16, WHITE,
          LV_TEXT_ALIGN_LEFT);
    build_member(content, 153, &havit_team_imgEllipse57, "rom@gmail.com (you)", true);
    build_member(content, 196, &havit_team_imgEllipse56, "email", false);
    image(content, &havit_team_imgGroup23, 169, 242);

    build_text_field(content, 274, "Nombre de la tarea:", 190);
    build_text_field(content, 335, "Materia:", 180);
    build_text_field(content, 396, "Descripción del proyecto:", 210);
    build_text_field(content, 457, "Descripción del proyecto:", 210);

    image(content, &havit_team_imgVector8, 27, 586);
    rect(content, 28, 539, 102, 36, TEAL, 7);
    label(content, "@", 38, 543, 82, 28, 16, WHITE, LV_TEXT_ALIGN_LEFT);
    image(content, &havit_team_imgBitcoinIconsEditFilled, 279, 550);
    rect(content, 28, 603, 102, 36, TEAL, 7);
    label(content, "@", 38, 607, 82, 28, 16, WHITE, LV_TEXT_ALIGN_LEFT);
    image(content, &havit_team_imgBitcoinIconsEditFilled, 275, 614);
    image(content, &havit_team_imgIcBaselineKeyboardVoice, 303, 551);
    image(content, &havit_team_imgIcBaselineKeyboardVoice, 299, 615);
    image(content, &havit_team_imgVector9, 42, 650);

    label(content, "Crear fechas límite a partir del:", 69, 678, 270, 28,
          16, WHITE, LV_TEXT_ALIGN_LEFT);
    label(content, "Inicio", 160, 712, 70, 28, 16, WHITE, LV_TEXT_ALIGN_LEFT);
    build_date(content, 749, 798, 790, "22 Junio 2026");
    label(content, "Entrega final", 132, 838, 96, 28, 16, WHITE,
          LV_TEXT_ALIGN_CENTER);
    build_date(content, 875, 922, 920, "22 Junio 2026");

    rect(content, 57, 977, 246, 76, TEAL, 38);
    label(content, "Generar fechas de entrega", 70, 1001, 220, 28, 16, WHITE,
          LV_TEXT_ALIGN_CENTER);
    return content;
}

static void arc_rotation(void *object, int32_t value)
{
    lv_image_set_rotation((lv_obj_t *)object, value * 10);
}

static void build_generating(lv_obj_t *view)
{
    label(view, "Generando línea de tiempo para\nequipo...", 40, 143, 279, 56,
          16, WHITE, LV_TEXT_ALIGN_CENTER);
    s_spinner = image(view, &havit_team_view1_imgGroup, 155, 215);
}

static void build_timeline(lv_obj_t *view)
{
    label(view, "Línea de tiempo\nFinal Research Paper", 95, 35, 171, 30,
          13, WHITE, LV_TEXT_ALIGN_CENTER);
    label(view, "Seminario de Investigación de Diseño", 55, 62, 251, 20,
          11, WHITE, LV_TEXT_ALIGN_CENTER);
    lv_obj_t *month = label(view, "Octubre", 133, 91, 82, 28, 20, WHITE, LV_TEXT_ALIGN_LEFT);
    lv_obj_set_style_text_opa(month, 217, 0);
    lv_obj_t *year = label(view, "2026", 215, 101, 40, 14, 12, WHITE, LV_TEXT_ALIGN_LEFT);
    lv_obj_set_style_text_opa(year, 128, 0);
    image(view, &havit_team_view2_imgIcon, 303, 187);
    static const char *week[] = {"L", "M", "X", "J", "V", "S", "D"};
    for(int col = 0; col < 7; col++) {
        lv_obj_t *day = label(view, week[col], 78 + (col * 2961 + 50) / 100, 128,
                             26, 14, 12, WHITE, LV_TEXT_ALIGN_CENTER);
        lv_obj_set_style_text_opa(day, 128, 0);
    }
    for(int day = 1; day <= 31; day++) {
        int index = day + 2;
        int col = index % 7;
        int row = index / 7;
        bool blue = day == 1 || day == 6 || day == 11 || day == 16 || day >= 19;
        uint32_t color = day == 18 ? WHITE : (blue ? 0x74B2CD : PURPLE);
        char value[4];
        lv_snprintf(value, sizeof(value), "%d", day);
        label(view, value, 78 + (col * 2961 + 50) / 100,
              150 + (row * 2913 + 50) / 100, 26, 24, 16, color, LV_TEXT_ALIGN_CENTER);
    }
    image(view, &havit_team_view2_imgEllipse57, 230, 229);
    image(view, &havit_team_view2_imgEllipse57, 176, 258);
    image(view, &havit_team_view2_imgEllipse58, 240, 229);
    image(view, &havit_team_view2_imgEllipse58, 144, 258);
    image(view, &havit_team_view2_imgGroup20, 163, 298);
}

static void build_breakdown(lv_obj_t *view)
{
    label(view, "Desglose", 138, 37, 120, 24, 20, WHITE, LV_TEXT_ALIGN_LEFT);
    image(view, &havit_team_view3_imgAkarIconsCircleCheckFill, 67, 56);
    image(view, &havit_team_view3_imgBitcoinIconsEditFilled, 254, 61);
    outline(view, 46, 101, 268, 59, BLUE, 38, 2);
    outline(view, 46, 170, 268, 56, BLUE, 64, 2);
    outline(view, 67, 239, 228, 48, BLUE, 38, 2);
    outline(view, 112, 294, 138, 29, BLUE, 38, 2);
    image(view, &havit_team_view3_imgEllipse63, 57, 113);
    image(view, &havit_team_view3_imgEllipse64, 57, 182);
    image(view, &havit_team_view3_imgEllipse65, 75, 248);
    image(view, &havit_team_view3_imgEllipse66, 117, 300);
    label(view, "18 Oct - 23:59", 104, 104, 153, 28, 16, WHITE, LV_TEXT_ALIGN_LEFT);
    label(view, "Entregar 5 fuentes académica.....", 100, 127, 192, 28, 11, WHITE, LV_TEXT_ALIGN_LEFT);
    label(view, "23 Oct - 23:59", 103, 176, 153, 28, 15, WHITE, LV_TEXT_ALIGN_LEFT);
    label(view, "Lista final de 15 fuentes y casos de ....", 103, 204, 192, 11, 10, WHITE, LV_TEXT_ALIGN_LEFT);
    label(view, "31 Oct - 23:59", 112, 245, 145, 28, 14, WHITE, LV_TEXT_ALIGN_LEFT);
    label(view, "Lectura y anotación de todas las fuente.......", 112, 260, 175, 28, 7, WHITE, LV_TEXT_ALIGN_LEFT);
    label(view, "03 Nov - 23:59", 142, 291, 85, 28, 10, WHITE, LV_TEXT_ALIGN_LEFT);
    label(view, "Esquema de investigación", 143, 299, 86, 28, 5, WHITE, LV_TEXT_ALIGN_LEFT);
    image(view, &havit_team_view3_imgGroup, 267, 115);
    image(view, &havit_team_view3_imgGroup, 269, 182);
    image(view, &havit_team_view3_imgGroup1, 255, 249);
    image(view, &havit_team_view3_imgGroup2, 228, 301);
    rect(view, 319, 137, 13, 39, BLUE, 39);
}

static void build_send(lv_obj_t *view)
{
    rect(view, 57, 142, 246, 76, TEAL, 63);
    label(view, "Enviar a Equipo", 79, 166, 153, 28, 20, WHITE, LV_TEXT_ALIGN_LEFT);
    image(view, &havit_team_view4_imgGroup45, 232, 151);
    image(view, &havit_team_view4_imgVector, 248, 164);
}

static void generation_ready(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    s_generate_timer = NULL;
    if(s_spinner != NULL) lv_anim_delete(s_spinner, NULL);
    ESP_LOGI(TAG, "Timeline generation prototype complete");
    havit_ui_navigate_to(HAVIT_UI_SCREEN_TEAM_TIMELINE);
}

static void create_form_event(lv_event_t *event)
{
    const lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_PRESSED) {
        ESP_LOGI(TAG, "Team form received pointer; remaining scroll=%ld px",
                 (long)lv_obj_get_scroll_bottom(s_create_scroll));
    }
    else if(code == LV_EVENT_SCROLL_BEGIN || code == LV_EVENT_SCROLL_END) {
        ESP_LOGI(TAG, "Team form scroll %s: y=%ld",
                 code == LV_EVENT_SCROLL_BEGIN ? "begin" : "end",
                 (long)lv_obj_get_scroll_y(s_create_scroll));
    }
    else if(code == LV_EVENT_CLICKED) {
        /* CLICKED is emitted on release only when LVGL did not recognize a
         * scroll. Starting a swipe over X or Generate must not navigate. */
        lv_indev_t *indev = lv_event_get_indev(event);
        if(indev == NULL) return;
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        const int32_t content_y = point.y + lv_obj_get_scroll_y(s_create_scroll);
        if(point.x >= 157 && point.x <= 203 && content_y >= 65 && content_y <= 109)
            havit_ui_navigate_to(HAVIT_UI_SCREEN_ADD);
        else if(point.x >= 47 && point.x <= 313 && content_y >= 968 && content_y <= 1062)
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TEAM_GENERATING);
    }
}

void havit_ui_build_team_views(lv_obj_t *create, lv_obj_t *generating,
                               lv_obj_t *timeline, lv_obj_t *breakdown,
                               lv_obj_t *send)
{
    if(create == NULL || generating == NULL || timeline == NULL ||
       breakdown == NULL || send == NULL) return;

    s_create_scroll = create;
    /* SCROLLABLE alone is not an input target: lv_obj_hit_test requires
     * CLICKABLE. Decorative children let the pointer reach this viewport. */
    lv_obj_add_flag(create, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE |
                            LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_scroll_dir(create, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(create, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(create, 0, 0);
    lv_obj_set_style_clip_corner(create, true, 0);
    lv_obj_add_event_cb(create, create_form_event, LV_EVENT_ALL, NULL);
    lv_obj_t *form_objects = build_create_content(create);
    /* Validate the real viewport with LVGL's own hit test before the screen
     * is loaded. Restore its visibility without drawing this startup check. */
    const bool was_hidden = lv_obj_has_flag(create, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(create, LV_OBJ_FLAG_HIDDEN);
    lv_obj_update_layout(create);
    /* Prepare the complete form once. Keep its label/image objects intact as
     * the editable source; scroll only the opaque RGB565 cache in this static
     * prototype. Rebuild the snapshot whenever form data changes. */
    s_form_cache = lv_snapshot_take(form_objects, LV_COLOR_FORMAT_RGB565);
    if(s_form_cache != NULL) {
        /* The top-centre of the Generate button is below the initial viewport.
         * A black sample here would reveal an incorrectly clipped snapshot. */
        const uint16_t *bottom_row = (const uint16_t *)(
            s_form_cache->data + 980 * s_form_cache->header.stride);
        if(bottom_row[180] == 0) {
            ESP_LOGE(TAG, "Form cache was clipped below the viewport; using live objects");
            lv_draw_buf_destroy(s_form_cache);
            s_form_cache = NULL;
        }
    }
    if(s_form_cache != NULL) {
        lv_draw_buf_to_image(s_form_cache, &s_form_cache_image);
        image(create, &s_form_cache_image, 0, 0);
        lv_obj_add_flag(form_objects, LV_OBJ_FLAG_HIDDEN);
        ESP_LOGI(TAG, "Prepared form cache: %ux%u, %lu bytes",
                 s_form_cache->header.w, s_form_cache->header.h,
                 (unsigned long)s_form_cache->data_size);
    }
    else ESP_LOGW(TAG, "Form cache unavailable; retaining native object rendering");
    lv_obj_update_layout(create);
    lv_point_t probe_point = {180, 180};
    const bool receives_pointer = lv_indev_search_obj(create, &probe_point) == create;
    const int32_t scroll_range = lv_obj_get_scroll_bottom(create);
    if(receives_pointer && scroll_range > 0)
        ESP_LOGI(TAG, "Form input check OK: viewport captures pointer, vertical range=%ld px",
                 (long)scroll_range);
    else
        ESP_LOGE(TAG, "Form input check FAILED: captures=%d, vertical range=%ld px",
                 receives_pointer, (long)scroll_range);
    if(was_hidden) lv_obj_add_flag(create, LV_OBJ_FLAG_HIDDEN);
    build_generating(generating);
    build_timeline(timeline);
    build_breakdown(breakdown);
    build_send(send);
    ESP_LOGI(TAG, "Team workflow screens created from Figma 1X1; form height 1091 px");
}

void havit_ui_team_screen_changed(havit_ui_screen_t screen)
{
    if(screen != HAVIT_UI_SCREEN_TEAM_GENERATING) {
        if(s_generate_timer != NULL) {
            lv_timer_delete(s_generate_timer);
            s_generate_timer = NULL;
        }
        if(s_spinner != NULL) lv_anim_delete(s_spinner, NULL);
    }
    if(screen == HAVIT_UI_SCREEN_TEAM_CREATE && s_create_scroll != NULL)
        lv_obj_scroll_to_y(s_create_scroll, 0, LV_ANIM_OFF);
    if(screen == HAVIT_UI_SCREEN_TEAM_GENERATING) {
        if(s_spinner != NULL) {
            lv_anim_t spin;
            lv_anim_init(&spin);
            lv_anim_set_var(&spin, s_spinner);
            lv_anim_set_exec_cb(&spin, arc_rotation);
            lv_anim_set_values(&spin, 0, 360);
            lv_anim_set_duration(&spin, 950);
            lv_anim_set_repeat_count(&spin, LV_ANIM_REPEAT_INFINITE);
            lv_anim_start(&spin);
        }
        s_generate_timer = lv_timer_create(generation_ready, 1700, NULL);
        if(s_generate_timer != NULL) lv_timer_set_repeat_count(s_generate_timer, 1);
        else ESP_LOGE(TAG, "Could not start timeline generation timer");
    }
}

bool havit_ui_team_handle_touch(havit_ui_screen_t screen, int32_t x, int32_t y)
{
    if(screen == HAVIT_UI_SCREEN_TEAM_CREATE) {
        /* Native LVGL CLICKED handles the form after release; raw touch-down
         * routing must leave the complete drag gesture in this view. */
        return true;
    }
    else if(screen == HAVIT_UI_SCREEN_TEAM_TIMELINE &&
            x >= 148 && x <= 212 && y >= 292 && y <= 346) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_TEAM_BREAKDOWN);
        return true;
    }
    else if(screen == HAVIT_UI_SCREEN_TEAM_BREAKDOWN &&
            x >= 61 && x <= 112 && y >= 50 && y <= 105) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_TEAM_SEND);
        return true;
    }
    else if(screen == HAVIT_UI_SCREEN_TEAM_SEND &&
            x >= 50 && x <= 310 && y >= 135 && y <= 225) {
        havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY);
        return true;
    }
    return false;
}
