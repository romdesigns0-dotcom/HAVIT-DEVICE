/**
 * @file today_b_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "today_b_gen.h"
#include "../ui.h"
#include "../ui_ring_overlay.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * today_b_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "today_b_#");
        lv_obj_set_style_bg_color(lv_obj_0, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(lv_obj_0, 10, 0);
        lv_obj_set_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_set_style_pad_all(lv_obj_0, 0, 0);

        lv_obj_add_style(lv_obj_0, &style_screen_base_global, 0);
        lv_obj_t * panel_0 = panel_create(lv_obj_0);
        lv_obj_set_align(panel_0, LV_ALIGN_CENTER);
        lv_obj_set_x(panel_0, 0);
        lv_obj_set_y(panel_0, 0);
        lv_obj_set_width(panel_0, 360);
        lv_obj_set_height(panel_0, 360);
        lv_obj_set_style_bg_color(panel_0, lv_color_hex(0x000000), 0);

        lv_obj_t * container_0 = container_create(lv_obj_0);
        lv_obj_set_align(container_0, LV_ALIGN_CENTER);
        lv_obj_set_x(container_0, 0);
        lv_obj_set_y(container_0, 0);
        lv_obj_set_width(container_0, 360);
        lv_obj_set_height(container_0, 360);
        lv_obj_set_style_outline_color(container_0, lv_color_hex(0x000000), 0);
        lv_obj_set_style_outline_opa(container_0, 16, 0);
        lv_obj_set_style_outline_width(container_0, 1, 0);
        lv_obj_set_style_radius(container_0, 32767, 0);
        lv_obj_set_style_clip_corner(container_0, true, 0);
        lv_obj_t * container_1 = container_create(lv_obj_0);
        lv_obj_set_align(container_1, LV_ALIGN_CENTER);
        lv_obj_set_x(container_1, 0);
        lv_obj_set_y(container_1, 0);
        lv_obj_set_width(container_1, 350);
        lv_obj_set_height(container_1, 350);
        lv_obj_set_style_border_color(container_1, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_1, 255, 0);
        lv_obj_set_style_border_width(container_1, 7, 0);
        lv_obj_set_style_transform_rotation(container_1, 1800, 0);
        lv_obj_set_style_transform_pivot_x(container_1, 175, 0);
        lv_obj_set_style_transform_pivot_y(container_1, 175, 0);
        lv_obj_set_style_radius(container_1, 32767, 0);
        lv_obj_set_style_clip_corner(container_1, true, 0);
        lv_obj_t * card_0 = card_create(lv_obj_0);
        lv_obj_set_align(card_0, LV_ALIGN_CENTER);
        lv_obj_set_x(card_0, 1);
        lv_obj_set_y(card_0, 1);
        lv_obj_set_width(card_0, 117);
        lv_obj_set_height(card_0, 117);
        lv_obj_set_style_bg_color(card_0, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_bg_grad_color(card_0, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_bg_grad_dir(card_0, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_opa(card_0, 196, 0);
        lv_obj_set_style_radius(card_0, 32767, 0);

        lv_obj_t * label_0 = label_create(lv_obj_0);
        lv_obj_set_align(label_0, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_0, 0);
        lv_obj_set_y(label_0, 172);
        lv_obj_set_height(label_0, 28);
        lv_label_set_text(label_0, "18 de octubre");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_align(label_1, LV_ALIGN_CENTER);
        lv_obj_set_x(label_1, 0);
        lv_obj_set_y(label_1, -22);
        lv_obj_set_height(label_1, 39);
        lv_label_set_text(label_1, "Hoy");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_32, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_2 = label_create(lv_obj_0);
        lv_obj_set_align(label_2, LV_ALIGN_CENTER);
        lv_obj_set_x(label_2, 0);
        lv_obj_set_y(label_2, 16);
        lv_obj_set_height(label_2, 40);
        lv_label_set_text(label_2, "0 tareas");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_24, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(label_2, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * container_2 = container_create(lv_obj_0);
        lv_obj_set_align(container_2, LV_ALIGN_CENTER);
        lv_obj_set_x(container_2, 1);
        lv_obj_set_y(container_2, 1);
        lv_obj_set_width(container_2, 129);
        lv_obj_set_height(container_2, 129);
        lv_obj_set_style_border_color(container_2, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_2, 255, 0);
        lv_obj_set_style_border_width(container_2, 2, 0);
        lv_obj_set_style_radius(container_2, 32767, 0);
        lv_obj_set_style_clip_corner(container_2, true, 0);
        lv_obj_t * container_3 = container_create(lv_obj_0);
        lv_obj_set_align(container_3, LV_ALIGN_CENTER);
        lv_obj_set_x(container_3, -1);
        lv_obj_set_y(container_3, 3);
        lv_obj_set_width(container_3, 249);
        lv_obj_set_height(container_3, 248);
        lv_obj_set_style_border_color(container_3, lv_color_hex(0x77B7DB), 0);
        lv_obj_set_style_border_opa(container_3, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(container_3, 2, 0);
        lv_obj_set_style_radius(container_3, 32767, 0);
        lv_obj_set_style_clip_corner(container_3, true, 0);
        lv_obj_t * container_4 = container_create(lv_obj_0);
        lv_obj_set_align(container_4, LV_ALIGN_CENTER);
        lv_obj_set_x(container_4, 0);
        lv_obj_set_y(container_4, 0);
        lv_obj_set_width(container_4, 360);
        lv_obj_set_height(container_4, 360);
        lv_obj_set_style_outline_color(container_4, lv_color_hex(0x4A4A4A), 0);
        lv_obj_set_style_outline_opa(container_4, 255, 0);
        lv_obj_set_style_outline_width(container_4, 4, 0);
        lv_obj_set_style_radius(container_4, 32767, 0);
        lv_obj_set_style_clip_corner(container_4, true, 0);
        lv_obj_t * button_subtle_0 = button_subtle_create(lv_obj_0);
        lv_obj_set_x(button_subtle_0, 26);
        lv_obj_set_y(button_subtle_0, 139);
        lv_obj_set_width(button_subtle_0, 67);
        lv_obj_set_height(button_subtle_0, 67);
        lv_obj_t * card_1 = card_create(button_subtle_0);
        lv_obj_set_align(card_1, LV_ALIGN_TOP_MID);
        lv_obj_set_x(card_1, 0);
        lv_obj_set_y(card_1, 0);
        lv_obj_set_width(card_1, 67);
        lv_obj_set_height(card_1, 67);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_radius(card_1, 32767, 0);
        lv_obj_set_flag(card_1, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(card_1, LV_OBJ_FLAG_EVENT_BUBBLE, true);

        lv_obj_add_screen_create_event(button_subtle_0, LV_EVENT_CLICKED, add_task_p1_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * button_subtle_1 = button_subtle_create(lv_obj_0);
        lv_obj_set_x(button_subtle_1, 263);
        lv_obj_set_y(button_subtle_1, 139);
        lv_obj_set_width(button_subtle_1, 67);
        lv_obj_set_height(button_subtle_1, 67);
        lv_obj_t * card_2 = card_create(button_subtle_1);
        lv_obj_set_align(card_2, LV_ALIGN_TOP_MID);
        lv_obj_set_x(card_2, 0);
        lv_obj_set_y(card_2, 0);
        lv_obj_set_width(card_2, 67);
        lv_obj_set_height(card_2, 67);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0xB3E3FF), 0);
        lv_obj_set_style_radius(card_2, 32767, 0);
        lv_obj_set_flag(card_2, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(card_2, LV_OBJ_FLAG_EVENT_BUBBLE, true);

        lv_obj_add_screen_create_event(button_subtle_1, LV_EVENT_CLICKED, add_task_p1_b_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * label_3 = label_create(lv_obj_0);
        lv_obj_set_x(label_3, 36);
        lv_obj_set_y(label_3, 159);
        lv_obj_set_width(label_3, 49);
        lv_obj_set_height(label_3, 28);
        lv_label_set_text(label_3, "Agregar \nTarea");
        lv_obj_set_style_text_font(label_3, font_instrument_sans_medium_12, 0);
        lv_obj_set_style_text_line_space(label_3, 2, 0);
        lv_obj_set_style_text_color(label_3, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_align(label_3, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * label_4 = label_create(lv_obj_0);
        lv_obj_set_x(label_4, 277);
        lv_obj_set_y(label_4, 159);
        lv_obj_set_width(label_4, 39);
        lv_obj_set_height(label_4, 28);
        lv_label_set_text(label_4, "Tarea \nRápida");
        lv_obj_set_style_text_font(label_4, font_instrument_sans_medium_12, 0);
        lv_obj_set_style_text_line_space(label_4, 2, 0);
        lv_obj_set_style_text_color(label_4, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_align(label_4, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * container_5 = container_create(lv_obj_0);
        lv_obj_set_x(container_5, 164);
        lv_obj_set_y(container_5, 43);
        lv_obj_set_width(container_5, 32);
        lv_obj_set_height(container_5, 32);
        lv_obj_t * card_3 = card_create(container_5);
        lv_obj_set_x(card_3, 0);
        lv_obj_set_y(card_3, 0);
        lv_obj_set_width(card_3, 32);
        lv_obj_set_height(card_3, 32);
        lv_obj_set_style_bg_color(card_3, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_radius(card_3, 32767, 0);
        lv_obj_set_flag(card_3, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(card_3, LV_OBJ_FLAG_EVENT_BUBBLE, true);

        lv_obj_t * container_6 = container_create(container_5);
        lv_obj_set_x(container_6, 2);
        lv_obj_set_y(container_6, 2);
        lv_obj_set_width(container_6, 28);
        lv_obj_set_height(container_6, 28);
        lv_obj_set_flag(container_6, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(container_6, LV_OBJ_FLAG_EVENT_BUBBLE, true);
        lv_obj_t * image_0 = image_create(container_6);
        lv_obj_set_x(image_0, 0);
        lv_obj_set_y(image_0, 0);
        lv_obj_set_width(image_0, 28);
        lv_obj_set_height(image_0, 28);
        lv_image_set_src(image_0, image_hugeicons_cancel_01_18077);
        lv_obj_set_flag(image_0, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_add_screen_create_event(container_5, LV_EVENT_CLICKED, today_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        the_root = lv_obj_0;
        ui_ring_overlay(the_root);
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

