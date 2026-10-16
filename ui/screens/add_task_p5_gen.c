/**
 * @file add_task_p5_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "add_task_p5_gen.h"
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

lv_obj_t * add_task_p5_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "add_task_p5_#");
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
        lv_obj_set_x(label_0, 70);
        lv_obj_set_y(label_0, 35);
        lv_obj_set_width(label_0, 220);
        lv_obj_set_height(label_0, 36);
        lv_label_set_text(label_0, "Línea de tiempo\nFinal Research Paper");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_13, 0);
        lv_obj_set_style_text_line_space(label_0, 2, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(label_0, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_x(label_1, 52);
        lv_obj_set_y(label_1, 70);
        lv_obj_set_width(label_1, 256);
        lv_obj_set_height(label_1, 16);
        lv_label_set_text(label_1, "Seminario de Investigación de Diseño");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_11, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(label_1, LV_TEXT_ALIGN_CENTER, 0);

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

        lv_obj_add_screen_create_event(container_3, LV_EVENT_CLICKED, desglose_b_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * container_4 = container_create(lv_obj_0);
        lv_obj_set_x(container_4, 78);
        lv_obj_set_y(container_4, 92);
        lv_obj_set_width(container_4, 246);
        lv_obj_set_height(container_4, 195);
        lv_obj_t * container_5 = container_create(container_4);
        lv_obj_set_x(container_5, 32);
        lv_obj_set_y(container_5, 0);
        lv_obj_set_width(container_5, 160);
        lv_obj_set_height(container_5, 28);
        lv_obj_t * label_2 = label_create(container_5);
        lv_obj_set_x(label_2, 0);
        lv_obj_set_y(label_2, 0);
        lv_obj_set_height(label_2, 28);
        lv_label_set_text(label_2, "octubre");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_2, 184, 0);

        lv_obj_t * label_3 = label_create(container_5);
        lv_obj_set_x(label_3, 103);
        lv_obj_set_y(label_3, 9);
        lv_obj_set_height(label_3, 15);
        lv_label_set_text(label_3, "2026");
        lv_obj_set_style_text_font(label_3, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_3, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_3, 64, 0);

        lv_obj_t * container_6 = container_create(container_4);
        lv_obj_set_flex_flow(container_6, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_flex_main_place(container_6, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_flex_cross_place(container_6, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_flex_track_place(container_6, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_all(container_6, 8, 0);
        lv_obj_set_style_pad_column(container_6, 0, 0);
        lv_obj_set_x(container_6, 219);
        lv_obj_set_y(container_6, 89);
        lv_obj_set_width(container_6, 27);
        lv_obj_set_height(container_6, 27);
        lv_obj_set_style_radius(container_6, 26843500, 0);
        lv_obj_set_style_clip_corner(container_6, true, 0);
        lv_obj_t * column_0 = column_create(container_6, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_width(column_0, LV_SIZE_CONTENT);
        lv_obj_set_height(column_0, LV_SIZE_CONTENT);
        lv_obj_set_flag(column_0, LV_OBJ_FLAG_CLICKABLE, false);
        lv_obj_set_flag(column_0, LV_OBJ_FLAG_EVENT_BUBBLE, true);
        lv_obj_t * image_1 = image_create(column_0);
        lv_obj_set_width(image_1, 16);
        lv_obj_set_height(image_1, 16);
        lv_image_set_src(image_1, image_icon_182108);
        lv_obj_set_flag(image_1, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_add_screen_create_event(container_6, LV_EVENT_CLICKED, add_task_p5_b_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * column_1 = column_create(container_4, 0, 0, 0, 0, 0, 1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_1, 0);
        lv_obj_set_y(column_1, 36);
        lv_obj_set_width(column_1, 26);
        lv_obj_set_height(column_1, 13);
        lv_obj_t * label_4 = label_create(column_1);
        lv_label_set_text(label_4, "L");
        lv_obj_set_style_text_font(label_4, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_4, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_4, 64, 0);
        lv_obj_set_style_text_align(label_4, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_2 = column_create(container_4, 0, 0, 0, 0, 0, 1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_2, 30);
        lv_obj_set_y(column_2, 36);
        lv_obj_set_width(column_2, 26);
        lv_obj_set_height(column_2, 13);
        lv_obj_t * label_5 = label_create(column_2);
        lv_label_set_text(label_5, "M");
        lv_obj_set_style_text_font(label_5, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_5, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_5, 64, 0);
        lv_obj_set_style_text_align(label_5, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_3 = column_create(container_4, 0, 0, 0, 0, 0, 1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_3, 59);
        lv_obj_set_y(column_3, 36);
        lv_obj_set_width(column_3, 26);
        lv_obj_set_height(column_3, 13);
        lv_obj_t * label_6 = label_create(column_3);
        lv_label_set_text(label_6, "X");
        lv_obj_set_style_text_font(label_6, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_6, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_6, 64, 0);
        lv_obj_set_style_text_align(label_6, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_4 = column_create(container_4, 0, 0, 0, 0, 0, 1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_4, 89);
        lv_obj_set_y(column_4, 36);
        lv_obj_set_width(column_4, 26);
        lv_obj_set_height(column_4, 13);
        lv_obj_t * label_7 = label_create(column_4);
        lv_label_set_text(label_7, "J");
        lv_obj_set_style_text_font(label_7, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_7, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_7, 64, 0);
        lv_obj_set_style_text_align(label_7, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_5 = column_create(container_4, 0, 0, 0, 0, 0, 1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_5, 118);
        lv_obj_set_y(column_5, 36);
        lv_obj_set_width(column_5, 26);
        lv_obj_set_height(column_5, 13);
        lv_obj_t * label_8 = label_create(column_5);
        lv_label_set_text(label_8, "V");
        lv_obj_set_style_text_font(label_8, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_8, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_8, 64, 0);
        lv_obj_set_style_text_align(label_8, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_6 = column_create(container_4, 0, 0, 0, 0, 0, 1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_6, 148);
        lv_obj_set_y(column_6, 36);
        lv_obj_set_width(column_6, 26);
        lv_obj_set_height(column_6, 13);
        lv_obj_t * label_9 = label_create(column_6);
        lv_label_set_text(label_9, "S");
        lv_obj_set_style_text_font(label_9, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_9, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_9, 64, 0);
        lv_obj_set_style_text_align(label_9, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_7 = column_create(container_4, 0, 0, 0, 0, 0, 1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_7, 178);
        lv_obj_set_y(column_7, 36);
        lv_obj_set_width(column_7, 26);
        lv_obj_set_height(column_7, 13);
        lv_obj_t * label_10 = label_create(column_7);
        lv_label_set_text(label_10, "D");
        lv_obj_set_style_text_font(label_10, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_10, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_10, 64, 0);
        lv_obj_set_style_text_align(label_10, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * container_7 = container_create(container_4);
        lv_obj_set_x(container_7, 0);
        lv_obj_set_y(container_7, 56);
        lv_obj_set_width(container_7, 26);
        lv_obj_set_height(container_7, 26);

        lv_obj_t * container_8 = container_create(container_4);
        lv_obj_set_x(container_8, 30);
        lv_obj_set_y(container_8, 56);
        lv_obj_set_width(container_8, 26);
        lv_obj_set_height(container_8, 26);

        lv_obj_t * container_9 = container_create(container_4);
        lv_obj_set_x(container_9, 59);
        lv_obj_set_y(container_9, 56);
        lv_obj_set_width(container_9, 26);
        lv_obj_set_height(container_9, 26);

        lv_obj_t * column_8 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_8, 99);
        lv_obj_set_y(column_8, 58);
        lv_obj_set_width(column_8, 6);
        lv_obj_set_height(column_8, 21);
        lv_obj_t * label_11 = label_create(column_8);
        lv_label_set_text(label_11, "1");
        lv_obj_set_style_text_font(label_11, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_11, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_11, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_9 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_9, 127);
        lv_obj_set_y(column_9, 58);
        lv_obj_set_width(column_9, 8);
        lv_obj_set_height(column_9, 21);
        lv_obj_t * label_12 = label_create(column_9);
        lv_label_set_text(label_12, "2");
        lv_obj_set_style_text_font(label_12, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_12, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_12, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_10 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_10, 157);
        lv_obj_set_y(column_10, 58);
        lv_obj_set_width(column_10, 9);
        lv_obj_set_height(column_10, 21);
        lv_obj_t * label_13 = label_create(column_10);
        lv_label_set_text(label_13, "3");
        lv_obj_set_style_text_font(label_13, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_13, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_13, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_11 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_11, 186);
        lv_obj_set_y(column_11, 58);
        lv_obj_set_width(column_11, 9);
        lv_obj_set_height(column_11, 21);
        lv_obj_t * label_14 = label_create(column_11);
        lv_label_set_text(label_14, "4");
        lv_obj_set_style_text_font(label_14, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_14, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_14, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_12 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_12, 9);
        lv_obj_set_y(column_12, 88);
        lv_obj_set_width(column_12, 9);
        lv_obj_set_height(column_12, 21);
        lv_obj_t * label_15 = label_create(column_12);
        lv_label_set_text(label_15, "5");
        lv_obj_set_style_text_font(label_15, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_15, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_15, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_13 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_13, 38);
        lv_obj_set_y(column_13, 88);
        lv_obj_set_width(column_13, 9);
        lv_obj_set_height(column_13, 21);
        lv_obj_t * label_16 = label_create(column_13);
        lv_label_set_text(label_16, "6");
        lv_obj_set_style_text_font(label_16, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_16, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_16, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_14 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_14, 68);
        lv_obj_set_y(column_14, 88);
        lv_obj_set_width(column_14, 8);
        lv_obj_set_height(column_14, 21);
        lv_obj_t * label_17 = label_create(column_14);
        lv_label_set_text(label_17, "7");
        lv_obj_set_style_text_font(label_17, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_17, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_17, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_15 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_15, 97);
        lv_obj_set_y(column_15, 88);
        lv_obj_set_width(column_15, 9);
        lv_obj_set_height(column_15, 21);
        lv_obj_t * label_18 = label_create(column_15);
        lv_label_set_text(label_18, "8");
        lv_obj_set_style_text_font(label_18, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_18, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_18, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_16 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_16, 127);
        lv_obj_set_y(column_16, 88);
        lv_obj_set_width(column_16, 9);
        lv_obj_set_height(column_16, 21);
        lv_obj_t * label_19 = label_create(column_16);
        lv_label_set_text(label_19, "9");
        lv_obj_set_style_text_font(label_19, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_19, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_19, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_17 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_17, 154);
        lv_obj_set_y(column_17, 88);
        lv_obj_set_width(column_17, 15);
        lv_obj_set_height(column_17, 21);
        lv_obj_t * label_20 = label_create(column_17);
        lv_label_set_text(label_20, "10");
        lv_obj_set_style_text_font(label_20, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_20, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_20, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_18 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_18, 185);
        lv_obj_set_y(column_18, 88);
        lv_obj_set_width(column_18, 11);
        lv_obj_set_height(column_18, 21);
        lv_obj_t * label_21 = label_create(column_18);
        lv_label_set_text(label_21, "11");
        lv_obj_set_style_text_font(label_21, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_21, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_21, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_19 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_19, 6);
        lv_obj_set_y(column_19, 117);
        lv_obj_set_width(column_19, 13);
        lv_obj_set_height(column_19, 21);
        lv_obj_t * label_22 = label_create(column_19);
        lv_label_set_text(label_22, "12");
        lv_obj_set_style_text_font(label_22, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_22, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_22, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_20 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_20, 36);
        lv_obj_set_y(column_20, 117);
        lv_obj_set_width(column_20, 14);
        lv_obj_set_height(column_20, 21);
        lv_obj_t * label_23 = label_create(column_20);
        lv_label_set_text(label_23, "13");
        lv_obj_set_style_text_font(label_23, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_23, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_23, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_21 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_21, 65);
        lv_obj_set_y(column_21, 117);
        lv_obj_set_width(column_21, 14);
        lv_obj_set_height(column_21, 21);
        lv_obj_t * label_24 = label_create(column_21);
        lv_label_set_text(label_24, "14");
        lv_obj_set_style_text_font(label_24, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_24, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_24, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_22 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_22, 95);
        lv_obj_set_y(column_22, 117);
        lv_obj_set_width(column_22, 14);
        lv_obj_set_height(column_22, 21);
        lv_obj_t * label_25 = label_create(column_22);
        lv_label_set_text(label_25, "15");
        lv_obj_set_style_text_font(label_25, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_25, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_25, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_23 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_23, 124);
        lv_obj_set_y(column_23, 117);
        lv_obj_set_width(column_23, 14);
        lv_obj_set_height(column_23, 21);
        lv_obj_t * label_26 = label_create(column_23);
        lv_label_set_text(label_26, "16");
        lv_obj_set_style_text_font(label_26, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_26, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_26, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_24 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_24, 154);
        lv_obj_set_y(column_24, 117);
        lv_obj_set_width(column_24, 13);
        lv_obj_set_height(column_24, 21);
        lv_obj_t * label_27 = label_create(column_24);
        lv_label_set_text(label_27, "17");
        lv_obj_set_style_text_font(label_27, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_27, lv_color_hex(0x9594C8), 0);
        lv_obj_set_style_text_align(label_27, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_25 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_25, 184);
        lv_obj_set_y(column_25, 117);
        lv_obj_set_width(column_25, 14);
        lv_obj_set_height(column_25, 21);
        lv_obj_t * label_28 = label_create(column_25);
        lv_label_set_text(label_28, "18");
        lv_obj_set_style_text_font(label_28, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_28, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(label_28, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_26 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_26, 9);
        lv_obj_set_y(column_26, 146);
        lv_obj_set_width(column_26, 15);
        lv_obj_set_height(column_26, 21);
        lv_obj_t * label_29 = label_create(column_26);
        lv_label_set_text(label_29, "19");
        lv_obj_set_style_text_font(label_29, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_29, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_29, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_27 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_27, 38);
        lv_obj_set_y(column_27, 146);
        lv_obj_set_width(column_27, 17);
        lv_obj_set_height(column_27, 21);
        lv_obj_t * label_30 = label_create(column_27);
        lv_label_set_text(label_30, "20");
        lv_obj_set_style_text_font(label_30, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_30, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_30, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_28 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_28, 69);
        lv_obj_set_y(column_28, 146);
        lv_obj_set_width(column_28, 14);
        lv_obj_set_height(column_28, 21);
        lv_obj_t * label_31 = label_create(column_28);
        lv_label_set_text(label_31, "21");
        lv_obj_set_style_text_font(label_31, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_31, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_31, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_29 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_29, 98);
        lv_obj_set_y(column_29, 146);
        lv_obj_set_width(column_29, 15);
        lv_obj_set_height(column_29, 21);
        lv_obj_t * label_32 = label_create(column_29);
        lv_label_set_text(label_32, "22");
        lv_obj_set_style_text_font(label_32, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_32, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_32, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_30 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_30, 127);
        lv_obj_set_y(column_30, 146);
        lv_obj_set_width(column_30, 15);
        lv_obj_set_height(column_30, 21);
        lv_obj_t * label_33 = label_create(column_30);
        lv_label_set_text(label_33, "23");
        lv_obj_set_style_text_font(label_33, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_33, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_33, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_31 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_31, 153);
        lv_obj_set_y(column_31, 146);
        lv_obj_set_width(column_31, 15);
        lv_obj_set_height(column_31, 21);
        lv_obj_t * label_34 = label_create(column_31);
        lv_label_set_text(label_34, "24");
        lv_obj_set_style_text_font(label_34, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_34, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_34, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_32 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_32, 183);
        lv_obj_set_y(column_32, 146);
        lv_obj_set_width(column_32, 15);
        lv_obj_set_height(column_32, 21);
        lv_obj_t * label_35 = label_create(column_32);
        lv_label_set_text(label_35, "25");
        lv_obj_set_style_text_font(label_35, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_35, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_35, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_33 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_33, 5);
        lv_obj_set_y(column_33, 175);
        lv_obj_set_width(column_33, 16);
        lv_obj_set_height(column_33, 21);
        lv_obj_t * label_36 = label_create(column_33);
        lv_label_set_text(label_36, "26");
        lv_obj_set_style_text_font(label_36, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_36, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_36, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_34 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_34, 35);
        lv_obj_set_y(column_34, 175);
        lv_obj_set_width(column_34, 15);
        lv_obj_set_height(column_34, 21);
        lv_obj_t * label_37 = label_create(column_34);
        lv_label_set_text(label_37, "27");
        lv_obj_set_style_text_font(label_37, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_37, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_37, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_35 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_35, 64);
        lv_obj_set_y(column_35, 175);
        lv_obj_set_width(column_35, 16);
        lv_obj_set_height(column_35, 21);
        lv_obj_t * label_38 = label_create(column_35);
        lv_label_set_text(label_38, "28");
        lv_obj_set_style_text_font(label_38, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_38, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_38, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_36 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_36, 94);
        lv_obj_set_y(column_36, 175);
        lv_obj_set_width(column_36, 16);
        lv_obj_set_height(column_36, 21);
        lv_obj_t * label_39 = label_create(column_36);
        lv_label_set_text(label_39, "29");
        lv_obj_set_style_text_font(label_39, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_39, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_39, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_37 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_37, 122);
        lv_obj_set_y(column_37, 175);
        lv_obj_set_width(column_37, 18);
        lv_obj_set_height(column_37, 21);
        lv_obj_t * label_40 = label_create(column_37);
        lv_label_set_text(label_40, "30");
        lv_obj_set_style_text_font(label_40, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_40, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_40, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * column_38 = column_create(container_4, 0, 0, 0, 0, 0, 0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_x(column_38, 158);
        lv_obj_set_y(column_38, 175);
        lv_obj_set_width(column_38, 14);
        lv_obj_set_height(column_38, 21);
        lv_obj_t * label_41 = label_create(column_38);
        lv_label_set_text(label_41, "31");
        lv_obj_set_style_text_font(label_41, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_41, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_text_align(label_41, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * card_1 = card_create(lv_obj_0);
        lv_obj_set_x(card_1, 255);
        lv_obj_set_y(card_1, 206);
        lv_obj_set_width(card_1, 28);
        lv_obj_set_height(card_1, 28);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_bg_opa(card_1, 121, 0);
        lv_obj_set_style_radius(card_1, 32767, 0);

        lv_obj_t * card_2 = card_create(lv_obj_0);
        lv_obj_set_x(card_2, 199);
        lv_obj_set_y(card_2, 236);
        lv_obj_set_width(card_2, 28);
        lv_obj_set_height(card_2, 28);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_bg_opa(card_2, 121, 0);
        lv_obj_set_style_radius(card_2, 32767, 0);

        lv_obj_t * card_3 = card_create(lv_obj_0);
        lv_obj_set_x(card_3, 229);
        lv_obj_set_y(card_3, 265);
        lv_obj_set_width(card_3, 28);
        lv_obj_set_height(card_3, 28);
        lv_obj_set_style_bg_color(card_3, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_bg_opa(card_3, 121, 0);
        lv_obj_set_style_radius(card_3, 32767, 0);

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

