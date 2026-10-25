#include "havit_ui.h"
#include "havit_tasks.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern const lv_font_t havit_instrument_sans_12, havit_instrument_sans_16, havit_instrument_sans_20;
static lv_obj_t *s_list, *s_detail, *s_editor, *s_rows, *s_status_label, *s_detail_title, *s_detail_text;
static lv_obj_t *s_title_input, *s_detail_input, *s_due_input, *s_keyboard, *s_form, *s_form_error, *s_complete_label;
static havit_task_t s_edit_task, s_current_task, s_scratch;
static char s_filter[11], s_detail_id[37];
static bool s_creating;
static uint32_t s_revision = UINT32_MAX;
static int s_last_screen = -1;

static lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y, int w, const lv_font_t *font) {
    lv_obj_t *l = lv_label_create(parent); lv_obj_set_pos(l, x, y); lv_obj_set_width(l, w);
    lv_obj_set_style_text_font(l, font, 0); lv_obj_set_style_text_color(l, lv_color_hex(0xffffff), 0);
    lv_label_set_text(l, text); return l;
}
static lv_obj_t *button(lv_obj_t *parent, const char *text, int x, int y, int w, int h, lv_event_cb_t cb) {
    lv_obj_t *b = lv_button_create(parent); lv_obj_set_pos(b, x, y); lv_obj_set_size(b, w, h);
    lv_obj_set_style_pad_all(b, 0, 0);
    lv_obj_set_style_radius(b, 22, 0); lv_obj_set_style_bg_color(b, lv_color_hex(0x73979a), 0);
    lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *l = label(b, text, 0, 0, w - 12, &havit_instrument_sans_12);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0); lv_obj_center(l); return b;
}
static bool find_task(const char *id, havit_task_t *out) {
    havit_tasks_status_t status; havit_tasks_status(&status);
    for(int i = 0; i < status.count; ++i) if(havit_tasks_get(i, out) && strcmp(out->id, id) == 0) return true;
    return false;
}
static void back(lv_event_t *e) { (void)e; havit_ui_navigate_to(HAVIT_UI_SCREEN_TODAY); }
static void detail_back(lv_event_t *e) { (void)e; havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_LIST); }
static void edit_back(lv_event_t *e) { (void)e; havit_ui_navigate_to(s_creating ? HAVIT_UI_SCREEN_TASK_LIST : HAVIT_UI_SCREEN_TASK_DETAIL); }
void havit_ui_tasks_open_list(const char *date) {
    snprintf(s_filter, sizeof(s_filter), "%s", date ? date : "");
    s_revision = UINT32_MAX; havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_LIST);
}
static void open_detail(lv_event_t *e) {
    lv_obj_t *b = lv_event_get_target(e); const char *id = lv_obj_get_user_data(b);
    if(id) { snprintf(s_detail_id, sizeof(s_detail_id), "%s", id); havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_DETAIL); }
}
static void free_row_id(lv_event_t *e) { free(lv_event_get_user_data(e)); }
static void fill_editor(void) {
    lv_textarea_set_text(s_title_input, s_edit_task.title); lv_textarea_set_text(s_detail_input, s_edit_task.detail);
    char date[24] = "";
    if(s_edit_task.due_at[0]) {
        havit_tasks_due_local(s_edit_task.due_at, date);
    } else if(s_filter[0] && s_creating) snprintf(date, sizeof(date), "%s 18:00", s_filter);
    lv_textarea_set_text(s_due_input, date); lv_label_set_text(s_form_error, "");
    lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN); lv_obj_set_height(s_form, 220);
}
void havit_ui_tasks_new(void) {
    memset(&s_edit_task, 0, sizeof(s_edit_task)); s_creating = true; fill_editor();
    havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_EDIT);
}
static void new_task(lv_event_t *e) { (void)e; havit_ui_tasks_new(); }
static void edit_task(lv_event_t *e) {
    (void)e; if(!find_task(s_detail_id, &s_edit_task)) return;
    s_creating = false; fill_editor(); havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_EDIT);
}
static void complete_task(lv_event_t *e) {
    (void)e; if(!find_task(s_detail_id, &s_current_task)) return;
    s_current_task.completed = !s_current_task.completed;
    if(havit_tasks_save(&s_current_task, false)) s_revision = UINT32_MAX;
}
static void delete_choice(lv_event_t *e) {
    lv_obj_t *box = lv_event_get_user_data(e);
    if(find_task(s_detail_id, &s_current_task) && havit_tasks_delete(&s_current_task)) {
        lv_msgbox_close(box); havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_LIST);
    }
}
static void delete_task(lv_event_t *e) {
    (void)e; lv_obj_t *box = lv_msgbox_create(NULL); lv_msgbox_add_title(box,"Eliminar actividad");
    lv_msgbox_add_text(box,"Se eliminara de tu cuenta y dispositivos al sincronizar.");
    lv_msgbox_add_close_button(box); lv_obj_t *b = lv_msgbox_add_footer_button(box,"Eliminar");
    lv_obj_add_event_cb(b, delete_choice, LV_EVENT_CLICKED, box);
}
static void save_task(lv_event_t *e) {
    (void)e; const char *title=lv_textarea_get_text(s_title_input), *detail=lv_textarea_get_text(s_detail_input);
    if(!title[0] || strlen(title)>160 || strlen(detail)>2048) { lv_label_set_text(s_form_error,"Acorta el titulo o los detalles."); return; }
    if(!havit_tasks_due_from_local(lv_textarea_get_text(s_due_input),s_edit_task.due_at)) {
        lv_label_set_text(s_form_error,"Fecha: AAAA-MM-DD HH:MM"); return;
    }
    snprintf(s_edit_task.title,sizeof(s_edit_task.title),"%s",title);
    snprintf(s_edit_task.detail,sizeof(s_edit_task.detail),"%s",detail);
    if(!havit_tasks_save(&s_edit_task,s_creating)) { havit_tasks_status_t status; havit_tasks_status(&status); lv_label_set_text(s_form_error,status.message); return; }
    havit_ui_navigate_to(HAVIT_UI_SCREEN_TASK_LIST);
}
static void focus_input(lv_event_t *e) {
    lv_keyboard_set_textarea(s_keyboard,lv_event_get_target(e));
    lv_obj_clear_flag(s_keyboard,LV_OBJ_FLAG_HIDDEN); lv_obj_set_height(s_form,105);
    lv_obj_scroll_to_view(lv_event_get_target(e),LV_ANIM_ON);
}
static void keyboard_done(lv_event_t *e) {
    if(lv_event_get_code(e)==LV_EVENT_READY || lv_event_get_code(e)==LV_EVENT_CANCEL) {
        lv_obj_add_flag(s_keyboard,LV_OBJ_FLAG_HIDDEN); lv_obj_set_height(s_form,220);
    }
}
static lv_obj_t *input(lv_obj_t *p,const char *placeholder,int y,int height) {
    lv_obj_t *t=lv_textarea_create(p); lv_obj_set_pos(t,0,y); lv_obj_set_size(t,246,height);
    lv_obj_set_style_text_font(t,&havit_instrument_sans_12,0); lv_textarea_set_placeholder_text(t,placeholder);
    lv_obj_add_event_cb(t,focus_input,LV_EVENT_CLICKED,NULL); return t;
}
static void resolve_local(lv_event_t *e) { havit_tasks_resolve(true); lv_msgbox_close(lv_event_get_user_data(e)); }
static void resolve_remote(lv_event_t *e) { havit_tasks_resolve(false); lv_msgbox_close(lv_event_get_user_data(e)); }
static void refresh_tasks(lv_event_t *e) {
    (void)e; havit_tasks_status_t status; havit_tasks_status(&status);
    if(!status.conflicts) { havit_tasks_refresh(); return; }
    lv_obj_t *box=lv_msgbox_create(NULL); lv_msgbox_add_title(box,"Cambio en conflicto");
    lv_msgbox_add_text(box,"Otra pantalla modifico la actividad. Elige que version conservar.");
    lv_msgbox_add_close_button(box);
    lv_obj_add_event_cb(lv_msgbox_add_footer_button(box,"Mi cambio"),resolve_local,LV_EVENT_CLICKED,box);
    lv_obj_add_event_cb(lv_msgbox_add_footer_button(box,"Servidor"),resolve_remote,LV_EVENT_CLICKED,box);
}
void havit_ui_build_task_views(lv_obj_t *list,lv_obj_t *detail,lv_obj_t *editor) {
    s_list=list; s_detail=detail; s_editor=editor;
    label(list,"Actividades",83,27,175,&havit_instrument_sans_20);
    button(list,"+",260,32,42,35,new_task);
    s_status_label=label(list,"Preparando…",52,68,256,&havit_instrument_sans_12);
    s_rows=lv_obj_create(list); lv_obj_set_pos(s_rows,45,102); lv_obj_set_size(s_rows,270,187);
    lv_obj_set_style_bg_opa(s_rows,LV_OPA_TRANSP,0); lv_obj_set_style_border_width(s_rows,0,0);
    lv_obj_set_flex_flow(s_rows,LV_FLEX_FLOW_COLUMN); lv_obj_set_style_pad_all(s_rows,2,0);
    button(list,"Volver",65,302,105,36,back); button(list,"Revisar / actualizar",181,302,126,36,refresh_tasks);
    s_detail_title=label(detail,"Actividad",60,35,240,&havit_instrument_sans_20);
    lv_obj_set_height(s_detail_title,46); lv_label_set_long_mode(s_detail_title,LV_LABEL_LONG_DOT);
    lv_obj_t *scroll=lv_obj_create(detail); lv_obj_set_pos(scroll,46,85); lv_obj_set_size(scroll,268,143);
    lv_obj_set_style_bg_opa(scroll,LV_OPA_TRANSP,0); lv_obj_set_style_border_width(scroll,0,0);
    s_detail_text=label(scroll,"",0,0,232,&havit_instrument_sans_12);
    lv_obj_t *complete=button(detail,"Completar",72,237,216,36,complete_task); s_complete_label=lv_obj_get_child(complete,0);
    button(detail,"Editar",47,281,82,35,edit_task); button(detail,"Eliminar",139,281,82,35,delete_task); button(detail,"Volver",231,281,82,35,detail_back);
    label(editor,"Actividad",80,25,140,&havit_instrument_sans_20); button(editor,"Guardar",239,30,70,34,save_task);
    s_form=lv_obj_create(editor); lv_obj_set_pos(s_form,48,72); lv_obj_set_size(s_form,264,220);
    lv_obj_set_style_bg_opa(s_form,LV_OPA_TRANSP,0); lv_obj_set_style_border_width(s_form,0,0); lv_obj_set_style_pad_all(s_form,4,0);
    s_title_input=input(s_form,"Titulo",0,43); lv_textarea_set_one_line(s_title_input,true); lv_textarea_set_max_length(s_title_input,160);
    s_detail_input=input(s_form,"Detalles",53,72); lv_textarea_set_max_length(s_detail_input,2048);
    s_due_input=input(s_form,"Fecha opcional: AAAA-MM-DD HH:MM",136,43); lv_textarea_set_one_line(s_due_input,true); lv_textarea_set_max_length(s_due_input,16);
    s_form_error=label(editor,"",65,286,230,&havit_instrument_sans_12); button(editor,"Cancelar",125,315,110,32,edit_back);
    s_keyboard=lv_keyboard_create(editor); lv_obj_set_pos(s_keyboard,15,186); lv_obj_set_size(s_keyboard,330,136);
    lv_obj_add_event_cb(s_keyboard,keyboard_done,LV_EVENT_ALL,NULL); lv_obj_add_flag(s_keyboard,LV_OBJ_FLAG_HIDDEN);
}
void havit_ui_tasks_tick(void) {
    static uint32_t last;
    if(lv_tick_elaps(last)<500 || !s_list) return;
    last=lv_tick_get();
    havit_tasks_status_t status; havit_tasks_status(&status); int screen=havit_ui_current_screen();
    if(screen!=HAVIT_UI_SCREEN_TASK_LIST && screen!=HAVIT_UI_SCREEN_TASK_DETAIL) return;
    if(status.ui_revision==s_revision && screen==s_last_screen) { if(screen==HAVIT_UI_SCREEN_TASK_LIST) lv_label_set_text(s_status_label,status.message); return; }
    s_revision=status.ui_revision; s_last_screen=screen;
    if(screen==HAVIT_UI_SCREEN_TASK_LIST) {
        lv_obj_clean(s_rows); lv_label_set_text(s_status_label,status.message); int visible=0;
        for(int i=0;i<status.count;++i) {
            if(!havit_tasks_get(i,&s_scratch)) continue;
            char day[11]; havit_tasks_date(s_scratch.due_at,day); if(s_filter[0] && strcmp(day,s_filter)!=0) continue;
            lv_obj_t *b=lv_button_create(s_rows); lv_obj_set_width(b,246); lv_obj_set_height(b,69);
            lv_obj_set_style_pad_all(b,8,0);
            lv_obj_set_style_bg_color(b,lv_color_hex(0x202124),0); lv_obj_set_style_radius(b,15,0);
            char *id=strdup(s_scratch.id); lv_obj_set_user_data(b,id); lv_obj_add_event_cb(b,open_detail,LV_EVENT_CLICKED,NULL);
            lv_obj_add_event_cb(b,free_row_id,LV_EVENT_DELETE,id);
            char text[240]; snprintf(text,sizeof(text),"%s%s\n%s",s_scratch.completed?"[OK] ":"",s_scratch.title,day[0]?day:"Sin fecha");
            lv_obj_t *row_label=label(b,text,0,0,223,&havit_instrument_sans_12);
            lv_obj_set_height(row_label,52); lv_label_set_long_mode(row_label,LV_LABEL_LONG_DOT); visible++;
        }
        if(!visible) label(s_rows,"No hay actividades aqui.\nPulsa + para agregar una.",0,0,240,&havit_instrument_sans_12);
    } else if(find_task(s_detail_id,&s_current_task)) {
        lv_label_set_text(s_detail_title,s_current_task.title);
        static char text[2300]; char due[24]; havit_tasks_due_local(s_current_task.due_at,due);
        snprintf(text,sizeof(text),"%s\n%s\n\n%s",s_current_task.title,due[0]?due:"Sin fecha de entrega",s_current_task.detail[0]?s_current_task.detail:"Sin detalles");
        lv_label_set_text(s_detail_text,text); lv_label_set_text(s_complete_label,s_current_task.completed?"Marcar pendiente":"Completar");
    } else { lv_label_set_text(s_detail_title,"Actividad eliminada"); lv_label_set_text(s_detail_text,"Esta actividad ya no esta disponible."); }
}
