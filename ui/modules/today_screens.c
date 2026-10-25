#include "havit_ui.h"
#include "havit_today_assets.h"
#include "havit_add_transition.h"
#include "esp_log.h"
#include <math.h>
#include "havit_tasks.h"
#include <string.h>
#include <time.h>

/* Figma 1X1: 364:13, 364:46, 364:260, 364:384. All coordinates
 * are in the native 360x360 display space. Assets are rasterized offline. */
extern const lv_font_t havit_instrument_sans_8;
extern const lv_font_t havit_instrument_sans_12;
extern const lv_font_t havit_instrument_sans_16;
extern const lv_font_t havit_instrument_sans_20;
extern const lv_font_t havit_instrument_sans_24;
extern const lv_font_t havit_instrument_sans_32;

#define WHITE 0xFFFFFF
#define BLUE 0x74B2CD
#define LILAC 0x9594C8

static lv_obj_t *s_day_title;
static lv_obj_t *s_day_date;
static lv_obj_t *s_activity_images[3];
static lv_obj_t *s_activity_labels[3];
static lv_obj_t *s_calendar_labels[31];
static int s_selected_day = 1, s_month = 1, s_year = 2026, s_days = 31, s_first_weekday;
static bool s_calendar_ready;
static int s_counts[31];
static lv_obj_t *s_dots[31], *s_month_title, *s_year_title;
static havit_task_t s_task;
static const char *s_months[] = {"enero","febrero","marzo","abril","mayo","junio","julio","agosto","septiembre","octubre","noviembre","diciembre"};
static havit_ui_screen_t s_add_return_screen = HAVIT_UI_SCREEN_TODAY;
static havit_add_scene_t s_today_scene;
static havit_add_scene_t s_add_scene;

static const lv_font_t *font_at(int size)
{
    switch(size) {
        case 8: return &havit_instrument_sans_8;
        case 12: return &havit_instrument_sans_12;
        case 16: return &havit_instrument_sans_16;
        case 20: return &havit_instrument_sans_20;
        case 24: return &havit_instrument_sans_24;
        default: return &havit_instrument_sans_32;
    }
}

static lv_obj_t *image_at(lv_obj_t *parent, const lv_image_dsc_t *asset, int x, int y)
{
    lv_obj_t *image = lv_image_create(parent);
    lv_image_set_src(image, asset);
    lv_obj_set_pos(image, x, y);
    lv_obj_clear_flag(image, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return image;
}

/* Figma line boxes can be smaller than the font's ascender/descender box.
 * Keep the line box's center while allowing LVGL to size the full glyph run. */
static lv_obj_t *text_at(lv_obj_t *parent, const char *text, int x, int y,
                         int width, int size, int figma_line_height, uint32_t color)
{
    const lv_font_t *font = font_at(size);
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_remove_style_all(label);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(label, figma_line_height - font->line_height, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, width);
    lv_obj_set_height(label, LV_SIZE_CONTENT);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y + (figma_line_height - font->line_height) / 2);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    return label;
}

static void summary(havit_add_scene_t *scene, const char *date)
{
    lv_obj_t *parent = scene->view;
    /* Images first, labels last: the central gradient never covers text. */
    scene->fixed[0] = image_at(parent, &havit_today_imgEllipse10, 122, 122);
    scene->fixed[1] = image_at(parent, &havit_today_imgEllipse11, 116, 116);
    scene->fixed[2] = image_at(parent, &havit_today_imgEllipse14, 54, 59);
    scene->fixed[3] = text_at(parent, "Tareas", 120, 134, 120, 32, 39, WHITE);
    scene->date = text_at(parent, date, 120, 164, 120, 12, 28, WHITE);
    scene->fixed[4] = text_at(parent, "0 tareas", 110, 189, 140, 24, 18, WHITE);
}

static void build_today(lv_obj_t *parent)
{
    s_today_scene.view = parent;
    summary(&s_today_scene, "Esperando fecha…");
    /* The four circles are 67px; their centers are (180.5,64.5),
     * (58.5,179.5), (302.5,179.5), (180.5,294.5). */
    s_today_scene.fixed[5] = image_at(parent, &havit_add_imgEllipse23, 147, 31);
    s_today_scene.fixed[6] = image_at(parent, &havit_add_imgEllipse23, 25, 146);
    s_today_scene.color_buttons[0] = image_at(parent, &havit_add_imgEllipse23, 269, 146);
    s_today_scene.color_buttons[1] = image_at(parent, &havit_add_imgEllipse23, 147, 261);
    s_today_scene.controls[0] = image_at(parent, &havit_today_imgGroup, 163, 48);
    s_today_scene.controls[1] = image_at(parent, &havit_today_imgGroup1, 286, 163);
    s_today_scene.controls[2] = image_at(parent, &havit_today_imgGroup2, 43, 163);
    s_today_scene.controls[3] = image_at(parent, &havit_today_imgAndrsrxnPlus, 157, 271);
    s_today_scene.control_count = 4;
}

static void build_add(lv_obj_t *parent)
{
    s_add_scene.view = parent;
    summary(&s_add_scene, "Esperando fecha…");
    s_add_scene.fixed[5] = image_at(parent, &havit_add_imgEllipse23, 147, 31);
    s_add_scene.fixed[6] = image_at(parent, &havit_add_imgEllipse23, 25, 146);
    s_add_scene.color_buttons[0] = image_at(parent, &havit_add_imgEllipse24, 269, 146);
    s_add_scene.color_buttons[1] = image_at(parent, &havit_add_imgEllipse26, 147, 261);
    s_add_scene.controls[0] = image_at(parent, &havit_add_imgEllipse28, 164, 48);
    s_add_scene.controls[1] = image_at(parent, &havit_add_imgHugeiconsCancel01, 166, 50);
    s_add_scene.controls[2] = text_at(parent, "Agregar\nTarea", 25, 166, 67, 12, 14, 0x000000);
    s_add_scene.controls[3] = text_at(parent, "Tarea\nRápida", 269, 164, 67, 12, 14, 0x000000);
    s_add_scene.controls[4] = text_at(parent, "Tarea en\nEquipo", 147, 283, 67, 12, 14, 0x000000);
    s_add_scene.control_count = 5;
}

bool havit_ui_start_add_transition(havit_ui_screen_t source,
                                    havit_ui_screen_t destination,
                                    lv_anim_completed_cb_t completed)
{
    bool opening = source == HAVIT_UI_SCREEN_TODAY && destination == HAVIT_UI_SCREEN_ADD;
    bool closing = source == HAVIT_UI_SCREEN_ADD && destination == HAVIT_UI_SCREEN_TODAY;
    if(!opening && !closing) return false;
    havit_add_transition_start(opening ? &s_today_scene : &s_add_scene,
                                opening ? &s_add_scene : &s_today_scene, completed);
    ESP_LOGI("havit_today", "Add component transition: 300 ms, Ease Out");
    return true;
}

static int activity_count(int day)
{
    return day >= 1 && day <= s_days ? s_counts[day - 1] : 0;
}

static int day_column(int day) { return (day - 1 + s_first_weekday) % 7; }
static int day_row(int day) { return (day - 1 + s_first_weekday) / 7; }
static void month_layout(void) {
    struct tm first = {.tm_year=s_year-1900,.tm_mon=s_month-1,.tm_mday=1}; mktime(&first);
    s_first_weekday = (first.tm_wday+6)%7;
    struct tm end = {.tm_year=s_year-1900,.tm_mon=s_month,.tm_mday=0}; mktime(&end); s_days=end.tm_mday;
}

static void build_calendar(lv_obj_t *parent)
{
    s_month_title = text_at(parent, "Calendario", 102, 48, 150, 20, 28, WHITE);
    s_year_title = text_at(parent, "", 210, 58, 55, 12, 15, WHITE);
    lv_obj_set_style_text_opa(s_year_title, LV_OPA_50, 0);
    image_at(parent, &havit_month_imgIcon, 24, 170);
    lv_obj_t *next = image_at(parent, &havit_month_imgIcon, 320, 171);
    lv_image_set_rotation(next, 1800);
    static const char *week[] = {"L", "M", "X", "J", "V", "S", "D"};
    for(int col = 0; col < 7; col++) {
        lv_obj_t *day = text_at(parent, week[col], (int)roundf(68.33f + 32.347f * col),
                               109, 29, 12, 15, WHITE);
        lv_obj_set_style_text_opa(day, LV_OPA_50, 0);
    }
    for(int day = 1; day <= 31; day++) {
        char number[4];
        lv_snprintf(number, sizeof(number), "%d", day);
        const int x = (int)roundf(68.33f + 32.347f * day_column(day));
        const int y = (int)roundf(132 + 27 * day_row(day));
        s_calendar_labels[day - 1] = text_at(parent, number, x, y, 29, 16, 24, BLUE);
        s_dots[day-1] = lv_obj_create(parent); lv_obj_remove_style_all(s_dots[day-1]);
        lv_obj_set_size(s_dots[day-1],4,4); lv_obj_set_style_radius(s_dots[day-1],2,0);
        lv_obj_set_style_bg_color(s_dots[day-1],lv_color_hex(LILAC),0); lv_obj_set_style_bg_opa(s_dots[day-1],LV_OPA_COVER,0);
        lv_obj_add_flag(s_dots[day-1],LV_OBJ_FLAG_HIDDEN);
    }
    /* Explicit requested back control; the supplied month frame has none. */
    image_at(parent, &havit_day_imgEllipse28, 164, 312);
    image_at(parent, &havit_day_imgHugeiconsCancel01, 166, 314);
}

static void build_calendar_day(lv_obj_t *parent)
{
    image_at(parent, &havit_day_imgEllipse19, 122, 122);
    image_at(parent, &havit_day_imgEllipse10, 116, 116);
    image_at(parent, &havit_day_imgEllipse14, 54, 59);
    s_day_title = text_at(parent, "0\nTareas", 120, 140, 121, 24, 18, WHITE);
    s_day_date = text_at(parent, "", 120, 180, 122, 16, 24, WHITE);
    s_activity_images[0] = image_at(parent, &havit_day_imgEllipse15, 150, 34);
    s_activity_images[1] = image_at(parent, &havit_day_imgEllipse16, 80, 61);
    s_activity_images[2] = image_at(parent, &havit_day_imgEllipse18, 40, 121);
    /* Text is a sibling of each circle, as in Figma. Parenting it to the circle
     * clips labels that intentionally sit outside the circle's bounds. */
    s_activity_labels[0] = text_at(parent, "", 153, 77, 80, 8, 28, WHITE);
    s_activity_labels[1] = text_at(parent, "", 76, 104, 80, 8, 28, WHITE);
    s_activity_labels[2] = text_at(parent, "", 46, 162, 80, 8, 28, WHITE);
    image_at(parent, &havit_day_imgEllipse28, 141, 312);
    image_at(parent, &havit_day_imgHugeiconsCancel01, 143, 314);
    image_at(parent, &havit_day_imgGroup23, 185, 311);
}

static void update_calendar_selection(void)
{
    if(!s_calendar_ready) {
        lv_label_set_text(s_month_title,"Esperando hora"); lv_label_set_text(s_year_title,"");
        lv_label_set_text(s_day_date,"Esperando hora");
        for(int i=0;i<31;++i) {
            lv_obj_add_flag(s_calendar_labels[i],LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_dots[i],LV_OBJ_FLAG_HIDDEN);
        }
        for(int i=0;i<3;++i) {
            lv_obj_add_flag(s_activity_images[i],LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_activity_labels[i],LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }
    char date[32], count[24];
    const int tasks = activity_count(s_selected_day);
    lv_snprintf(date, sizeof(date), "%d de %s", s_selected_day, s_months[s_month-1]);
    lv_snprintf(count, sizeof(count), "%d\nTareas", tasks);
    lv_label_set_text(s_day_title, count);
    lv_label_set_text(s_day_date, date);
    for(int i = 0; i < 3; i++) {
        if(i < tasks) {
            lv_obj_clear_flag(s_activity_images[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_activity_labels[i], LV_OBJ_FLAG_HIDDEN);
        }
        else {
            lv_obj_add_flag(s_activity_images[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_activity_labels[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    for(int day = 1; day <= 31; day++) {
        int x=(int)roundf(68.33f+32.347f*day_column(day)), y=132+27*day_row(day);
        lv_obj_set_pos(s_calendar_labels[day-1],x,y);
        lv_obj_set_pos(s_dots[day-1],x+12,y+23);
        if(day>s_days) lv_obj_add_flag(s_calendar_labels[day-1],LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(s_calendar_labels[day-1],LV_OBJ_FLAG_HIDDEN);
        if(day<=s_days && activity_count(day)>0) lv_obj_clear_flag(s_dots[day-1],LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_dots[day-1],LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_color(s_calendar_labels[day - 1],
            lv_color_hex(day == s_selected_day ? WHITE : BLUE), 0);
    }
    havit_tasks_status_t status; havit_tasks_status(&status); int shown=0;
    char selected[11]; snprintf(selected,sizeof(selected),"%04d-%02d-%02d",s_year,s_month,s_selected_day);
    for(int i=0;i<status.count && shown<3;++i) {
        if(!havit_tasks_get(i,&s_task)) continue;
        char day[11]; havit_tasks_date(s_task.due_at,day);
        if(strcmp(day,selected)==0) lv_label_set_text(s_activity_labels[shown++],s_task.title);
    }
    lv_label_set_text(s_month_title,s_months[s_month-1]); char year[8]; snprintf(year,sizeof(year),"%d",s_year); lv_label_set_text(s_year_title,year);
}

void havit_ui_today_refresh(void) {
    static uint32_t last, revision=UINT32_MAX; static char previous_date[11];
    if(!s_day_title || lv_tick_elaps(last)<1000) return;
    last=lv_tick_get();
    havit_tasks_status_t status; havit_tasks_status(&status); struct tm now; havit_tasks_local_time(&now);
    char date[11]; strftime(date,sizeof(date),"%Y-%m-%d",&now);
    if(status.ui_revision==revision && strcmp(date,previous_date)==0) return;
    revision=status.ui_revision; snprintf(previous_date,sizeof(previous_date),"%s",date);
    int total=0,done=0;
    for(int i=0;i<status.count;++i) {
        if(!havit_tasks_get(i,&s_task)) continue;
        total++; if(s_task.completed) done++;
    }
    char display[48]; snprintf(display,sizeof(display),"%d/%d tareas",done,total);
    lv_label_set_text(s_today_scene.fixed[4],display); lv_label_set_text(s_add_scene.fixed[4],display);
    if(!status.clock_valid || !status.time_zone[0]) {
        havit_net_snapshot_t network; havit_connectivity_snapshot(&network);
        const char *date_status = network.wifi_connected ? "Conectando hora…" : "Sin conexión";
        lv_label_set_text(s_today_scene.date,date_status); lv_label_set_text(s_add_scene.date,date_status); return;
    }
    if(!s_calendar_ready) { s_calendar_ready=true; s_year=now.tm_year+1900; s_month=now.tm_mon+1; s_selected_day=now.tm_mday; }
    month_layout(); memset(s_counts,0,sizeof(s_counts));
    for(int i=0;i<status.count;++i) {
        if(!havit_tasks_get(i,&s_task)) continue;
        char day[11]; havit_tasks_date(s_task.due_at,day);
        if(day[0]) { int y,m,d; if(sscanf(day,"%d-%d-%d",&y,&m,&d)==3 && y==s_year && m==s_month && d>=1 && d<=s_days) s_counts[d-1]++; }
    }
    snprintf(display,sizeof(display),"%d de %s",now.tm_mday,s_months[now.tm_mon]);
    lv_label_set_text(s_today_scene.date,display); lv_label_set_text(s_add_scene.date,display);
    update_calendar_selection();
}

void havit_ui_build_today_views(lv_obj_t *today, lv_obj_t *add,
                                lv_obj_t *calendar, lv_obj_t *calendar_day)
{
    build_today(today);
    build_add(add);
    build_calendar(calendar);
    build_calendar_day(calendar_day);
    update_calendar_selection();
    ESP_LOGI("havit_today", "Figma geometry ready: 67px orbit buttons, 32/12/24px summary fonts");
}

static bool inside_circle(int x, int y, int cx, int cy, int radius)
{
    const int dx = x - cx, dy = y - cy;
    return dx * dx + dy * dy <= radius * radius;
}

void havit_ui_today_handle_touch(havit_ui_screen_t screen, int32_t x, int32_t y)
{
    if(screen == HAVIT_UI_SCREEN_TODAY) {
        if(inside_circle(x, y, 180, 295, 39)) {
            s_add_return_screen = HAVIT_UI_SCREEN_TODAY;
            havit_ui_navigate_to(HAVIT_UI_SCREEN_ADD);
        }
        else if(inside_circle(x, y, 303, 180, 39))
            havit_ui_navigate_to(HAVIT_UI_SCREEN_CALENDAR);
        else if(inside_circle(x,y,180,180,55))
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY_TASKS);
        else if(inside_circle(x,y,58,180,39))
            havit_ui_tasks_open_list(NULL);
    }
    else if(screen == HAVIT_UI_SCREEN_ADD) {
        if(inside_circle(x, y, 180, 65, 39))
            havit_ui_navigate_to(s_add_return_screen);
        else if(inside_circle(x, y, 303, 180, 39))
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_QUICK);
        else if(inside_circle(x,y,58,180,39)) havit_ui_tasks_new();
        else if(inside_circle(x, y, 180, 294, 39))
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TEAM_CREATE);
    }
    else if(screen == HAVIT_UI_SCREEN_CALENDAR) {
        if(inside_circle(x, y, 180, 328, 24)) {
            havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY);
            return;
        }
        if(!s_calendar_ready) return;
        if((x<52 || x>308) && y>140 && y<220) {
            s_month += x<52 ? -1 : 1;
            if(s_month<1) { s_month=12; s_year--; } if(s_month>12) { s_month=1; s_year++; }
            if(s_year<2020) { s_year=2020; s_month=1; }
            if(s_year>2099) { s_year=2099; s_month=12; }
            month_layout(); s_selected_day=1;
            /* Force data/count refresh even if task revision is unchanged. */
            memset(s_counts,0,sizeof(s_counts)); havit_tasks_status_t status; havit_tasks_status(&status);
            for(int i=0;i<status.count;++i) { if(!havit_tasks_get(i,&s_task)) continue; char date[11]; havit_tasks_date(s_task.due_at,date); int yy,mm,dd;
                if(sscanf(date,"%d-%d-%d",&yy,&mm,&dd)==3 && yy==s_year && mm==s_month && dd>=1 && dd<=s_days) s_counts[dd-1]++; }
            update_calendar_selection(); return;
        }
        if(x >= 66 && x < 293 && y >= 132 && y < 294) {
            int col = (int)((x - 66) / 32.347f);
            int row = (y - 132) / 27;
            int day = row * 7 + col + 1 - s_first_weekday;
            if(day >= 1 && day <= s_days) {
                s_selected_day = day;
                update_calendar_selection();
                havit_ui_navigate_to(HAVIT_UI_SCREEN_CALENDAR_DAY);
            }
        }
    }
    else if(screen == HAVIT_UI_SCREEN_CALENDAR_DAY) {
        if(inside_circle(x, y, 157, 328, 24))
            havit_ui_navigate_to(HAVIT_UI_SCREEN_CALENDAR);
        else if(inside_circle(x, y, 201, 327, 24)) {
            s_add_return_screen = HAVIT_UI_SCREEN_CALENDAR_DAY;
            havit_ui_navigate_to(HAVIT_UI_SCREEN_ADD);
        }
        else if(y<285) { char date[11]; snprintf(date,sizeof(date),"%04d-%02d-%02d",s_year,s_month,s_selected_day); havit_ui_tasks_open_list(date); }
    }
}
