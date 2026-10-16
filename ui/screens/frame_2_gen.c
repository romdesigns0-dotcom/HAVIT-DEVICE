/**
 * @file frame_2_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "frame_2_gen.h"
#include "../ui.h"

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

lv_obj_t * frame_2_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "frame_2_#");
        lv_obj_set_style_bg_color(lv_obj_0, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(lv_obj_0, 10, 0);
        lv_obj_set_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE, false);
        lv_obj_set_style_pad_all(lv_obj_0, 0, 0);

        lv_obj_add_style(lv_obj_0, &style_screen_base_global, 0);
        lv_obj_t * panel_0 = panel_create(lv_obj_0);
        lv_obj_set_x(panel_0, 0);
        lv_obj_set_y(panel_0, 0);
        lv_obj_set_width(panel_0, 360);
        lv_obj_set_height(panel_0, 360);
        lv_obj_set_style_bg_color(panel_0, lv_color_hex(0x161621), 0);

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
        lv_obj_set_align(label_0, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_0, 1);
        lv_obj_set_y(label_0, 143);
        lv_obj_set_height(label_0, 28);
        lv_label_set_text(label_0, "Inicia Sesión");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_22, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_0, 184, 0);

        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_align(label_1, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_1, 1);
        lv_obj_set_y(label_1, 171);
        lv_obj_set_height(label_1, 18);
        lv_label_set_text(label_1, "Tu próximo paso empieza aquí");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_15, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_1, 64, 0);

        lv_obj_t * container_1 = container_create(lv_obj_0);
        lv_obj_set_x(container_1, 75);
        lv_obj_set_y(container_1, 214);
        lv_obj_set_width(container_1, 210);
        lv_obj_set_height(container_1, 74);
        lv_obj_t * container_2 = container_create(container_1);
        lv_obj_set_x(container_2, 80);
        lv_obj_set_y(container_2, 0);
        lv_obj_set_width(container_2, 130);
        lv_obj_set_height(container_2, 74);
        lv_obj_t * image_0 = image_create(container_2);
        lv_obj_set_x(image_0, -5);
        lv_obj_set_y(image_0, -5);
        lv_obj_set_width(image_0, 30);
        lv_obj_set_height(image_0, 84);
        lv_image_set_src(image_0, image_vec_9_466);

        lv_obj_t * label_2 = label_create(container_2);
        lv_obj_set_x(label_2, 33);
        lv_obj_set_y(label_2, 10);
        lv_label_set_text(label_2, "Welcome");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_2, 128, 0);

        lv_obj_t * label_3 = label_create(container_2);
        lv_obj_set_x(label_3, 33);
        lv_obj_set_y(label_3, 34);
        lv_label_set_text(label_3, "Back");
        lv_obj_set_style_text_font(label_3, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_3, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_3, 128, 0);

        lv_obj_t * label_4 = label_create(container_2);
        lv_obj_set_x(label_4, 83);
        lv_obj_set_y(label_4, 34);
        lv_label_set_text(label_4, "ROM");
        lv_obj_set_style_text_font(label_4, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_4, lv_color_hex(0x5250F3), 0);

        lv_obj_t * container_3 = container_create(container_1);
        lv_obj_set_x(container_3, 0);
        lv_obj_set_y(container_3, 2);
        lv_obj_set_width(container_3, 68);
        lv_obj_set_height(container_3, 68);
        lv_obj_t * card_0 = card_create(container_3);
        lv_obj_set_x(card_0, 0);
        lv_obj_set_y(card_0, 0);
        lv_obj_set_width(card_0, 68);
        lv_obj_set_height(card_0, 68);
        lv_obj_set_style_bg_color(card_0, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_0, 32767, 0);

        lv_obj_t * image_1 = image_create(container_3);
        lv_obj_set_x(image_1, 10);
        lv_obj_set_y(image_1, 10);
        lv_obj_set_width(image_1, 47);
        lv_obj_set_height(image_1, 48);
        lv_image_set_src(image_1, image_google_logo_1);

        lv_obj_t * image_2 = image_create(lv_obj_0);
        lv_obj_set_x(image_2, 10);
        lv_obj_set_y(image_2, 10);
        lv_obj_set_width(image_2, 340);
        lv_obj_set_height(image_2, 340);
        lv_image_set_src(image_2, image_gradient_ring_f2);

        lv_obj_t * container_4 = container_create(lv_obj_0);
        lv_obj_set_x(container_4, 126);
        lv_obj_set_y(container_4, 37);
        lv_obj_set_width(container_4, 107);
        lv_obj_set_height(container_4, 83);
        lv_obj_t * card_1 = card_create(container_4);
        lv_obj_set_x(card_1, 77);
        lv_obj_set_y(card_1, 1);
        lv_obj_set_width(card_1, 27);
        lv_obj_set_height(card_1, 81);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0x1A1A24), 0);
        lv_obj_set_style_radius(card_1, 999, 0);
        lv_obj_set_style_transform_rotation(card_1, 50, 0);
        lv_obj_set_style_transform_pivot_x(card_1, 14, 0);
        lv_obj_set_style_transform_pivot_y(card_1, 41, 0);

        lv_obj_t * card_2 = card_create(container_4);
        lv_obj_set_x(card_2, 4);
        lv_obj_set_y(card_2, 1);
        lv_obj_set_width(card_2, 27);
        lv_obj_set_height(card_2, 81);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0x1A1A24), 0);
        lv_obj_set_style_radius(card_2, 999, 0);
        lv_obj_set_style_transform_rotation(card_2, 1750, 0);
        lv_obj_set_style_transform_pivot_x(card_2, 14, 0);
        lv_obj_set_style_transform_pivot_y(card_2, 41, 0);

        the_root = lv_obj_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

