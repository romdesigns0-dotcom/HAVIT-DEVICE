/**
 * @file add_task_p1_d_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "add_task_p1_d_gen.h"
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

lv_obj_t * add_task_p1_d_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "add_task_p1_d_#");
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
        lv_obj_set_x(label_0, 0);
        lv_obj_set_y(label_0, -63);
        lv_obj_set_height(label_0, 39);
        lv_label_set_text(label_0, "Humanidades");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_32, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_align(label_1, LV_ALIGN_CENTER);
        lv_obj_set_x(label_1, 1);
        lv_obj_set_y(label_1, 58);
        lv_obj_set_height(label_1, 24);
        lv_label_set_text(label_1, "Completado");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(label_1, LV_TEXT_ALIGN_CENTER, 0);

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
        lv_obj_t * button_subtle_0 = button_subtle_create(lv_obj_0);
        lv_obj_set_x(button_subtle_0, 149);
        lv_obj_set_y(button_subtle_0, 149);
        lv_obj_set_width(button_subtle_0, 62);
        lv_obj_set_height(button_subtle_0, 62);
        lv_obj_t * image_0 = image_create(button_subtle_0);
        lv_obj_set_x(image_0, 3);
        lv_obj_set_y(image_0, 3);
        lv_obj_set_width(image_0, 57);
        lv_obj_set_height(image_0, 57);
        lv_image_set_src(image_0, image_akar_icons_circle_check_fill_199677);

        lv_obj_add_screen_create_event(button_subtle_0, LV_EVENT_CLICKED, today_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

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

