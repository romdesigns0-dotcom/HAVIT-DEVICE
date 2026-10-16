/**
 * @file welcome_back_rom_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "welcome_back_rom_gen.h"
#include "../../ui.h"

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

lv_obj_t * welcome_back_rom_create(lv_obj_t * parent)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_span_1;
    static lv_style_t style_span_2;

    static bool style_inited = false;

    if (!style_inited) {
        /*Init all styles*/
        lv_style_init(&style_span_1);
        lv_style_init(&style_span_2);

        lv_style_set_text_color(&style_span_1, lv_color_hex(0xFFFFFF));
        lv_style_set_text_font(&style_span_1, font_instrument_sans_regular_20);
        lv_style_set_text_color(&style_span_2, lv_color_hex(0x5250F3));
        lv_style_set_text_font(&style_span_2, font_instrument_sans_regular_20);

        style_inited = true;
    }


    lv_obj_t * the_root = NULL;

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        lv_obj_t * spangroup_0 = spangroup_create(parent);
        lv_obj_set_name_static(spangroup_0, "welcome_back_rom_#");
        lv_obj_set_width(spangroup_0, LV_SIZE_CONTENT);
        lv_obj_set_height(spangroup_0, LV_SIZE_CONTENT);

        lv_span_t * lv_spangroup_span_0 = lv_spangroup_add_span(spangroup_0);
        lv_spangroup_set_span_text(spangroup_0, lv_spangroup_span_0, "Welcome\nBack ");
        lv_spangroup_set_span_style(spangroup_0, lv_spangroup_span_0, &style_span_1);
        lv_span_t * lv_spangroup_span_1 = lv_spangroup_add_span(spangroup_0);
        lv_spangroup_set_span_text(spangroup_0, lv_spangroup_span_1, "ROM");
        lv_spangroup_set_span_style(spangroup_0, lv_spangroup_span_1, &style_span_2);

        the_root = spangroup_0;
    }
    #endif

    LV_TRACE_OBJ_CREATE("finished");

    return the_root;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

