/**
 * @file ui_gen.h
 */

#ifndef LVGL_PRO_UI_GEN_H
#define LVGL_PRO_UI_GEN_H

#ifndef UI_SUBJECT_STRING_LENGTH
#define UI_SUBJECT_STRING_LENGTH 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
    #include "lvgl_private.h"
#else
    #include "lvgl/lvgl.h"
    #include "lvgl/lvgl_private.h"
#endif

#if defined(LV_USE_XML) && LV_USE_XML
    #include "lv_xml/lv_xml.h"
#endif



/* Prototypes for target functions, needed by responsive const definitions */

void ui_set_target(uint32_t target);
uint32_t ui_get_target(void);
bool ui_check_target(uint32_t target);

/*********************
 *      DEFINES
 *********************/

#define UI_TARGET_UNDEFINED  (0 << 1)
#define UI_TARGET_TARGET1    (1 << 1)
#define UI_TARGET_ALL        0x0FFFFFFF

/* By default compile for all targets, allowing to switch to any targets at runtime */
#ifndef UI_COMPILE_TARGET
#define UI_COMPILE_TARGET UI_TARGET_ALL
#endif

#define UI_CHECK_COMPILE_TARGET(target) (UI_COMPILE_TARGET & (target) ? 1 : 0)

#define CONST_BASELIB_ACCENT_GLOBAL lv_color_hex(0x70609C)
#define CONST_BASELIB_ACCENT_TEXT_GLOBAL lv_color_hex(0xFFFFFF)
#define CONST_BASELIB_RADIUS_GLOBAL 8
#define CONST_BASELIB_UNIT_SM_GLOBAL 4


#ifndef LV_XML_EVAL_STRING_BUF_SIZE
    #define LV_XML_EVAL_STRING_BUF_SIZE 256
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL VARIABLES
 **********************/

/*-------------------
 * Permanent screens
 *------------------*/

/*----------------
 * Global styles
 *----------------*/

extern lv_style_t style_screen_base_global;

/*----------------
 * Fonts
 *----------------*/

/* Targets: any */
extern lv_font_t * font_instrument_sans_regular_22;
extern lv_font_t * font_instrument_sans_regular_15;
extern lv_font_t * font_instrument_sans_regular_20;
extern lv_font_t * font_instrument_sans_regular_24;
extern lv_font_t * font_instrument_sans_regular_12;
extern lv_font_t * font_instrument_sans_regular_16;
extern lv_font_t * font_instrument_sans_regular_40;
extern lv_font_t * font_instrument_sans_regular_32;
extern lv_font_t * font_instrument_sans_medium_12;
extern lv_font_t * font_instrument_sans_medium_16;
extern lv_font_t * font_instrument_sans_regular_14;
extern lv_font_t * font_instrument_sans_medium_48;
extern lv_font_t * font_instrument_sans_regular_13;
extern lv_font_t * font_instrument_sans_regular_11;
extern lv_font_t * font_instrument_sans_regular_8;
extern lv_font_t * font_instrument_sans_regular_6;
extern lv_font_t * font_instrument_sans_regular_10;
extern lv_font_t * font_instrument_sans_regular_7;
extern lv_font_t * font_instrument_sans_regular_5;
extern lv_font_t * font_inter_bold_32;
extern lv_font_t * font_inter_medium_32;
extern lv_font_t * font_inter_bold_48;
extern lv_font_t * font_inter_regular_20;
extern lv_font_t * font_inter_semi_bold_24;
extern lv_font_t * font_instrument_sans_bold_24;
extern lv_font_t * font_instrument_sans_semibold_16;


/*----------------
 * Images
 *----------------*/

/* Targets: any */
extern const void * image_google_logo_1;
extern const void * image_vec_9_570;
extern const void * image_vec_9_466;
extern const void * image_vec_9_535;
extern const void * image_vec_9_540;
extern const void * image_vec_46_233;
extern const void * image_ant_design_google_outlined;
extern const void * image_vec_165_36;
extern const void * image_vec_165_37;
extern const void * image_vec_165_45;
extern const void * image_vec_165_46;
extern const void * image_vec_170_54;
extern const void * image_vec_170_55;
extern const void * image_vec_170_56;
extern const void * image_andrsrxn_plus;
extern const void * image_hugeicons_cancel_01_18077;
extern const void * image_hugeicons_cancel_01_199693;
extern const void * image_vec_176_104;
extern const void * image_vec_176_108;
extern const void * image_ic_baseline_keyboard_voice;
extern const void * image_akar_icons_arrow_down_176161;
extern const void * image_akar_icons_arrow_down_176157;
extern const void * image_bitcoin_icons_edit_filled_176222;
extern const void * image_bitcoin_icons_edit_filled_182424;
extern const void * image_vec_197_581;
extern const void * image_vec_199_671;
extern const void * image_vec_199_672;
extern const void * image_akar_icons_circle_check_fill_199677;
extern const void * image_akar_icons_circle_check_fill_190478;
extern const void * image_akar_icons_circle_check_fill_182427;
extern const void * image_vec_190_531;
extern const void * image_vec_190_532;
extern const void * image_vec_190_537;
extern const void * image_vec_190_538;
extern const void * image_vec_190_543;
extern const void * image_vec_190_544;
extern const void * image_vec_190_545;
extern const void * image_ant_design_calendar_filled_176193;
extern const void * image_ant_design_calendar_filled_176169;
extern const void * image_vec_176_197;
extern const void * image_fluent_sound_wave_circle_16_filled;
extern const void * image_fluent_person_voice_32_filled;
extern const void * image_lets_icons_arrow_drop_down;
extern const void * image_vec_176_179;
extern const void * image_vec_176_180;
extern const void * image_vec_176_187;
extern const void * image_vec_176_188;
extern const void * image_vec_96_7;
extern const void * image_vec_96_8;
extern const void * image_vec_96_9;
extern const void * image_vec_96_12;
extern const void * image_vec_96_13;
extern const void * image_vec_96_14;
extern const void * image_vec_96_17;
extern const void * image_vec_96_18;
extern const void * image_vec_96_19;
extern const void * image_vec_96_22;
extern const void * image_vec_96_23;
extern const void * image_vec_96_24;
extern const void * image_icon_182108;
extern const void * image_icon_182206;
extern const void * image_vec_180_18;
extern const void * image_vec_180_19;
extern const void * image_ant_design_calendar_filled;
extern const void * image_gradient_ring;
extern const void * image_gradient_ring_f2;
extern const void * image_gradient_ring_f3;
extern const void * image_continue_pill;

/*----------------
 * Subjects
 *----------------*/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*----------------
 * Event Callbacks
 *----------------*/

/**
 * Initialize the component library
 */

void ui_init_gen(const char * asset_path);

/**********************
 *      MACROS
 **********************/

/**********************
 *   POST INCLUDES
 **********************/

/*Include all the widgets, components and screens of this library*/
#include "components/base/button_subtle/button_subtle_gen.h"
#include "components/base/button/button_gen.h"
#include "components/base/card/card_gen.h"
#include "components/base/column/column_gen.h"
#include "components/base/container/container_gen.h"
#include "components/base/image/image_gen.h"
#include "components/base/label/label_gen.h"
#include "components/base/panel/panel_gen.h"
#include "components/base/spangroup/spangroup_gen.h"
#include "components/welcome_back_rom/welcome_back_rom_gen.h"
#include "screens/add_task_p1_b_gen.h"
#include "screens/add_task_p1_c_gen.h"
#include "screens/add_task_p1_d_gen.h"
#include "screens/add_task_p1_gen.h"
#include "screens/add_task_p2_gen.h"
#include "screens/add_task_p3_gen.h"
#include "screens/add_task_p4_gen.h"
#include "screens/add_task_p5_b_gen.h"
#include "screens/add_task_p5_c_gen.h"
#include "screens/add_task_p5_d_gen.h"
#include "screens/add_task_p5_e_gen.h"
#include "screens/add_task_p5_gen.h"
#include "screens/desglose_b_gen.h"
#include "screens/desglose_gen.h"
#include "screens/esta_semana_gen.h"
#include "screens/frame_1_gen.h"
#include "screens/frame_2_gen.h"
#include "screens/frame_3_gen.h"
#include "screens/home_gen.h"
#include "screens/intro_1_gen.h"
#include "screens/intro_2_gen.h"
#include "screens/intro_3_gen.h"
#include "screens/intro_4_gen.h"
#include "screens/intro_5_gen.h"
#include "screens/log_in_gen.h"
#include "screens/today_b_gen.h"
#include "screens/today_c_gen.h"
#include "screens/today_gen.h"
#include "screens/welcome_back_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LVGL_PRO_UI_GEN_H*/