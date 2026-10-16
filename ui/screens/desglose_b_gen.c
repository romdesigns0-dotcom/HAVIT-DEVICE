/**
 * @file desglose_b_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "desglose_b_gen.h"
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

lv_obj_t * desglose_b_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "desglose_b_#");
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
        lv_obj_t * label_0 = label_create(lv_obj_0);
        lv_obj_set_align(label_0, LV_ALIGN_CENTER);
        lv_obj_set_x(label_0, 1);
        lv_obj_set_y(label_0, -131);
        lv_obj_set_height(label_0, 24);
        lv_label_set_text(label_0, "Desglose");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * container_1 = container_create(lv_obj_0);
        lv_obj_set_align(container_1, LV_ALIGN_TOP_MID);
        lv_obj_set_x(container_1, 0);
        lv_obj_set_y(container_1, 101);
        lv_obj_set_width(container_1, 268);
        lv_obj_set_height(container_1, 59);
        lv_obj_set_style_border_color(container_1, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_1, 255, 0);
        lv_obj_set_style_border_width(container_1, 2, 0);
        lv_obj_set_style_radius(container_1, 38, 0);
        lv_obj_set_style_clip_corner(container_1, true, 0);
        lv_obj_t * container_2 = container_create(lv_obj_0);
        lv_obj_set_align(container_2, LV_ALIGN_TOP_MID);
        lv_obj_set_x(container_2, 1);
        lv_obj_set_y(container_2, 239);
        lv_obj_set_width(container_2, 228);
        lv_obj_set_height(container_2, 48);
        lv_obj_set_style_border_color(container_2, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_2, 255, 0);
        lv_obj_set_style_border_width(container_2, 2, 0);
        lv_obj_set_style_radius(container_2, 38, 0);
        lv_obj_set_style_clip_corner(container_2, true, 0);
        lv_obj_t * container_3 = container_create(lv_obj_0);
        lv_obj_set_align(container_3, LV_ALIGN_TOP_MID);
        lv_obj_set_x(container_3, 1);
        lv_obj_set_y(container_3, 294);
        lv_obj_set_width(container_3, 138);
        lv_obj_set_height(container_3, 29);
        lv_obj_set_style_border_color(container_3, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_3, 255, 0);
        lv_obj_set_style_border_width(container_3, 2, 0);
        lv_obj_set_style_radius(container_3, 38, 0);
        lv_obj_set_style_clip_corner(container_3, true, 0);
        lv_obj_t * container_4 = container_create(lv_obj_0);
        lv_obj_set_align(container_4, LV_ALIGN_TOP_MID);
        lv_obj_set_x(container_4, 0);
        lv_obj_set_y(container_4, 170);
        lv_obj_set_width(container_4, 268);
        lv_obj_set_height(container_4, 56);
        lv_obj_set_style_border_color(container_4, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_4, 255, 0);
        lv_obj_set_style_border_width(container_4, 2, 0);
        lv_obj_set_style_radius(container_4, 64, 0);
        lv_obj_set_style_clip_corner(container_4, true, 0);
        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_x(label_1, 66);
        lv_obj_set_y(label_1, 104);
        lv_obj_set_height(label_1, 28);
        lv_label_set_text(label_1, "18 Oct - 23:59");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_16, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_2 = label_create(lv_obj_0);
        lv_obj_set_x(label_2, 62);
        lv_obj_set_y(label_2, 176);
        lv_obj_set_height(label_2, 28);
        lv_label_set_text(label_2, "23 Oct - 23:59");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_15, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_3 = label_create(lv_obj_0);
        lv_obj_set_x(label_3, 78);
        lv_obj_set_y(label_3, 245);
        lv_obj_set_height(label_3, 28);
        lv_label_set_text(label_3, "31 Oct - 23:59");
        lv_obj_set_style_text_font(label_3, font_instrument_sans_regular_14, 0);
        lv_obj_set_style_text_color(label_3, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_4 = label_create(lv_obj_0);
        lv_obj_set_x(label_4, 123);
        lv_obj_set_y(label_4, 291);
        lv_obj_set_height(label_4, 21);
        lv_label_set_text(label_4, "03 Nov - 23:59");
        lv_obj_set_style_text_font(label_4, font_instrument_sans_regular_10, 0);
        lv_obj_set_style_text_color(label_4, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_5 = label_create(lv_obj_0);
        lv_obj_set_x(label_5, 66);
        lv_obj_set_y(label_5, 127);
        lv_obj_set_height(label_5, 28);
        lv_label_set_text(label_5, "Entregar 5 fuentes académicas iniciales");
        lv_obj_set_style_text_font(label_5, font_instrument_sans_regular_11, 0);
        lv_obj_set_style_text_color(label_5, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_6 = label_create(lv_obj_0);
        lv_obj_set_x(label_6, 62);
        lv_obj_set_y(label_6, 198);
        lv_obj_set_width(label_6, 205);
        lv_obj_set_height(label_6, 26);
        lv_label_set_text(label_6, "Lista final de 15 fuentes y casos de estudio \ncompletada.");
        lv_obj_set_style_text_font(label_6, font_instrument_sans_regular_8, 0);
        lv_obj_set_style_text_line_space(label_6, 1, 0);
        lv_obj_set_style_text_color(label_6, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_7 = label_create(lv_obj_0);
        lv_obj_set_x(label_7, 84);
        lv_obj_set_y(label_7, 260);
        lv_obj_set_height(label_7, 28);
        lv_label_set_text(label_7, "Lectura y anotación de todas las fuentes finalizada.");
        lv_obj_set_style_text_font(label_7, font_instrument_sans_regular_7, 0);
        lv_obj_set_style_text_color(label_7, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * container_5 = container_create(lv_obj_0);
        lv_obj_set_align(container_5, LV_ALIGN_CENTER);
        lv_obj_set_x(container_5, 0);
        lv_obj_set_y(container_5, 0);
        lv_obj_set_width(container_5, 360);
        lv_obj_set_height(container_5, 360);
        lv_obj_set_style_outline_color(container_5, lv_color_hex(0x4A4A4A), 0);
        lv_obj_set_style_outline_opa(container_5, 255, 0);
        lv_obj_set_style_outline_width(container_5, 4, 0);
        lv_obj_set_style_radius(container_5, 32767, 0);
        lv_obj_set_style_clip_corner(container_5, true, 0);
        lv_obj_t * container_6 = container_create(lv_obj_0);
        lv_obj_set_align(container_6, LV_ALIGN_CENTER);
        lv_obj_set_x(container_6, 0);
        lv_obj_set_y(container_6, 0);
        lv_obj_set_width(container_6, 350);
        lv_obj_set_height(container_6, 350);
        lv_obj_set_style_border_color(container_6, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_6, 255, 0);
        lv_obj_set_style_border_width(container_6, 7, 0);
        lv_obj_set_style_transform_rotation(container_6, 1800, 0);
        lv_obj_set_style_transform_pivot_x(container_6, 175, 0);
        lv_obj_set_style_transform_pivot_y(container_6, 175, 0);
        lv_obj_set_style_radius(container_6, 32767, 0);
        lv_obj_set_style_clip_corner(container_6, true, 0);
        lv_obj_t * container_7 = container_create(lv_obj_0);
        lv_obj_set_x(container_7, 268);
        lv_obj_set_y(container_7, 116);
        lv_obj_set_width(container_7, 31);
        lv_obj_set_height(container_7, 31);
        lv_obj_t * container_8 = container_create(container_7);
        lv_obj_set_x(container_8, 0);
        lv_obj_set_y(container_8, 0);
        lv_obj_set_width(container_8, 31);
        lv_obj_set_height(container_8, 31);
        lv_obj_t * image_0 = image_create(container_8);
        lv_obj_set_x(image_0, 0);
        lv_obj_set_y(image_0, 0);
        lv_obj_set_width(image_0, 31);
        lv_obj_set_height(image_0, 31);
        lv_image_set_src(image_0, image_vec_96_7);

        lv_obj_t * image_1 = image_create(container_8);
        lv_obj_set_x(image_1, 8);
        lv_obj_set_y(image_1, 14);
        lv_obj_set_width(image_1, 15);
        lv_obj_set_height(image_1, 3);
        lv_image_set_src(image_1, image_vec_96_8);

        lv_obj_t * image_2 = image_create(container_8);
        lv_obj_set_x(image_2, 19);
        lv_obj_set_y(image_2, 10);
        lv_obj_set_width(image_2, 8);
        lv_obj_set_height(image_2, 12);
        lv_image_set_src(image_2, image_vec_96_9);

        lv_obj_t * container_9 = container_create(lv_obj_0);
        lv_obj_set_x(container_9, 268);
        lv_obj_set_y(container_9, 183);
        lv_obj_set_width(container_9, 31);
        lv_obj_set_height(container_9, 31);
        lv_obj_t * container_10 = container_create(container_9);
        lv_obj_set_x(container_10, 0);
        lv_obj_set_y(container_10, 0);
        lv_obj_set_width(container_10, 31);
        lv_obj_set_height(container_10, 31);
        lv_obj_t * image_3 = image_create(container_10);
        lv_obj_set_x(image_3, 0);
        lv_obj_set_y(image_3, 0);
        lv_obj_set_width(image_3, 31);
        lv_obj_set_height(image_3, 31);
        lv_image_set_src(image_3, image_vec_96_12);

        lv_obj_t * image_4 = image_create(container_10);
        lv_obj_set_x(image_4, 8);
        lv_obj_set_y(image_4, 14);
        lv_obj_set_width(image_4, 15);
        lv_obj_set_height(image_4, 3);
        lv_image_set_src(image_4, image_vec_96_13);

        lv_obj_t * image_5 = image_create(container_10);
        lv_obj_set_x(image_5, 19);
        lv_obj_set_y(image_5, 10);
        lv_obj_set_width(image_5, 8);
        lv_obj_set_height(image_5, 12);
        lv_image_set_src(image_5, image_vec_96_14);

        lv_obj_t * container_11 = container_create(lv_obj_0);
        lv_obj_set_x(container_11, 254);
        lv_obj_set_y(container_11, 249);
        lv_obj_set_width(container_11, 28);
        lv_obj_set_height(container_11, 28);
        lv_obj_t * container_12 = container_create(container_11);
        lv_obj_set_x(container_12, 0);
        lv_obj_set_y(container_12, 0);
        lv_obj_set_width(container_12, 28);
        lv_obj_set_height(container_12, 28);
        lv_obj_t * image_6 = image_create(container_12);
        lv_obj_set_x(image_6, 0);
        lv_obj_set_y(image_6, 0);
        lv_obj_set_width(image_6, 28);
        lv_obj_set_height(image_6, 28);
        lv_image_set_src(image_6, image_vec_96_17);

        lv_obj_t * image_7 = image_create(container_12);
        lv_obj_set_x(image_7, 7);
        lv_obj_set_y(image_7, 13);
        lv_obj_set_width(image_7, 14);
        lv_obj_set_height(image_7, 3);
        lv_image_set_src(image_7, image_vec_96_18);

        lv_obj_t * image_8 = image_create(container_12);
        lv_obj_set_x(image_8, 17);
        lv_obj_set_y(image_8, 8);
        lv_obj_set_width(image_8, 7);
        lv_obj_set_height(image_8, 11);
        lv_image_set_src(image_8, image_vec_96_19);

        lv_obj_t * container_13 = container_create(lv_obj_0);
        lv_obj_set_x(container_13, 229);
        lv_obj_set_y(container_13, 301);
        lv_obj_set_width(container_13, 15);
        lv_obj_set_height(container_13, 15);
        lv_obj_t * container_14 = container_create(container_13);
        lv_obj_set_x(container_14, 0);
        lv_obj_set_y(container_14, 0);
        lv_obj_set_width(container_14, 15);
        lv_obj_set_height(container_14, 15);
        lv_obj_t * image_9 = image_create(container_14);
        lv_obj_set_x(image_9, 0);
        lv_obj_set_y(image_9, 0);
        lv_obj_set_width(image_9, 15);
        lv_obj_set_height(image_9, 15);
        lv_image_set_src(image_9, image_vec_96_22);

        lv_obj_t * image_10 = image_create(container_14);
        lv_obj_set_x(image_10, 4);
        lv_obj_set_y(image_10, 7);
        lv_obj_set_width(image_10, 7);
        lv_obj_set_height(image_10, 1);
        lv_image_set_src(image_10, image_vec_96_23);

        lv_obj_t * image_11 = image_create(container_14);
        lv_obj_set_x(image_11, 9);
        lv_obj_set_y(image_11, 4);
        lv_obj_set_width(image_11, 4);
        lv_obj_set_height(image_11, 6);
        lv_image_set_src(image_11, image_vec_96_24);

        lv_obj_t * card_0 = card_create(lv_obj_0);
        lv_obj_set_x(card_0, 319);
        lv_obj_set_y(card_0, 137);
        lv_obj_set_width(card_0, 13);
        lv_obj_set_height(card_0, 39);
        lv_obj_set_style_bg_color(card_0, lv_color_hex(0x77B7DB), 0);
        lv_obj_set_style_radius(card_0, 39, 0);

        lv_obj_t * image_12 = image_create(lv_obj_0);
        lv_obj_set_x(image_12, 254);
        lv_obj_set_y(image_12, 61);
        lv_obj_set_width(image_12, 33);
        lv_obj_set_height(image_12, 33);
        lv_image_set_src(image_12, image_bitcoin_icons_edit_filled_182424);
        lv_obj_set_flag(image_12, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * label_8 = label_create(lv_obj_0);
        lv_obj_set_x(label_8, 124);
        lv_obj_set_y(label_8, 307);
        lv_obj_set_height(label_8, 12);
        lv_label_set_text(label_8, "Esquema de investigación");
        lv_obj_set_style_text_font(label_8, font_instrument_sans_regular_5, 0);
        lv_obj_set_style_text_color(label_8, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * button_subtle_0 = button_subtle_create(lv_obj_0);
        lv_obj_set_x(button_subtle_0, 67);
        lv_obj_set_y(button_subtle_0, 56);
        lv_obj_set_width(button_subtle_0, 39);
        lv_obj_set_height(button_subtle_0, 39);
        lv_obj_t * image_13 = image_create(button_subtle_0);
        lv_obj_set_x(image_13, 2);
        lv_obj_set_y(image_13, 2);
        lv_obj_set_width(image_13, 36);
        lv_obj_set_height(image_13, 36);
        lv_image_set_src(image_13, image_akar_icons_circle_check_fill_182427);

        lv_obj_add_screen_create_event(button_subtle_0, LV_EVENT_CLICKED, desglose_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

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

