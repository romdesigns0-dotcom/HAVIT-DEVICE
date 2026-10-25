#include "havit_ui.h"
#include "havit_today_task_assets.h"
#include "esp_log.h"
#include <string.h>

extern const lv_font_t havit_instrument_sans_10;
extern const lv_font_t havit_instrument_sans_11;
extern const lv_font_t havit_instrument_sans_14;
extern const lv_font_t havit_instrument_sans_15;
extern const lv_font_t havit_instrument_sans_16;
extern const lv_font_t havit_instrument_sans_20;
extern const lv_font_t havit_instrument_sans_24;

static const char *TAG = "havit_today_tasks";
static lv_obj_t *s_views[3];
static lv_obj_t *s_checks[3];
static lv_obj_t *s_check_marks[3];
static lv_obj_t *s_started_label;
static bool s_selected[3];
static bool s_removed[3];
static bool s_started;
static const int s_row_y[3] = {101, 170, 239};
static const int s_row_h[3] = {59, 56, 48};

static lv_obj_t *image(lv_obj_t *parent, const lv_image_dsc_t *src,
                       int x, int y, int size)
{
    lv_obj_t *obj = lv_image_create(parent);
    lv_image_set_src(obj, src);
    lv_obj_set_pos(obj, x, y);
    if(size > 0) lv_obj_set_size(obj, size, size);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static lv_obj_t *text(lv_obj_t *parent, const char *value, int x, int y,
                      int width, const lv_font_t *font, bool centered,
                      lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, value);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_align(label, centered ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_line_space(label, 0, 0);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return label;
}

static lv_obj_t *outline(lv_obj_t *parent, int x, int y, int w, int h, int radius)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x77B7DB), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static void common_header(lv_obj_t *view, bool select)
{
    lv_color_t white = lv_color_white();
    text(view, "Tareas de Hoy", 115, 46, 129, &havit_instrument_sans_20, true, white);
    text(view, "10 de octubre", 136, 70, 88, &havit_instrument_sans_14, true, white);
    image(view, &havit_today_task_month, 108, 75, 16);
    lv_obj_t *month_next = image(view, &havit_today_task_month, 236, 77, 16);
    lv_image_set_rotation(month_next, 1800);
    image(view, select ? &havit_today_task_edit_select : &havit_today_task_pencil,
          266, 61, 33);
    image(view, select ? &havit_today_task_select_bg : &havit_today_task_close_bg,
          61, 61, 32);
    image(view, select ? &havit_today_task_select_close : &havit_today_task_close,
          63, 63, 28);
}

static void build_list(lv_obj_t *view, bool select)
{
    common_header(view, select);
    outline(view, 46, 101, 268, 59, 38);
    outline(view, 46, 170, 268, 56, 38);
    outline(view, 67, 239, 228, 48, 38);

    text(view, "9:00 am - Seminario", 66, 104, 190, &havit_instrument_sans_16, false, lv_color_white());
    text(view, "Entregar 5 fuentes académicas iniciales", 66, 127, 205, &havit_instrument_sans_11, false, lv_color_white());
    text(view, "10:30 am - Practicum", 62, 176, 190, &havit_instrument_sans_15, false, lv_color_white());
    text(view, "Lista final de 15 fuentes y casos de estudio\ncompletada.", 62, 198, 205, &havit_instrument_sans_10, false, lv_color_white());
    text(view, "3:00 pm - Interiores", 78, 250, 175, &havit_instrument_sans_14, false, lv_color_white());
    lv_obj_t *scroll = lv_obj_create(view);
    lv_obj_remove_style_all(scroll);
    lv_obj_set_pos(scroll, 319, 137); lv_obj_set_size(scroll, 13, 39);
    lv_obj_set_style_radius(scroll, 7, 0);
    lv_obj_set_style_bg_color(scroll, lv_color_hex(0x77B7DB), 0);
    lv_obj_set_style_bg_opa(scroll, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scroll, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    if(!select) {
        image(view, &havit_today_task_arrow, 271, 115, 31);
        image(view, &havit_today_task_arrow, 271, 182, 31);
        image(view, &havit_today_task_arrow_alt, 255, 249, 28);
    } else {
        const lv_image_dsc_t *boxes[] = {&havit_today_task_box1, &havit_today_task_box2,
                                         &havit_today_task_box3};
        const int xs[] = {274, 274, 257};
        const int ys[] = {118, 185, 251};
        for(int i = 0; i < 3; i++) {
            s_checks[i] = image(view, boxes[i], xs[i], ys[i], 0);
            s_check_marks[i] = lv_obj_create(view);
            lv_obj_remove_style_all(s_check_marks[i]);
            lv_obj_set_pos(s_check_marks[i], xs[i] + 4, ys[i] + 4);
            lv_obj_set_size(s_check_marks[i], 19, 19);
            lv_obj_set_style_radius(s_check_marks[i], 5, 0);
            lv_obj_set_style_bg_color(s_check_marks[i], lv_color_hex(0x77B7DB), 0);
            lv_obj_set_style_bg_opa(s_check_marks[i], LV_OPA_COVER, 0);
            lv_obj_add_flag(s_check_marks[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_t *mark = lv_label_create(s_check_marks[i]);
            lv_label_set_text(mark, "✓");
            lv_obj_set_style_text_color(mark, lv_color_white(), 0);
            lv_obj_set_style_text_font(mark, &havit_instrument_sans_14, 0);
            lv_obj_center(mark);
            lv_obj_clear_flag(s_check_marks[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        }
        /* The updated Figma has a pink trash action followed by a white share action. */
        image(view, &havit_today_task_share, 148, 307, 32);
        image(view, &havit_today_task_delete_bg, 194, 307, 32);
        image(view, &havit_today_task_delete, 202, 315, 16);
    }
    for(int i = 0; i < 3; i++) {
        if(s_removed[i]) {
            /* Hide row objects by their Figma row band. */
            for(uint32_t j = 0; j < lv_obj_get_child_count(view); j++) {
                lv_obj_t *child = lv_obj_get_child(view, j);
                lv_area_t a; lv_obj_get_coords(child, &a);
                if(a.y1 >= s_row_y[i] && a.y1 < s_row_y[i] + s_row_h[i]) lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void build_detail(lv_obj_t *view)
{
    image(view, &havit_today_task_detail_bg, 164, 22, 32);
    image(view, &havit_today_task_detail_close, 166, 24, 28);
    text(view, "ACT 8 - Fuentes", 60, 80, 240, &havit_instrument_sans_24, true, lv_color_white());
    text(view, "Seminario de Investigación de Diseño", 35, 113, 290,
         &havit_instrument_sans_15, true, lv_color_hex(0x00C3FF));
    text(view, "Entregar 5 fuentes académicas\niniciales", 45, 132, 270,
         &havit_instrument_sans_16, true, lv_color_white());
    text(view, "Limite: 11 de octubre 23:59", 100, 202, 170,
         &havit_instrument_sans_11, false, lv_color_hex(0xA4A4A4));
    image(view, &havit_today_task_more, 168, 180, 24);
    const char *buttons[] = {"EMPEZAR", "TERMINADO"};
    for(int i = 0; i < 2; i++) {
        lv_obj_t *button = lv_obj_create(view);
        lv_obj_remove_style_all(button);
        lv_obj_set_pos(button, 116, 229 + i * 51); lv_obj_set_size(button, 129, 40);
        lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(button, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
        lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        text(button, buttons[i], 0, 6, 129, &havit_instrument_sans_16, true, lv_color_white());
    }
    s_started_label = text(view, "", 116, 229, 129, &havit_instrument_sans_14,
                            true, lv_color_white());
}

void havit_ui_build_today_task_views(lv_obj_t *list, lv_obj_t *select, lv_obj_t *detail)
{
    s_views[0] = list; s_views[1] = select; s_views[2] = detail;
    memset(s_selected, 0, sizeof(s_selected));
    memset(s_removed, 0, sizeof(s_removed));
    s_started = false;
    build_list(list, false);
    build_list(select, true);
    build_detail(detail);
    ESP_LOGI(TAG, "Built Figma Today task list, selection and ACT 8 detail views");
}

static bool inside(int x, int y, int x1, int y1, int x2, int y2)
{ return x >= x1 && x <= x2 && y >= y1 && y <= y2; }

bool havit_ui_today_task_handle_touch(havit_ui_screen_t screen, int32_t x, int32_t y)
{
    if(screen == HAVIT_UI_SCREEN_TODAY_TASKS) {
        if(inside(x,y,55,55,99,99)) havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY);
        else if(inside(x,y,258,55,310,99)) havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY_TASKS_SELECT);
        else if(inside(x,y,265,110,305,161)) havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY_TASK_DETAIL);
        return true;
    }
    if(screen == HAVIT_UI_SCREEN_TODAY_TASKS_SELECT) {
        if(inside(x,y,55,55,99,99)) havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY);
        else if(inside(x,y,258,55,310,99)) havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY_TASKS);
        else if(y >= 110 && y <= 290) {
            int row = y < 163 ? 0 : y < 232 ? 1 : 2;
            s_selected[row] = !s_selected[row];
            if(s_selected[row]) { lv_obj_clear_flag(s_check_marks[row], LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(s_checks[row], LV_OBJ_FLAG_HIDDEN); }
            else { lv_obj_add_flag(s_check_marks[row], LV_OBJ_FLAG_HIDDEN); lv_obj_clear_flag(s_checks[row], LV_OBJ_FLAG_HIDDEN); }
        } else if(inside(x,y,140,300,186,343)) {
            int removed = 0;
            for(int i=0;i<3;i++) if(s_selected[i]) { s_removed[i]=true; removed++; }
            if(removed) {
                /* Rebuild the two local mock-up views; app task data is untouched. */
                lv_obj_clean(s_views[0]); lv_obj_clean(s_views[1]);
                memset(s_selected,0,sizeof(s_selected));
                build_list(s_views[0], false); build_list(s_views[1], true);
                ESP_LOGI(TAG, "Removed %d selected demo rows locally", removed);
            }
        } else if(inside(x,y,188,300,233,343)) {
            ESP_LOGI(TAG, "Share action tapped; device share service is not part of this view");
        }
        return true;
    }
    if(screen == HAVIT_UI_SCREEN_TODAY_TASK_DETAIL) {
        if(inside(x,y,153,15,207,63)) { s_started=false; havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY_TASKS); }
        else if(inside(x,y,105,220,255,273)) {
            s_started=true;
            lv_label_set_text(s_started_label, "EN CURSO");
            ESP_LOGI(TAG, "ACT 8 started in demo detail view");
        } else if(inside(x,y,105,274,255,326)) {
            s_started=false;
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY_TASKS);
        }
        return true;
    }
    return false;
}
