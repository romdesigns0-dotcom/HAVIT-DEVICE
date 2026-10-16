/**
 * @file desglose_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "desglose_gen.h"
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

lv_obj_t * desglose_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
        lv_obj_set_name_static(lv_obj_0, "desglose_#");
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
        lv_obj_set_x(label_0, 2);
        lv_obj_set_y(label_0, -117);
        lv_obj_set_height(label_0, 24);
        lv_label_set_text(label_0, "Notificaciones");
        lv_obj_set_style_text_font(label_0, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_0, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * container_1 = container_create(lv_obj_0);
        lv_obj_set_align(container_1, LV_ALIGN_CENTER);
        lv_obj_set_x(container_1, 0);
        lv_obj_set_y(container_1, 0);
        lv_obj_set_width(container_1, 360);
        lv_obj_set_height(container_1, 360);
        lv_obj_set_style_outline_color(container_1, lv_color_hex(0x4A4A4A), 0);
        lv_obj_set_style_outline_opa(container_1, 255, 0);
        lv_obj_set_style_outline_width(container_1, 4, 0);
        lv_obj_set_style_radius(container_1, 32767, 0);
        lv_obj_set_style_clip_corner(container_1, true, 0);
        lv_obj_t * container_2 = container_create(lv_obj_0);
        lv_obj_set_align(container_2, LV_ALIGN_CENTER);
        lv_obj_set_x(container_2, 0);
        lv_obj_set_y(container_2, 0);
        lv_obj_set_width(container_2, 350);
        lv_obj_set_height(container_2, 350);
        lv_obj_set_style_border_color(container_2, lv_color_hex(0x74B2CD), 0);
        lv_obj_set_style_border_opa(container_2, 255, 0);
        lv_obj_set_style_border_width(container_2, 7, 0);
        lv_obj_set_style_transform_rotation(container_2, 1800, 0);
        lv_obj_set_style_transform_pivot_x(container_2, 175, 0);
        lv_obj_set_style_transform_pivot_y(container_2, 175, 0);
        lv_obj_set_style_radius(container_2, 32767, 0);
        lv_obj_set_style_clip_corner(container_2, true, 0);
        lv_obj_t * card_0 = card_create(lv_obj_0);
        lv_obj_set_x(card_0, 79);
        lv_obj_set_y(card_0, 162);
        lv_obj_set_width(card_0, 23);
        lv_obj_set_height(card_0, 23);
        lv_obj_set_style_bg_color(card_0, lv_color_hex(0x1B9E86), 0);
        lv_obj_set_style_radius(card_0, 32767, 0);

        lv_obj_t * image_0 = image_create(lv_obj_0);
        lv_obj_set_x(image_0, 77);
        lv_obj_set_y(image_0, 120);
        lv_obj_set_width(image_0, 26);
        lv_obj_set_height(image_0, 26);
        lv_image_set_src(image_0, image_fluent_sound_wave_circle_16_filled);
        lv_obj_set_flag(image_0, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * image_1 = image_create(lv_obj_0);
        lv_obj_set_x(image_1, 79);
        lv_obj_set_y(image_1, 200);
        lv_obj_set_width(image_1, 23);
        lv_obj_set_height(image_1, 23);
        lv_image_set_src(image_1, image_fluent_person_voice_32_filled);
        lv_obj_set_style_bg_color(image_1, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(image_1, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(image_1, 4, 0);
        lv_obj_set_flag(image_1, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * button_subtle_0 = button_subtle_create(lv_obj_0);
        lv_obj_set_x(button_subtle_0, 160);
        lv_obj_set_y(button_subtle_0, 286);
        lv_obj_set_width(button_subtle_0, 42);
        lv_obj_set_height(button_subtle_0, 42);
        lv_obj_t * image_2 = image_create(button_subtle_0);
        lv_obj_set_x(image_2, 2);
        lv_obj_set_y(image_2, 2);
        lv_obj_set_width(image_2, 39);
        lv_obj_set_height(image_2, 39);
        lv_image_set_src(image_2, image_akar_icons_circle_check_fill_190478);

        lv_obj_add_screen_create_event(button_subtle_0, LV_EVENT_CLICKED, today_c_create, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0);

        lv_obj_t * label_1 = label_create(lv_obj_0);
        lv_obj_set_x(label_1, 118);
        lv_obj_set_y(label_1, 254);
        lv_obj_set_height(label_1, 24);
        lv_label_set_text(label_1, "Agregar Tarea");
        lv_obj_set_style_text_font(label_1, font_instrument_sans_regular_20, 0);
        lv_obj_set_style_text_color(label_1, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_2 = label_create(lv_obj_0);
        lv_obj_set_align(label_2, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_2, -51);
        lv_obj_set_y(label_2, 120);
        lv_obj_set_height(label_2, 28);
        lv_label_set_text(label_2, "Sonido");
        lv_obj_set_style_text_font(label_2, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_2, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_3 = label_create(lv_obj_0);
        lv_obj_set_align(label_3, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_3, -44);
        lv_obj_set_y(label_3, 160);
        lv_obj_set_height(label_3, 28);
        lv_label_set_text(label_3, "Luz/Color");
        lv_obj_set_style_text_font(label_3, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_3, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * label_4 = label_create(lv_obj_0);
        lv_obj_set_align(label_4, LV_ALIGN_TOP_MID);
        lv_obj_set_x(label_4, -57);
        lv_obj_set_y(label_4, 200);
        lv_obj_set_height(label_4, 28);
        lv_label_set_text(label_4, "Tono");
        lv_obj_set_style_text_font(label_4, font_instrument_sans_regular_12, 0);
        lv_obj_set_style_text_color(label_4, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t * card_1 = card_create(lv_obj_0);
        lv_obj_set_x(card_1, 163);
        lv_obj_set_y(card_1, 120);
        lv_obj_set_width(card_1, 62);
        lv_obj_set_height(card_1, 24);
        lv_obj_set_style_bg_color(card_1, lv_color_hex(0x77B7DB), 0);
        lv_obj_set_style_radius(card_1, 5, 0);

        lv_obj_t * card_2 = card_create(lv_obj_0);
        lv_obj_set_x(card_2, 169);
        lv_obj_set_y(card_2, 162);
        lv_obj_set_width(card_2, 62);
        lv_obj_set_height(card_2, 24);
        lv_obj_set_style_bg_color(card_2, lv_color_hex(0x77B7DB), 0);
        lv_obj_set_style_radius(card_2, 5, 0);

        lv_obj_t * card_3 = card_create(lv_obj_0);
        lv_obj_set_x(card_3, 143);
        lv_obj_set_y(card_3, 201);
        lv_obj_set_width(card_3, 62);
        lv_obj_set_height(card_3, 24);
        lv_obj_set_style_bg_color(card_3, lv_color_hex(0x77B7DB), 0);
        lv_obj_set_style_radius(card_3, 5, 0);

        lv_obj_t * image_3 = image_create(lv_obj_0);
        lv_obj_set_x(image_3, 162);
        lv_obj_set_y(image_3, 120);
        lv_obj_set_width(image_3, 24);
        lv_obj_set_height(image_3, 24);
        lv_image_set_src(image_3, image_lets_icons_arrow_drop_down);
        lv_obj_set_flag(image_3, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * image_4 = image_create(lv_obj_0);
        lv_obj_set_x(image_4, 168);
        lv_obj_set_y(image_4, 162);
        lv_obj_set_width(image_4, 24);
        lv_obj_set_height(image_4, 24);
        lv_image_set_src(image_4, image_lets_icons_arrow_drop_down);
        lv_obj_set_flag(image_4, LV_OBJ_FLAG_SCROLLABLE, false);

        lv_obj_t * image_5 = image_create(lv_obj_0);
        lv_obj_set_x(image_5, 142);
        lv_obj_set_y(image_5, 201);
        lv_obj_set_width(image_5, 24);
        lv_obj_set_height(image_5, 24);
        lv_image_set_src(image_5, image_lets_icons_arrow_drop_down);
        lv_obj_set_flag(image_5, LV_OBJ_FLAG_SCROLLABLE, false);

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

