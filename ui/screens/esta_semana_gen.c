/**
 * @file esta_semana_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "esta_semana_gen.h"
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

lv_obj_t * esta_semana_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "esta_semana_#");
        lv_obj_set_style_bg_color(lv_obj_0, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(lv_obj_0, 10, 0);
        lv_obj_set_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_set_style_pad_all(lv_obj_0, 0, 0);

        lv_obj_add_style(lv_obj_0, &style_screen_base_global, 0);
        lv_obj_t * panel_0 = panel_create(lv_obj_0);
        lv_obj_set_align(panel_0, LV_ALIGN_CENTER);
        lv_obj_set_x(panel_0, -4);
        lv_obj_set_y(panel_0, 0);
        lv_obj_set_width(panel_0, 360);
        lv_obj_set_height(panel_0, 360);
        lv_obj_set_style_bg_color(panel_0, lv_color_hex(0x000000), 0);

        lv_obj_t * container_0 = container_create(lv_obj_0);
        lv_obj_set_align(container_0, LV_ALIGN_CENTER);
        lv_obj_set_x(container_0, -4);
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
        lv_obj_set_x(card_0, -3);
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
        lv_obj_set_x(label_0, -3);
        lv_obj_set_y(label_0, 187);
        lv_obj_set_height(label_0, 28);
        lv_label_set_text(label_0, "12-18 de octubre");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_align(label_1, LV_ALIGN_CENTER);
        lv_obj_set_x(label_1, -5);
        lv_obj_set_y(label_1, -18);
        lv_obj_set_width(label_1, 76);
        lv_obj_set_height(label_1, 48);
        lv_label_set_text(label_1, "Esta\nSemana");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_line_space(label_1, 1, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(label_1, LV_TEXT_ALIGN_CENTER, 0);

        lv_obj_t * container_2 = container_create(lv_obj_0);
        lv_obj_set_align(container_2, LV_ALIGN_CENTER);
        lv_obj_set_x(container_2, -3);
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
        lv_obj_set_x(container_3, -5);
        lv_obj_set_y(container_3, 3);
        lv_obj_set_width(container_3, 249);
        lv_obj_set_height(container_3, 248);
        lv_obj_set_style_border_color(container_3, lv_color_hex(0x77B7DB), 0);
        lv_obj_set_style_radius(container_3, 32767, 0);
        lv_obj_set_style_border_opa(container_3, 255, 0);
        lv_obj_set_style_clip_corner(container_3, false, 0);
        lv_obj_t * card_1 = card_create(lv_obj_0);
        lv_obj_set_x(card_1, 161);
        lv_obj_set_y(card_1, 44);
        lv_obj_set_width(card_1, 30);
        lv_obj_set_height(card_1, 30);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0x1B9E86), 0);
        lv_obj_set_style_radius(card_1, 32767, 0);

        lv_obj_t * card_2 = card_create(lv_obj_0);
        lv_obj_set_x(card_2, 52);
        lv_obj_set_y(card_2, 229);
        lv_obj_set_width(card_2, 30);
        lv_obj_set_height(card_2, 30);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0x000000), 0);
        lv_obj_set_style_border_color(card_2, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(card_2, 7, 0);
        lv_obj_set_style_radius(card_2, 32767, 0);

        lv_obj_t * card_3 = card_create(lv_obj_0);
        lv_obj_set_x(card_3, 82);
        lv_obj_set_y(card_3, 263);
        lv_obj_set_width(card_3, 30);
        lv_obj_set_height(card_3, 30);
        lv_obj_set_style_bg_color(card_3, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_radius(card_3, 32767, 0);

        lv_obj_t * card_4 = card_create(lv_obj_0);
        lv_obj_set_x(card_4, 116);
        lv_obj_set_y(card_4, 53);
        lv_obj_set_width(card_4, 30);
        lv_obj_set_height(card_4, 30);
        lv_obj_set_style_bg_color(card_4, lv_color_hex(0x9AA5C2), 0);
        lv_obj_set_style_radius(card_4, 32767, 0);

        lv_obj_t * card_5 = card_create(lv_obj_0);
        lv_obj_set_x(card_5, 76);
        lv_obj_set_y(card_5, 74);
        lv_obj_set_width(card_5, 30);
        lv_obj_set_height(card_5, 30);
        lv_obj_set_style_bg_color(card_5, lv_color_hex(0xB1FFE9), 0);
        lv_obj_set_style_radius(card_5, 32767, 0);

        lv_obj_t * card_6 = card_create(lv_obj_0);
        lv_obj_set_x(card_6, 50);
        lv_obj_set_y(card_6, 107);
        lv_obj_set_width(card_6, 30);
        lv_obj_set_height(card_6, 30);
        lv_obj_set_style_bg_color(card_6, lv_color_hex(0x699767), 0);
        lv_obj_set_style_radius(card_6, 32767, 0);

        lv_obj_t * card_7 = card_create(lv_obj_0);
        lv_obj_set_x(card_7, 35);
        lv_obj_set_y(card_7, 147);
        lv_obj_set_width(card_7, 30);
        lv_obj_set_height(card_7, 30);
        lv_obj_set_style_bg_color(card_7, lv_color_hex(0xD0E4BF), 0);
        lv_obj_set_style_radius(card_7, 32767, 0);

        lv_obj_t * card_8 = card_create(lv_obj_0);
        lv_obj_set_x(card_8, 35);
        lv_obj_set_y(card_8, 189);
        lv_obj_set_width(card_8, 30);
        lv_obj_set_height(card_8, 30);
        lv_obj_set_style_bg_color(card_8, lv_color_hex(0x77B7DB), 0);
        lv_obj_set_style_radius(card_8, 32767, 0);

        lv_obj_t * container_4 = container_create(lv_obj_0);
        lv_obj_set_align(container_4, LV_ALIGN_CENTER);
        lv_obj_set_x(container_4, -4);
        lv_obj_set_y(container_4, 0);
        lv_obj_set_width(container_4, 360);
        lv_obj_set_height(container_4, 360);
        lv_obj_set_style_outline_color(container_4, lv_color_hex(0x4A4A4A), 0);
        lv_obj_set_style_outline_opa(container_4, 255, 0);
        lv_obj_set_style_outline_width(container_4, 4, 0);
        lv_obj_set_style_radius(container_4, 32767, 0);
        lv_obj_set_style_clip_corner(container_4, true, 0);
        lv_obj_t * label_2 = label_create(lv_obj_0);
        lv_obj_set_x(label_2, 161);
        lv_obj_set_y(label_2, 74);
        lv_obj_set_height(label_2, 30);
        lv_label_set_text(label_2, "Web Design");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_3 = label_create(lv_obj_0);
        lv_obj_set_x(label_3, 118);
        lv_obj_set_y(label_3, 81);
        lv_obj_set_height(label_3, 30);
        lv_label_set_text(label_3, "Practicum 1");
        lv_obj_set_style_text_font(label_3, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_3, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_4 = label_create(lv_obj_0);
        lv_obj_set_x(label_4, 85);
        lv_obj_set_y(label_4, 104);
        lv_obj_set_height(label_4, 30);
        lv_label_set_text(label_4, "Practicum 2");
        lv_obj_set_style_text_font(label_4, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_4, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_5 = label_create(lv_obj_0);
        lv_obj_set_x(label_5, 67);
        lv_obj_set_y(label_5, 134);
        lv_obj_set_height(label_5, 30);
        lv_label_set_text(label_5, "Web Design");
        lv_obj_set_style_text_font(label_5, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_5, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_6 = label_create(lv_obj_0);
        lv_obj_set_x(label_6, 57);
        lv_obj_set_y(label_6, 174);
        lv_obj_set_height(label_6, 30);
        lv_label_set_text(label_6, "Web Design");
        lv_obj_set_style_text_font(label_6, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_6, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_7 = label_create(lv_obj_0);
        lv_obj_set_x(label_7, 65);
        lv_obj_set_y(label_7, 211);
        lv_obj_set_height(label_7, 30);
        lv_label_set_text(label_7, "Web Design");
        lv_obj_set_style_text_font(label_7, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_7, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_8 = label_create(lv_obj_0);
        lv_obj_set_x(label_8, 84);
        lv_obj_set_y(label_8, 246);
        lv_obj_set_height(label_8, 30);
        lv_label_set_text(label_8, "Web Design");
        lv_obj_set_style_text_font(label_8, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_8, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_9 = label_create(lv_obj_0);
        lv_obj_set_x(label_9, 115);
        lv_obj_set_y(label_9, 276);
        lv_obj_set_height(label_9, 30);
        lv_label_set_text(label_9, "Web Design");
        lv_obj_set_style_text_font(label_9, font_instrument_sans_regular_6, 0);
        lv_obj_set_style_text_color(label_9, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * image_0 = image_create(lv_obj_0);
        lv_obj_set_x(image_0, 164);
        lv_obj_set_y(image_0, 204);
        lv_obj_set_width(image_0, 25);
        lv_obj_set_height(image_0, 25);
        lv_image_set_src(image_0, image_ant_design_calendar_filled);
        lv_obj_set_flag(image_0, LV_OBJ_FLAG_SCROLLABLE, false);

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

