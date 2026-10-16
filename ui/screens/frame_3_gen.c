/**
 * @file frame_3_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "frame_3_gen.h"
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

lv_obj_t * frame_3_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "frame_3_#");
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
        lv_obj_t * container_1 = container_create(lv_obj_0);
        lv_obj_set_x(container_1, 73);
        lv_obj_set_y(container_1, 154);
        lv_obj_set_width(container_1, 215);
        lv_obj_set_height(container_1, 55);
        lv_obj_t * label_0 = label_create(container_1);
        lv_obj_set_align(label_0, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_0, 0);
        lv_obj_set_y(label_0, 0);
        lv_obj_set_height(label_0, 28);
        lv_label_set_text(label_0, "Bienvenido de nuevo.");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_22, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_0, 184, 0);

        lv_obj_t * label_1 = label_create(container_1);
        lv_obj_set_align(label_1, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_1, 1);
        lv_obj_set_y(label_1, 28);
        lv_obj_set_height(label_1, 27);
        lv_label_set_text(label_1, "ROM.");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_22, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0x5250F3), 0);

        lv_obj_t * image_0 = image_create(lv_obj_0);
        lv_obj_set_x(image_0, 10);
        lv_obj_set_y(image_0, 10);
        lv_obj_set_width(image_0, 340);
        lv_obj_set_height(image_0, 340);
        lv_image_set_src(image_0, image_gradient_ring_f3);

        lv_obj_t * container_2 = container_create(lv_obj_0);
        lv_obj_set_x(container_2, 126);
        lv_obj_set_y(container_2, 37);
        lv_obj_set_width(container_2, 107);
        lv_obj_set_height(container_2, 83);
        lv_obj_t * card_0 = card_create(container_2);
        lv_obj_set_x(card_0, 77);
        lv_obj_set_y(card_0, 1);
        lv_obj_set_width(card_0, 27);
        lv_obj_set_height(card_0, 81);
        lv_obj_set_style_bg_color(card_0, lv_color_hex(0x1A1A24), 0);
        lv_obj_set_style_radius(card_0, 999, 0);
        lv_obj_set_style_transform_rotation(card_0, 50, 0);
        lv_obj_set_style_transform_pivot_x(card_0, 14, 0);
        lv_obj_set_style_transform_pivot_y(card_0, 41, 0);

        lv_obj_t * card_1 = card_create(container_2);
        lv_obj_set_x(card_1, 4);
        lv_obj_set_y(card_1, 1);
        lv_obj_set_width(card_1, 27);
        lv_obj_set_height(card_1, 81);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0x1A1A24), 0);
        lv_obj_set_style_radius(card_1, 999, 0);
        lv_obj_set_style_transform_rotation(card_1, 1750, 0);
        lv_obj_set_style_transform_pivot_x(card_1, 14, 0);
        lv_obj_set_style_transform_pivot_y(card_1, 41, 0);

        lv_obj_t * image_1 = image_create(lv_obj_0);
        lv_obj_set_x(image_1, 87);
        lv_obj_set_y(image_1, 242);
        lv_obj_set_width(image_1, 180);
        lv_obj_set_height(image_1, 51);
        lv_image_set_src(image_1, image_continue_pill);

        lv_obj_t * label_2 = label_create(lv_obj_0);
        lv_obj_set_x(label_2, 98);
        lv_obj_set_y(label_2, 253);
        lv_label_set_text(label_2, "Continuar");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_24, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_opa(label_2, 184, 0);

        lv_obj_t * container_3 = container_create(lv_obj_0);
        lv_obj_set_x(container_3, 98);
        lv_obj_set_y(container_3, 244);
        lv_obj_set_width(container_3, 165);
        lv_obj_set_height(container_3, 45);
        lv_obj_t * card_2 = card_create(container_3);
        lv_obj_set_x(card_2, 120);
        lv_obj_set_y(card_2, 0);
        lv_obj_set_width(card_2, 45);
        lv_obj_set_height(card_2, 45);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0xD9D9D9), 0);
        lv_obj_set_style_radius(card_2, 32767, 0);

        lv_obj_t * image_2 = image_create(container_3);
        lv_obj_set_x(image_2, 130);
        lv_obj_set_y(image_2, 16);
        lv_obj_set_width(image_2, 25);
        lv_obj_set_height(image_2, 14);
        lv_image_set_src(image_2, image_vec_9_540);

        the_root = lv_obj_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

