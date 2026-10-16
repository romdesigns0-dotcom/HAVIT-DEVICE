/**
 * @file add_task_p1_b_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "add_task_p1_b_gen.h"
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

lv_obj_t * add_task_p1_b_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "add_task_p1_b_#");
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
        lv_obj_t * label_0 = label_create(lv_obj_0);
        lv_obj_set_align(label_0, LV_ALIGN_CENTER);
        lv_obj_set_x(label_0, -7);
        lv_obj_set_y(label_0, -114);
        lv_obj_set_height(label_0, 29);
        lv_label_set_text(label_0, "Tarea Rápida");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_24, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * container_2 = container_create(lv_obj_0);
        lv_obj_set_align(container_2, LV_ALIGN_CENTER);
        lv_obj_set_x(container_2, 0);
        lv_obj_set_y(container_2, 0);
        lv_obj_set_width(container_2, 360);
        lv_obj_set_height(container_2, 360);
        lv_obj_set_style_outline_color(container_2, lv_color_hex(0x4A4A4A), 0);
        lv_obj_set_style_outline_opa(container_2, 255, 0);
        lv_obj_set_style_outline_width(container_2, 4, 0);
        lv_obj_set_style_radius(container_2, 32767, 0);
        lv_obj_set_style_clip_corner(container_2, true, 0);
        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_align(label_1, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_1, -62);
        lv_obj_set_y(label_1, 147);
        lv_obj_set_height(label_1, 28);
        lv_label_set_text(label_1, "Nombre de la tarea:");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * image_0 = image_create(lv_obj_0);
        lv_obj_set_x(image_0, 46);
        lv_obj_set_y(image_0, 196);
        lv_obj_set_width(image_0, 250);
        lv_obj_set_height(image_0, 1);
        lv_image_set_src(image_0, image_vec_197_581);

        lv_obj_t * image_1 = image_create(lv_obj_0);
        lv_obj_set_x(image_1, 272);
        lv_obj_set_y(image_1, 161);
        lv_obj_set_width(image_1, 24);
        lv_obj_set_height(image_1, 24);
        lv_image_set_src(image_1, image_ic_baseline_keyboard_voice);
        lv_obj_set_style_bg_opa(image_1, 255, 0);
        lv_obj_set_style_bg_color(image_1, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_radius(image_1, 3, 0);
        lv_obj_set_flag(image_1, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * container_3 = container_create(lv_obj_0);
        lv_obj_set_x(container_3, 164);
        lv_obj_set_y(container_3, 90);
        lv_obj_set_width(container_3, 32);
        lv_obj_set_height(container_3, 32);
        lv_obj_t * card_0 = card_create(container_3);
        lv_obj_set_x(card_0, 0);
        lv_obj_set_y(card_0, 0);
        lv_obj_set_width(card_0, 32);
        lv_obj_set_height(card_0, 32);
        lv_obj_set_style_bg_color(card_0, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_radius(card_0, 32767, 0);
        lv_obj_set_flag(card_0, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(card_0, LV_OBJ_FLAG_EVENT_BUBBLE, true);

        lv_obj_t * container_4 = container_create(container_3);
        lv_obj_set_x(container_4, 2);
        lv_obj_set_y(container_4, 2);
        lv_obj_set_width(container_4, 28);
        lv_obj_set_height(container_4, 28);
        lv_obj_set_flag(container_4, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(container_4, LV_OBJ_FLAG_EVENT_BUBBLE, true);
        lv_obj_t * image_2 = image_create(container_4);
        lv_obj_set_x(image_2, 0);
        lv_obj_set_y(image_2, 0);
        lv_obj_set_width(image_2, 28);
        lv_obj_set_height(image_2, 28);
        lv_image_set_src(image_2, image_hugeicons_cancel_01_18077);
        lv_obj_set_flag(image_2, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_add_screen_create_event(container_3, LV_EVENT_CLICKED, today_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * image_3 = image_create(lv_obj_0);
        lv_obj_set_x(image_3, 246);
        lv_obj_set_y(image_3, 159);
        lv_obj_set_width(image_3, 24);
        lv_obj_set_height(image_3, 24);
        lv_image_set_src(image_3, image_bitcoin_icons_edit_filled_176222);
        lv_obj_set_flag(image_3, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * container_5 = container_create(lv_obj_0);
        lv_obj_set_x(container_5, 116);
        lv_obj_set_y(container_5, 282);
        lv_obj_set_width(container_5, 129);
        lv_obj_set_height(container_5, 40);
        lv_obj_t * container_6 = container_create(container_5);
        lv_obj_set_x(container_6, 0);
        lv_obj_set_y(container_6, 0);
        lv_obj_set_width(container_6, 129);
        lv_obj_set_height(container_6, 40);
        lv_obj_set_flag(container_6, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(container_6, LV_OBJ_FLAG_EVENT_BUBBLE, true);
        lv_obj_t * card_1 = card_create(container_6);
        lv_obj_set_x(card_1, 0);
        lv_obj_set_y(card_1, 0);
        lv_obj_set_width(card_1, 129);
        lv_obj_set_height(card_1, 40);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_1, 63, 0);
        lv_obj_set_flag(card_1, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(card_1, LV_OBJ_FLAG_EVENT_BUBBLE, true);

        lv_obj_t * label_2 = label_create(container_5);
        lv_obj_set_x(label_2, 13);
        lv_obj_set_y(label_2, 14);
        lv_obj_set_height(label_2, 15);
        lv_label_set_text(label_2, "EMPEZAR");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_medium_16, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(label_2, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_add_screen_create_event(container_5, LV_EVENT_CLICKED, add_task_p1_c_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * container_7 = container_create(lv_obj_0);
        lv_obj_set_x(container_7, 111);
        lv_obj_set_y(container_7, 211);
        lv_obj_set_width(container_7, 125);
        lv_obj_set_height(container_7, 46);
        lv_obj_t * card_2 = card_create(container_7);
        lv_obj_set_x(card_2, 38);
        lv_obj_set_y(card_2, 0);
        lv_obj_set_width(card_2, 39);
        lv_obj_set_height(card_2, 46);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_2, 10, 0);

        lv_obj_t * card_3 = card_create(container_7);
        lv_obj_set_x(card_3, 86);
        lv_obj_set_y(card_3, 0);
        lv_obj_set_width(card_3, 39);
        lv_obj_set_height(card_3, 46);
        lv_obj_set_style_bg_color(card_3, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_3, 10, 0);

        lv_obj_t * image_4 = image_create(container_7);
        lv_obj_set_x(image_4, 0);
        lv_obj_set_y(image_4, 12);
        lv_obj_set_width(image_4, 22);
        lv_obj_set_height(image_4, 22);
        lv_image_set_src(image_4, image_vec_199_671);

        lv_obj_t * image_5 = image_create(container_7);
        lv_obj_set_x(image_5, 10);
        lv_obj_set_y(image_5, 18);
        lv_obj_set_width(image_5, 7);
        lv_obj_set_height(image_5, 7);
        lv_image_set_src(image_5, image_vec_199_672);

        lv_obj_t * card_4 = card_create(container_7);
        lv_obj_set_x(card_4, 79);
        lv_obj_set_y(card_4, 12);
        lv_obj_set_width(card_4, 6);
        lv_obj_set_height(card_4, 6);
        lv_obj_set_style_bg_color(card_4, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_4, 32767, 0);

        lv_obj_t * card_5 = card_create(container_7);
        lv_obj_set_x(card_5, 79);
        lv_obj_set_y(card_5, 29);
        lv_obj_set_width(card_5, 6);
        lv_obj_set_height(card_5, 6);
        lv_obj_set_style_bg_color(card_5, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_5, 32767, 0);

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

