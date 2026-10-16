/**
 * @file add_task_p3_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "add_task_p3_gen.h"
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

lv_obj_t * add_task_p3_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "add_task_p3_#");
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
        lv_obj_t * label_0 = label_create(lv_obj_0);
        lv_obj_set_align(label_0, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_0, -23);
        lv_obj_set_y(label_0, 93);
        lv_obj_set_height(label_0, 28);
        lv_label_set_text(label_0, "Crear fechas límite a partir del:");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * container_3 = container_create(lv_obj_0);
        lv_obj_set_x(container_3, 162);
        lv_obj_set_y(container_3, 296);
        lv_obj_set_width(container_3, 36);
        lv_obj_set_height(container_3, 37);
        lv_obj_t * card_0 = card_create(container_3);
        lv_obj_set_x(card_0, 0);
        lv_obj_set_y(card_0, 0);
        lv_obj_set_width(card_0, 36);
        lv_obj_set_height(card_0, 37);
        lv_obj_set_style_bg_color(card_0, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_radius(card_0, 32767, 0);
        lv_obj_set_flag(card_0, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(card_0, LV_OBJ_FLAG_EVENT_BUBBLE, true);

        lv_obj_t * image_0 = image_create(container_3);
        lv_obj_set_x(image_0, 2);
        lv_obj_set_y(image_0, 2);
        lv_obj_set_width(image_0, 33);
        lv_obj_set_height(image_0, 33);
        lv_image_set_src(image_0, image_akar_icons_arrow_down_176161);
        lv_obj_set_flag(image_0, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_add_screen_create_event(container_3, LV_EVENT_CLICKED, add_task_p4_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * container_4 = container_create(lv_obj_0);
        lv_obj_set_x(container_4, 162);
        lv_obj_set_y(container_4, 29);
        lv_obj_set_width(container_4, 36);
        lv_obj_set_height(container_4, 37);
        lv_obj_set_style_transform_rotation(container_4, -1800, 0);
        lv_obj_set_style_transform_pivot_x(container_4, 18, 0);
        lv_obj_set_style_transform_pivot_y(container_4, 19, 0);
        lv_obj_t * card_1 = card_create(container_4);
        lv_obj_set_x(card_1, 0);
        lv_obj_set_y(card_1, 0);
        lv_obj_set_width(card_1, 36);
        lv_obj_set_height(card_1, 37);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_transform_rotation(card_1, -1800, 0);
        lv_obj_set_style_transform_pivot_x(card_1, 18, 0);
        lv_obj_set_style_transform_pivot_y(card_1, 19, 0);
        lv_obj_set_style_radius(card_1, 32767, 0);
        lv_obj_set_flag(card_1, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(card_1, LV_OBJ_FLAG_EVENT_BUBBLE, true);

        lv_obj_t * image_1 = image_create(container_4);
        lv_obj_set_x(image_1, -2);
        lv_obj_set_y(image_1, -2);
        lv_obj_set_width(image_1, 33);
        lv_obj_set_height(image_1, 33);
        lv_image_set_src(image_1, image_akar_icons_arrow_down_176157);
        lv_obj_set_style_transform_rotation(image_1, -1800, 0);
        lv_obj_set_style_transform_pivot_x(image_1, 17, 0);
        lv_obj_set_style_transform_pivot_y(image_1, 17, 0);
        lv_obj_set_flag(image_1, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_add_screen_create_event(container_4, LV_EVENT_CLICKED, add_task_p2_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * container_5 = container_create(lv_obj_0);
        lv_obj_set_x(container_5, 79);
        lv_obj_set_y(container_5, 119);
        lv_obj_set_width(container_5, 201);
        lv_obj_set_height(container_5, 60);
        lv_obj_t * card_2 = card_create(container_5);
        lv_obj_set_x(card_2, 0);
        lv_obj_set_y(card_2, 24);
        lv_obj_set_width(card_2, 112);
        lv_obj_set_height(card_2, 36);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_2, 10, 0);

        lv_obj_t * image_2 = image_create(container_5);
        lv_obj_set_x(image_2, 6);
        lv_obj_set_y(image_2, 34);
        lv_obj_set_width(image_2, 19);
        lv_obj_set_height(image_2, 19);
        lv_image_set_src(image_2, image_ant_design_calendar_filled_176169);
        lv_obj_set_flag(image_2, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * label_1 = label_create(container_5);
        lv_obj_set_x(label_1, 6);
        lv_obj_set_y(label_1, 0);
        lv_obj_set_width(label_1, 30);
        lv_obj_set_height(label_1, 33);
        lv_label_set_text(label_1, "Inicio");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_line_space(label_1, 16, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * container_6 = container_create(container_5);
        lv_obj_set_x(container_6, 123);
        lv_obj_set_y(container_6, 26);
        lv_obj_set_width(container_6, 77);
        lv_obj_set_height(container_6, 29);
        lv_obj_t * card_3 = card_create(container_6);
        lv_obj_set_x(card_3, 24);
        lv_obj_set_y(card_3, 0);
        lv_obj_set_width(card_3, 24);
        lv_obj_set_height(card_3, 29);
        lv_obj_set_style_bg_color(card_3, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_3, 10, 0);

        lv_obj_t * card_4 = card_create(container_6);
        lv_obj_set_x(card_4, 53);
        lv_obj_set_y(card_4, 0);
        lv_obj_set_width(card_4, 24);
        lv_obj_set_height(card_4, 29);
        lv_obj_set_style_bg_color(card_4, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_4, 10, 0);

        lv_obj_t * image_3 = image_create(container_6);
        lv_obj_set_x(image_3, 0);
        lv_obj_set_y(image_3, 7);
        lv_obj_set_width(image_3, 14);
        lv_obj_set_height(image_3, 14);
        lv_image_set_src(image_3, image_vec_176_179);

        lv_obj_t * image_4 = image_create(container_6);
        lv_obj_set_x(image_4, 6);
        lv_obj_set_y(image_4, 11);
        lv_obj_set_width(image_4, 4);
        lv_obj_set_height(image_4, 4);
        lv_image_set_src(image_4, image_vec_176_180);

        lv_obj_t * card_5 = card_create(container_6);
        lv_obj_set_x(card_5, 49);
        lv_obj_set_y(card_5, 7);
        lv_obj_set_width(card_5, 4);
        lv_obj_set_height(card_5, 4);
        lv_obj_set_style_bg_color(card_5, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_5, 32767, 0);

        lv_obj_t * card_6 = card_create(container_6);
        lv_obj_set_x(card_6, 49);
        lv_obj_set_y(card_6, 18);
        lv_obj_set_width(card_6, 4);
        lv_obj_set_height(card_6, 4);
        lv_obj_set_style_bg_color(card_6, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_6, 32767, 0);

        lv_obj_t * container_7 = container_create(lv_obj_0);
        lv_obj_set_x(container_7, 79);
        lv_obj_set_y(container_7, 201);
        lv_obj_set_width(container_7, 201);
        lv_obj_set_height(container_7, 60);
        lv_obj_t * card_7 = card_create(container_7);
        lv_obj_set_x(card_7, 148);
        lv_obj_set_y(card_7, 28);
        lv_obj_set_width(card_7, 24);
        lv_obj_set_height(card_7, 29);
        lv_obj_set_style_bg_color(card_7, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_7, 10, 0);

        lv_obj_t * card_8 = card_create(container_7);
        lv_obj_set_x(card_8, 177);
        lv_obj_set_y(card_8, 28);
        lv_obj_set_width(card_8, 24);
        lv_obj_set_height(card_8, 29);
        lv_obj_set_style_bg_color(card_8, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_8, 10, 0);

        lv_obj_t * card_9 = card_create(container_7);
        lv_obj_set_x(card_9, 0);
        lv_obj_set_y(card_9, 24);
        lv_obj_set_width(card_9, 112);
        lv_obj_set_height(card_9, 36);
        lv_obj_set_style_bg_color(card_9, lv_color_hex(0x73979A), 0);
        lv_obj_set_style_radius(card_9, 10, 0);

        lv_obj_t * image_5 = image_create(container_7);
        lv_obj_set_x(image_5, 10);
        lv_obj_set_y(image_5, 32);
        lv_obj_set_width(image_5, 19);
        lv_obj_set_height(image_5, 19);
        lv_image_set_src(image_5, image_ant_design_calendar_filled_176169);
        lv_obj_set_flag(image_5, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * label_2 = label_create(container_7);
        lv_obj_set_x(label_2, 3);
        lv_obj_set_y(label_2, 0);
        lv_obj_set_width(label_2, 91);
        lv_obj_set_height(label_2, 33);
        lv_label_set_text(label_2, "Entrega Final");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_line_space(label_2, 16, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * image_6 = image_create(container_7);
        lv_obj_set_x(image_6, 124);
        lv_obj_set_y(image_6, 35);
        lv_obj_set_width(image_6, 14);
        lv_obj_set_height(image_6, 14);
        lv_image_set_src(image_6, image_vec_176_187);

        lv_obj_t * image_7 = image_create(container_7);
        lv_obj_set_x(image_7, 130);
        lv_obj_set_y(image_7, 38);
        lv_obj_set_width(image_7, 4);
        lv_obj_set_height(image_7, 4);
        lv_image_set_src(image_7, image_vec_176_188);

        lv_obj_t * card_10 = card_create(container_7);
        lv_obj_set_x(card_10, 173);
        lv_obj_set_y(card_10, 35);
        lv_obj_set_width(card_10, 4);
        lv_obj_set_height(card_10, 4);
        lv_obj_set_style_bg_color(card_10, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_10, 32767, 0);

        lv_obj_t * card_11 = card_create(container_7);
        lv_obj_set_x(card_11, 173);
        lv_obj_set_y(card_11, 46);
        lv_obj_set_width(card_11, 4);
        lv_obj_set_height(card_11, 4);
        lv_obj_set_style_bg_color(card_11, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_11, 32767, 0);

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

