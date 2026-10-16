/**
 * @file ui_gen.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "ui_gen.h"

#if defined(LV_USE_XML) && LV_USE_XML
#endif /* LV_USE_XML */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void check_font(lv_font_t ** font, const char * name);

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t ui_target = UI_TARGET_ALL;

/*----------------
 * Translations
 *----------------*/

#ifndef LV_EDITOR_PREVIEW
    static const char * translation_languages[] = {"en", "de", NULL};
    static const char * translation_tags[] = {"dog", "cat", "house", NULL};
    static const char * translation_texts[] = {
        "This is a dog", "Das ist ein Hund", /* dog */
        "A curious little cat", "Eine neugierige kleine Katze", /* cat */
        "The house is cozy and warm", "Das Haus ist gemütlich und warm", /* house */
    };
#endif

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/

/*----------------
 * Fonts
 *----------------*/

lv_font_t * font_instrument_sans_regular_22;
lv_font_t * font_instrument_sans_regular_15;
lv_font_t * font_instrument_sans_regular_20;
lv_font_t * font_instrument_sans_regular_24;
lv_font_t * font_instrument_sans_regular_12;
lv_font_t * font_instrument_sans_regular_16;
lv_font_t * font_instrument_sans_regular_40;
lv_font_t * font_instrument_sans_regular_32;
lv_font_t * font_instrument_sans_medium_12;
lv_font_t * font_instrument_sans_medium_16;
lv_font_t * font_instrument_sans_regular_14;
lv_font_t * font_instrument_sans_medium_48;
lv_font_t * font_instrument_sans_regular_13;
lv_font_t * font_instrument_sans_regular_11;
lv_font_t * font_instrument_sans_regular_8;
lv_font_t * font_instrument_sans_regular_6;
lv_font_t * font_instrument_sans_regular_10;
lv_font_t * font_instrument_sans_regular_7;
lv_font_t * font_instrument_sans_regular_5;
lv_font_t * font_inter_bold_32;
lv_font_t * font_inter_medium_32;
lv_font_t * font_inter_bold_48;
lv_font_t * font_inter_regular_20;
lv_font_t * font_inter_semi_bold_24;
lv_font_t * font_instrument_sans_bold_24;
lv_font_t * font_instrument_sans_semibold_16;

/*----------------
 * Images
 *----------------*/

/* Targets: any */
const void * image_google_logo_1 = NULL;
extern const void * image_google_logo_1_data;
const void * image_vec_9_570 = NULL;
extern const void * image_vec_9_570_data;
const void * image_vec_9_466 = NULL;
extern const void * image_vec_9_466_data;
const void * image_vec_9_535 = NULL;
extern const void * image_vec_9_535_data;
const void * image_vec_9_540 = NULL;
extern const void * image_vec_9_540_data;
const void * image_vec_46_233 = NULL;
extern const void * image_vec_46_233_data;
const void * image_ant_design_google_outlined = NULL;
extern const void * image_ant_design_google_outlined_data;
const void * image_vec_165_36 = NULL;
extern const void * image_vec_165_36_data;
const void * image_vec_165_37 = NULL;
extern const void * image_vec_165_37_data;
const void * image_vec_165_45 = NULL;
extern const void * image_vec_165_45_data;
const void * image_vec_165_46 = NULL;
extern const void * image_vec_165_46_data;
const void * image_vec_170_54 = NULL;
extern const void * image_vec_170_54_data;
const void * image_vec_170_55 = NULL;
extern const void * image_vec_170_55_data;
const void * image_vec_170_56 = NULL;
extern const void * image_vec_170_56_data;
const void * image_andrsrxn_plus = NULL;
extern const void * image_andrsrxn_plus_data;
const void * image_hugeicons_cancel_01_18077 = NULL;
extern const void * image_hugeicons_cancel_01_18077_data;
const void * image_hugeicons_cancel_01_199693 = NULL;
extern const void * image_hugeicons_cancel_01_199693_data;
const void * image_vec_176_104 = NULL;
extern const void * image_vec_176_104_data;
const void * image_vec_176_108 = NULL;
extern const void * image_vec_176_108_data;
const void * image_ic_baseline_keyboard_voice = NULL;
extern const void * image_ic_baseline_keyboard_voice_data;
const void * image_akar_icons_arrow_down_176161 = NULL;
extern const void * image_akar_icons_arrow_down_176161_data;
const void * image_akar_icons_arrow_down_176157 = NULL;
extern const void * image_akar_icons_arrow_down_176157_data;
const void * image_bitcoin_icons_edit_filled_176222 = NULL;
extern const void * image_bitcoin_icons_edit_filled_176222_data;
const void * image_bitcoin_icons_edit_filled_182424 = NULL;
extern const void * image_bitcoin_icons_edit_filled_182424_data;
const void * image_vec_197_581 = NULL;
extern const void * image_vec_197_581_data;
const void * image_vec_199_671 = NULL;
extern const void * image_vec_199_671_data;
const void * image_vec_199_672 = NULL;
extern const void * image_vec_199_672_data;
const void * image_akar_icons_circle_check_fill_199677 = NULL;
extern const void * image_akar_icons_circle_check_fill_199677_data;
const void * image_akar_icons_circle_check_fill_190478 = NULL;
extern const void * image_akar_icons_circle_check_fill_190478_data;
const void * image_akar_icons_circle_check_fill_182427 = NULL;
extern const void * image_akar_icons_circle_check_fill_182427_data;
const void * image_vec_190_531 = NULL;
extern const void * image_vec_190_531_data;
const void * image_vec_190_532 = NULL;
extern const void * image_vec_190_532_data;
const void * image_vec_190_537 = NULL;
extern const void * image_vec_190_537_data;
const void * image_vec_190_538 = NULL;
extern const void * image_vec_190_538_data;
const void * image_vec_190_543 = NULL;
extern const void * image_vec_190_543_data;
const void * image_vec_190_544 = NULL;
extern const void * image_vec_190_544_data;
const void * image_vec_190_545 = NULL;
extern const void * image_vec_190_545_data;
const void * image_ant_design_calendar_filled_176193 = NULL;
extern const void * image_ant_design_calendar_filled_176193_data;
const void * image_ant_design_calendar_filled_176169 = NULL;
extern const void * image_ant_design_calendar_filled_176169_data;
const void * image_vec_176_197 = NULL;
extern const void * image_vec_176_197_data;
const void * image_fluent_sound_wave_circle_16_filled = NULL;
extern const void * image_fluent_sound_wave_circle_16_filled_data;
const void * image_fluent_person_voice_32_filled = NULL;
extern const void * image_fluent_person_voice_32_filled_data;
const void * image_lets_icons_arrow_drop_down = NULL;
extern const void * image_lets_icons_arrow_drop_down_data;
const void * image_vec_176_179 = NULL;
extern const void * image_vec_176_179_data;
const void * image_vec_176_180 = NULL;
extern const void * image_vec_176_180_data;
const void * image_vec_176_187 = NULL;
extern const void * image_vec_176_187_data;
const void * image_vec_176_188 = NULL;
extern const void * image_vec_176_188_data;
const void * image_vec_96_7 = NULL;
extern const void * image_vec_96_7_data;
const void * image_vec_96_8 = NULL;
extern const void * image_vec_96_8_data;
const void * image_vec_96_9 = NULL;
extern const void * image_vec_96_9_data;
const void * image_vec_96_12 = NULL;
extern const void * image_vec_96_12_data;
const void * image_vec_96_13 = NULL;
extern const void * image_vec_96_13_data;
const void * image_vec_96_14 = NULL;
extern const void * image_vec_96_14_data;
const void * image_vec_96_17 = NULL;
extern const void * image_vec_96_17_data;
const void * image_vec_96_18 = NULL;
extern const void * image_vec_96_18_data;
const void * image_vec_96_19 = NULL;
extern const void * image_vec_96_19_data;
const void * image_vec_96_22 = NULL;
extern const void * image_vec_96_22_data;
const void * image_vec_96_23 = NULL;
extern const void * image_vec_96_23_data;
const void * image_vec_96_24 = NULL;
extern const void * image_vec_96_24_data;
const void * image_icon_182108 = NULL;
extern const void * image_icon_182108_data;
const void * image_icon_182206 = NULL;
extern const void * image_icon_182206_data;
const void * image_vec_180_18 = NULL;
extern const void * image_vec_180_18_data;
const void * image_vec_180_19 = NULL;
extern const void * image_vec_180_19_data;
const void * image_ant_design_calendar_filled = NULL;
extern const void * image_ant_design_calendar_filled_data;
const void * image_gradient_ring = NULL;
extern const void * image_gradient_ring_data;
const void * image_gradient_ring_f2 = NULL;
extern const void * image_gradient_ring_f2_data;
const void * image_gradient_ring_f3 = NULL;
extern const void * image_gradient_ring_f3_data;
const void * image_continue_pill = NULL;
extern const void * image_continue_pill_data;

/*----------------
 * Global styles
 *----------------*/

lv_style_t style_screen_base_global;

/*----------------
 * Subjects
 *----------------*/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void ui_init_gen(const char * asset_path)
{
    char buf[256];

    /* When running from the editor the theme set from the XML should overwrite this */
#if !defined(LV_EDITOR_PREVIEW)
#if LV_USE_THEME_SIMPLE
    lv_display_t * disp = lv_display_get_default();
    lv_theme_t * th = lv_theme_simple_init(disp);
    lv_display_set_theme(disp, th);
#else
    LV_LOG_WARN("Simple theme is selected in project.xml but LV_USE_THEME_SIMPLE is disabled");
#endif
#endif /*LV_EDITOR_PREVIEW*/


    /*----------------
     * Fonts
     *----------------*/

    /* Targets: any */

    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        if (!font_instrument_sans_regular_22) {
            /* font_instrument_sans_regular_22 */
            /* create bin font 'font_instrument_sans_regular_22' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_22.bin");
            font_instrument_sans_regular_22 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_15) {
            /* font_instrument_sans_regular_15 */
            /* create bin font 'font_instrument_sans_regular_15' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_15.bin");
            font_instrument_sans_regular_15 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_20) {
            /* font_instrument_sans_regular_20 */
            /* create bin font 'font_instrument_sans_regular_20' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_20.bin");
            font_instrument_sans_regular_20 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_24) {
            /* font_instrument_sans_regular_24 */
            /* create bin font 'font_instrument_sans_regular_24' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_24.bin");
            font_instrument_sans_regular_24 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_12) {
            /* font_instrument_sans_regular_12 */
            /* create bin font 'font_instrument_sans_regular_12' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_12.bin");
            font_instrument_sans_regular_12 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_16) {
            /* font_instrument_sans_regular_16 */
            /* create bin font 'font_instrument_sans_regular_16' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_16.bin");
            font_instrument_sans_regular_16 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_40) {
            /* font_instrument_sans_regular_40 */
            /* create bin font 'font_instrument_sans_regular_40' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_40.bin");
            font_instrument_sans_regular_40 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_32) {
            /* font_instrument_sans_regular_32 */
            /* create bin font 'font_instrument_sans_regular_32' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_32.bin");
            font_instrument_sans_regular_32 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_medium_12) {
            /* font_instrument_sans_medium_12 */
            /* create bin font 'font_instrument_sans_medium_12' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_medium_12.bin");
            font_instrument_sans_medium_12 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_medium_16) {
            /* font_instrument_sans_medium_16 */
            /* create bin font 'font_instrument_sans_medium_16' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_medium_16.bin");
            font_instrument_sans_medium_16 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_14) {
            /* font_instrument_sans_regular_14 */
            /* create bin font 'font_instrument_sans_regular_14' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_14.bin");
            font_instrument_sans_regular_14 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_medium_48) {
            /* font_instrument_sans_medium_48 */
            /* create bin font 'font_instrument_sans_medium_48' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_medium_48.bin");
            font_instrument_sans_medium_48 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_13) {
            /* font_instrument_sans_regular_13 */
            /* create bin font 'font_instrument_sans_regular_13' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_13.bin");
            font_instrument_sans_regular_13 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_11) {
            /* font_instrument_sans_regular_11 */
            /* create bin font 'font_instrument_sans_regular_11' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_11.bin");
            font_instrument_sans_regular_11 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_8) {
            /* font_instrument_sans_regular_8 */
            /* create bin font 'font_instrument_sans_regular_8' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_8.bin");
            font_instrument_sans_regular_8 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_6) {
            /* font_instrument_sans_regular_6 */
            /* create bin font 'font_instrument_sans_regular_6' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_6.bin");
            font_instrument_sans_regular_6 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_10) {
            /* font_instrument_sans_regular_10 */
            /* create bin font 'font_instrument_sans_regular_10' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_10.bin");
            font_instrument_sans_regular_10 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_7) {
            /* font_instrument_sans_regular_7 */
            /* create bin font 'font_instrument_sans_regular_7' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_7.bin");
            font_instrument_sans_regular_7 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_regular_5) {
            /* font_instrument_sans_regular_5 */
            /* create bin font 'font_instrument_sans_regular_5' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_regular_5.bin");
            font_instrument_sans_regular_5 = lv_binfont_create(buf);

        }
        if (!font_inter_bold_32) {
            /* font_inter_bold_32 */
            /* create bin font 'font_inter_bold_32' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_inter_bold_32.bin");
            font_inter_bold_32 = lv_binfont_create(buf);

        }
        if (!font_inter_medium_32) {
            /* font_inter_medium_32 */
            /* create bin font 'font_inter_medium_32' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_inter_medium_32.bin");
            font_inter_medium_32 = lv_binfont_create(buf);

        }
        if (!font_inter_bold_48) {
            /* font_inter_bold_48 */
            /* create bin font 'font_inter_bold_48' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_inter_bold_48.bin");
            font_inter_bold_48 = lv_binfont_create(buf);

        }
        if (!font_inter_regular_20) {
            /* font_inter_regular_20 */
            /* create bin font 'font_inter_regular_20' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_inter_regular_20.bin");
            font_inter_regular_20 = lv_binfont_create(buf);

        }
        if (!font_inter_semi_bold_24) {
            /* font_inter_semi_bold_24 */
            /* create bin font 'font_inter_semi_bold_24' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_inter_semi_bold_24.bin");
            font_inter_semi_bold_24 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_bold_24) {
            /* font_instrument_sans_bold_24 */
            /* create bin font 'font_instrument_sans_bold_24' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_bold_24.bin");
            font_instrument_sans_bold_24 = lv_binfont_create(buf);

        }
        if (!font_instrument_sans_semibold_16) {
            /* font_instrument_sans_semibold_16 */
            /* create bin font 'font_instrument_sans_semibold_16' from file */
            lv_snprintf(buf, 256, "%s%s", asset_path, "fonts/font_instrument_sans_semibold_16.bin");
            font_instrument_sans_semibold_16 = lv_binfont_create(buf);

        }
    }
    #endif

    /*----------------
     * Images
     *----------------*/

    /* Targets: any */
    #if UI_CHECK_COMPILE_TARGET(UI_TARGET_ALL)
    if (ui_check_target(UI_TARGET_ALL)) {
        /* image_google_logo_1 */
        if (!image_google_logo_1) {
            image_google_logo_1 = &image_google_logo_1_data;
        }
        /* image_vec_9_570 */
        if (!image_vec_9_570) {
            image_vec_9_570 = &image_vec_9_570_data;
        }
        /* image_vec_9_466 */
        if (!image_vec_9_466) {
            image_vec_9_466 = &image_vec_9_466_data;
        }
        /* image_vec_9_535 */
        if (!image_vec_9_535) {
            image_vec_9_535 = &image_vec_9_535_data;
        }
        /* image_vec_9_540 */
        if (!image_vec_9_540) {
            image_vec_9_540 = &image_vec_9_540_data;
        }
        /* image_vec_46_233 */
        if (!image_vec_46_233) {
            image_vec_46_233 = &image_vec_46_233_data;
        }
        /* image_ant_design_google_outlined */
        if (!image_ant_design_google_outlined) {
            image_ant_design_google_outlined = &image_ant_design_google_outlined_data;
        }
        /* image_vec_165_36 */
        if (!image_vec_165_36) {
            image_vec_165_36 = &image_vec_165_36_data;
        }
        /* image_vec_165_37 */
        if (!image_vec_165_37) {
            image_vec_165_37 = &image_vec_165_37_data;
        }
        /* image_vec_165_45 */
        if (!image_vec_165_45) {
            image_vec_165_45 = &image_vec_165_45_data;
        }
        /* image_vec_165_46 */
        if (!image_vec_165_46) {
            image_vec_165_46 = &image_vec_165_46_data;
        }
        /* image_vec_170_54 */
        if (!image_vec_170_54) {
            image_vec_170_54 = &image_vec_170_54_data;
        }
        /* image_vec_170_55 */
        if (!image_vec_170_55) {
            image_vec_170_55 = &image_vec_170_55_data;
        }
        /* image_vec_170_56 */
        if (!image_vec_170_56) {
            image_vec_170_56 = &image_vec_170_56_data;
        }
        /* image_andrsrxn_plus */
        if (!image_andrsrxn_plus) {
            image_andrsrxn_plus = &image_andrsrxn_plus_data;
        }
        /* image_hugeicons_cancel_01_18077 */
        if (!image_hugeicons_cancel_01_18077) {
            image_hugeicons_cancel_01_18077 = &image_hugeicons_cancel_01_18077_data;
        }
        /* image_hugeicons_cancel_01_199693 */
        if (!image_hugeicons_cancel_01_199693) {
            image_hugeicons_cancel_01_199693 = &image_hugeicons_cancel_01_199693_data;
        }
        /* image_vec_176_104 */
        if (!image_vec_176_104) {
            image_vec_176_104 = &image_vec_176_104_data;
        }
        /* image_vec_176_108 */
        if (!image_vec_176_108) {
            image_vec_176_108 = &image_vec_176_108_data;
        }
        /* image_ic_baseline_keyboard_voice */
        if (!image_ic_baseline_keyboard_voice) {
            image_ic_baseline_keyboard_voice = &image_ic_baseline_keyboard_voice_data;
        }
        /* image_akar_icons_arrow_down_176161 */
        if (!image_akar_icons_arrow_down_176161) {
            image_akar_icons_arrow_down_176161 = &image_akar_icons_arrow_down_176161_data;
        }
        /* image_akar_icons_arrow_down_176157 */
        if (!image_akar_icons_arrow_down_176157) {
            image_akar_icons_arrow_down_176157 = &image_akar_icons_arrow_down_176157_data;
        }
        /* image_bitcoin_icons_edit_filled_176222 */
        if (!image_bitcoin_icons_edit_filled_176222) {
            image_bitcoin_icons_edit_filled_176222 = &image_bitcoin_icons_edit_filled_176222_data;
        }
        /* image_bitcoin_icons_edit_filled_182424 */
        if (!image_bitcoin_icons_edit_filled_182424) {
            image_bitcoin_icons_edit_filled_182424 = &image_bitcoin_icons_edit_filled_182424_data;
        }
        /* image_vec_197_581 */
        if (!image_vec_197_581) {
            image_vec_197_581 = &image_vec_197_581_data;
        }
        /* image_vec_199_671 */
        if (!image_vec_199_671) {
            image_vec_199_671 = &image_vec_199_671_data;
        }
        /* image_vec_199_672 */
        if (!image_vec_199_672) {
            image_vec_199_672 = &image_vec_199_672_data;
        }
        /* image_akar_icons_circle_check_fill_199677 */
        if (!image_akar_icons_circle_check_fill_199677) {
            image_akar_icons_circle_check_fill_199677 = &image_akar_icons_circle_check_fill_199677_data;
        }
        /* image_akar_icons_circle_check_fill_190478 */
        if (!image_akar_icons_circle_check_fill_190478) {
            image_akar_icons_circle_check_fill_190478 = &image_akar_icons_circle_check_fill_190478_data;
        }
        /* image_akar_icons_circle_check_fill_182427 */
        if (!image_akar_icons_circle_check_fill_182427) {
            image_akar_icons_circle_check_fill_182427 = &image_akar_icons_circle_check_fill_182427_data;
        }
        /* image_vec_190_531 */
        if (!image_vec_190_531) {
            image_vec_190_531 = &image_vec_190_531_data;
        }
        /* image_vec_190_532 */
        if (!image_vec_190_532) {
            image_vec_190_532 = &image_vec_190_532_data;
        }
        /* image_vec_190_537 */
        if (!image_vec_190_537) {
            image_vec_190_537 = &image_vec_190_537_data;
        }
        /* image_vec_190_538 */
        if (!image_vec_190_538) {
            image_vec_190_538 = &image_vec_190_538_data;
        }
        /* image_vec_190_543 */
        if (!image_vec_190_543) {
            image_vec_190_543 = &image_vec_190_543_data;
        }
        /* image_vec_190_544 */
        if (!image_vec_190_544) {
            image_vec_190_544 = &image_vec_190_544_data;
        }
        /* image_vec_190_545 */
        if (!image_vec_190_545) {
            image_vec_190_545 = &image_vec_190_545_data;
        }
        /* image_ant_design_calendar_filled_176193 */
        if (!image_ant_design_calendar_filled_176193) {
            image_ant_design_calendar_filled_176193 = &image_ant_design_calendar_filled_176193_data;
        }
        /* image_ant_design_calendar_filled_176169 */
        if (!image_ant_design_calendar_filled_176169) {
            image_ant_design_calendar_filled_176169 = &image_ant_design_calendar_filled_176169_data;
        }
        /* image_vec_176_197 */
        if (!image_vec_176_197) {
            image_vec_176_197 = &image_vec_176_197_data;
        }
        /* image_fluent_sound_wave_circle_16_filled */
        if (!image_fluent_sound_wave_circle_16_filled) {
            image_fluent_sound_wave_circle_16_filled = &image_fluent_sound_wave_circle_16_filled_data;
        }
        /* image_fluent_person_voice_32_filled */
        if (!image_fluent_person_voice_32_filled) {
            image_fluent_person_voice_32_filled = &image_fluent_person_voice_32_filled_data;
        }
        /* image_lets_icons_arrow_drop_down */
        if (!image_lets_icons_arrow_drop_down) {
            image_lets_icons_arrow_drop_down = &image_lets_icons_arrow_drop_down_data;
        }
        /* image_vec_176_179 */
        if (!image_vec_176_179) {
            image_vec_176_179 = &image_vec_176_179_data;
        }
        /* image_vec_176_180 */
        if (!image_vec_176_180) {
            image_vec_176_180 = &image_vec_176_180_data;
        }
        /* image_vec_176_187 */
        if (!image_vec_176_187) {
            image_vec_176_187 = &image_vec_176_187_data;
        }
        /* image_vec_176_188 */
        if (!image_vec_176_188) {
            image_vec_176_188 = &image_vec_176_188_data;
        }
        /* image_vec_96_7 */
        if (!image_vec_96_7) {
            image_vec_96_7 = &image_vec_96_7_data;
        }
        /* image_vec_96_8 */
        if (!image_vec_96_8) {
            image_vec_96_8 = &image_vec_96_8_data;
        }
        /* image_vec_96_9 */
        if (!image_vec_96_9) {
            image_vec_96_9 = &image_vec_96_9_data;
        }
        /* image_vec_96_12 */
        if (!image_vec_96_12) {
            image_vec_96_12 = &image_vec_96_12_data;
        }
        /* image_vec_96_13 */
        if (!image_vec_96_13) {
            image_vec_96_13 = &image_vec_96_13_data;
        }
        /* image_vec_96_14 */
        if (!image_vec_96_14) {
            image_vec_96_14 = &image_vec_96_14_data;
        }
        /* image_vec_96_17 */
        if (!image_vec_96_17) {
            image_vec_96_17 = &image_vec_96_17_data;
        }
        /* image_vec_96_18 */
        if (!image_vec_96_18) {
            image_vec_96_18 = &image_vec_96_18_data;
        }
        /* image_vec_96_19 */
        if (!image_vec_96_19) {
            image_vec_96_19 = &image_vec_96_19_data;
        }
        /* image_vec_96_22 */
        if (!image_vec_96_22) {
            image_vec_96_22 = &image_vec_96_22_data;
        }
        /* image_vec_96_23 */
        if (!image_vec_96_23) {
            image_vec_96_23 = &image_vec_96_23_data;
        }
        /* image_vec_96_24 */
        if (!image_vec_96_24) {
            image_vec_96_24 = &image_vec_96_24_data;
        }
        /* image_icon_182108 */
        if (!image_icon_182108) {
            image_icon_182108 = &image_icon_182108_data;
        }
        /* image_icon_182206 */
        if (!image_icon_182206) {
            image_icon_182206 = &image_icon_182206_data;
        }
        /* image_vec_180_18 */
        if (!image_vec_180_18) {
            image_vec_180_18 = &image_vec_180_18_data;
        }
        /* image_vec_180_19 */
        if (!image_vec_180_19) {
            image_vec_180_19 = &image_vec_180_19_data;
        }
        /* image_ant_design_calendar_filled */
        if (!image_ant_design_calendar_filled) {
            image_ant_design_calendar_filled = &image_ant_design_calendar_filled_data;
        }
        /* image_gradient_ring */
        if (!image_gradient_ring) {
            image_gradient_ring = &image_gradient_ring_data;
        }
        /* image_gradient_ring_f2 */
        if (!image_gradient_ring_f2) {
            image_gradient_ring_f2 = &image_gradient_ring_f2_data;
        }
        /* image_gradient_ring_f3 */
        if (!image_gradient_ring_f3) {
            image_gradient_ring_f3 = &image_gradient_ring_f3_data;
        }
        /* image_continue_pill */
        if (!image_continue_pill) {
            image_continue_pill = &image_continue_pill_data;
        }
    }
    #endif

    /*----------------
     * Global styles
     *----------------*/

    static bool style_inited = false;

    if (!style_inited) {
        /*Init all styles*/
        lv_style_init(&style_screen_base_global);

        lv_style_set_border_width(&style_screen_base_global, 0);
        lv_style_set_radius(&style_screen_base_global, 0);
        lv_style_set_shadow_width(&style_screen_base_global, 0);
        lv_style_set_shadow_opa(&style_screen_base_global, 0);

        style_inited = true;
    }

    /*----------------
     * Subjects
     *----------------*/
    /*----------------
     * Translations
     *----------------*/

    #ifndef LV_EDITOR_PREVIEW
        lv_translation_add_static(translation_languages, translation_tags, translation_texts);
        lv_translation_set_language(translation_languages[0]);
    #endif

#if defined(LV_USE_XML) && LV_USE_XML
    /* Register widgets */

    /* Check all fonts / default if needed. This prevents fonts that are used in one target but
       defined in another from causing assertion failures during rendering of the Preview. */
    check_font(&font_instrument_sans_regular_22, "font_instrument_sans_regular_22");
    check_font(&font_instrument_sans_regular_15, "font_instrument_sans_regular_15");
    check_font(&font_instrument_sans_regular_20, "font_instrument_sans_regular_20");
    check_font(&font_instrument_sans_regular_24, "font_instrument_sans_regular_24");
    check_font(&font_instrument_sans_regular_12, "font_instrument_sans_regular_12");
    check_font(&font_instrument_sans_regular_16, "font_instrument_sans_regular_16");
    check_font(&font_instrument_sans_regular_40, "font_instrument_sans_regular_40");
    check_font(&font_instrument_sans_regular_32, "font_instrument_sans_regular_32");
    check_font(&font_instrument_sans_medium_12, "font_instrument_sans_medium_12");
    check_font(&font_instrument_sans_medium_16, "font_instrument_sans_medium_16");
    check_font(&font_instrument_sans_regular_14, "font_instrument_sans_regular_14");
    check_font(&font_instrument_sans_medium_48, "font_instrument_sans_medium_48");
    check_font(&font_instrument_sans_regular_13, "font_instrument_sans_regular_13");
    check_font(&font_instrument_sans_regular_11, "font_instrument_sans_regular_11");
    check_font(&font_instrument_sans_regular_8, "font_instrument_sans_regular_8");
    check_font(&font_instrument_sans_regular_6, "font_instrument_sans_regular_6");
    check_font(&font_instrument_sans_regular_10, "font_instrument_sans_regular_10");
    check_font(&font_instrument_sans_regular_7, "font_instrument_sans_regular_7");
    check_font(&font_instrument_sans_regular_5, "font_instrument_sans_regular_5");
    check_font(&font_inter_bold_32, "font_inter_bold_32");
    check_font(&font_inter_medium_32, "font_inter_medium_32");
    check_font(&font_inter_bold_48, "font_inter_bold_48");
    check_font(&font_inter_regular_20, "font_inter_regular_20");
    check_font(&font_inter_semi_bold_24, "font_inter_semi_bold_24");
    check_font(&font_instrument_sans_bold_24, "font_instrument_sans_bold_24");
    check_font(&font_instrument_sans_semibold_16, "font_instrument_sans_semibold_16");

    /* Register fonts */
    lv_xml_register_font(NULL, "font_instrument_sans_regular_22", font_instrument_sans_regular_22);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_15", font_instrument_sans_regular_15);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_20", font_instrument_sans_regular_20);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_24", font_instrument_sans_regular_24);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_12", font_instrument_sans_regular_12);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_16", font_instrument_sans_regular_16);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_40", font_instrument_sans_regular_40);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_32", font_instrument_sans_regular_32);
    lv_xml_register_font(NULL, "font_instrument_sans_medium_12", font_instrument_sans_medium_12);
    lv_xml_register_font(NULL, "font_instrument_sans_medium_16", font_instrument_sans_medium_16);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_14", font_instrument_sans_regular_14);
    lv_xml_register_font(NULL, "font_instrument_sans_medium_48", font_instrument_sans_medium_48);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_13", font_instrument_sans_regular_13);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_11", font_instrument_sans_regular_11);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_8", font_instrument_sans_regular_8);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_6", font_instrument_sans_regular_6);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_10", font_instrument_sans_regular_10);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_7", font_instrument_sans_regular_7);
    lv_xml_register_font(NULL, "font_instrument_sans_regular_5", font_instrument_sans_regular_5);
    lv_xml_register_font(NULL, "font_inter_bold_32", font_inter_bold_32);
    lv_xml_register_font(NULL, "font_inter_medium_32", font_inter_medium_32);
    lv_xml_register_font(NULL, "font_inter_bold_48", font_inter_bold_48);
    lv_xml_register_font(NULL, "font_inter_regular_20", font_inter_regular_20);
    lv_xml_register_font(NULL, "font_inter_semi_bold_24", font_inter_semi_bold_24);
    lv_xml_register_font(NULL, "font_instrument_sans_bold_24", font_instrument_sans_bold_24);
    lv_xml_register_font(NULL, "font_instrument_sans_semibold_16", font_instrument_sans_semibold_16);

    /* Register subjects */

    /* Register callbacks */
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if defined(LV_USE_XML) && LV_USE_XML && !defined(LV_EDITOR_PREVIEW)
    /* Register images */
    lv_xml_register_image(NULL, "image_google_logo_1", image_google_logo_1);
    lv_xml_register_image(NULL, "image_vec_9_570", image_vec_9_570);
    lv_xml_register_image(NULL, "image_vec_9_466", image_vec_9_466);
    lv_xml_register_image(NULL, "image_vec_9_535", image_vec_9_535);
    lv_xml_register_image(NULL, "image_vec_9_540", image_vec_9_540);
    lv_xml_register_image(NULL, "image_vec_46_233", image_vec_46_233);
    lv_xml_register_image(NULL, "image_ant_design_google_outlined", image_ant_design_google_outlined);
    lv_xml_register_image(NULL, "image_vec_165_36", image_vec_165_36);
    lv_xml_register_image(NULL, "image_vec_165_37", image_vec_165_37);
    lv_xml_register_image(NULL, "image_vec_165_45", image_vec_165_45);
    lv_xml_register_image(NULL, "image_vec_165_46", image_vec_165_46);
    lv_xml_register_image(NULL, "image_vec_170_54", image_vec_170_54);
    lv_xml_register_image(NULL, "image_vec_170_55", image_vec_170_55);
    lv_xml_register_image(NULL, "image_vec_170_56", image_vec_170_56);
    lv_xml_register_image(NULL, "image_andrsrxn_plus", image_andrsrxn_plus);
    lv_xml_register_image(NULL, "image_hugeicons_cancel_01_18077", image_hugeicons_cancel_01_18077);
    lv_xml_register_image(NULL, "image_hugeicons_cancel_01_199693", image_hugeicons_cancel_01_199693);
    lv_xml_register_image(NULL, "image_vec_176_104", image_vec_176_104);
    lv_xml_register_image(NULL, "image_vec_176_108", image_vec_176_108);
    lv_xml_register_image(NULL, "image_ic_baseline_keyboard_voice", image_ic_baseline_keyboard_voice);
    lv_xml_register_image(NULL, "image_akar_icons_arrow_down_176161", image_akar_icons_arrow_down_176161);
    lv_xml_register_image(NULL, "image_akar_icons_arrow_down_176157", image_akar_icons_arrow_down_176157);
    lv_xml_register_image(NULL, "image_bitcoin_icons_edit_filled_176222", image_bitcoin_icons_edit_filled_176222);
    lv_xml_register_image(NULL, "image_bitcoin_icons_edit_filled_182424", image_bitcoin_icons_edit_filled_182424);
    lv_xml_register_image(NULL, "image_vec_197_581", image_vec_197_581);
    lv_xml_register_image(NULL, "image_vec_199_671", image_vec_199_671);
    lv_xml_register_image(NULL, "image_vec_199_672", image_vec_199_672);
    lv_xml_register_image(NULL, "image_akar_icons_circle_check_fill_199677", image_akar_icons_circle_check_fill_199677);
    lv_xml_register_image(NULL, "image_akar_icons_circle_check_fill_190478", image_akar_icons_circle_check_fill_190478);
    lv_xml_register_image(NULL, "image_akar_icons_circle_check_fill_182427", image_akar_icons_circle_check_fill_182427);
    lv_xml_register_image(NULL, "image_vec_190_531", image_vec_190_531);
    lv_xml_register_image(NULL, "image_vec_190_532", image_vec_190_532);
    lv_xml_register_image(NULL, "image_vec_190_537", image_vec_190_537);
    lv_xml_register_image(NULL, "image_vec_190_538", image_vec_190_538);
    lv_xml_register_image(NULL, "image_vec_190_543", image_vec_190_543);
    lv_xml_register_image(NULL, "image_vec_190_544", image_vec_190_544);
    lv_xml_register_image(NULL, "image_vec_190_545", image_vec_190_545);
    lv_xml_register_image(NULL, "image_ant_design_calendar_filled_176193", image_ant_design_calendar_filled_176193);
    lv_xml_register_image(NULL, "image_ant_design_calendar_filled_176169", image_ant_design_calendar_filled_176169);
    lv_xml_register_image(NULL, "image_vec_176_197", image_vec_176_197);
    lv_xml_register_image(NULL, "image_fluent_sound_wave_circle_16_filled", image_fluent_sound_wave_circle_16_filled);
    lv_xml_register_image(NULL, "image_fluent_person_voice_32_filled", image_fluent_person_voice_32_filled);
    lv_xml_register_image(NULL, "image_lets_icons_arrow_drop_down", image_lets_icons_arrow_drop_down);
    lv_xml_register_image(NULL, "image_vec_176_179", image_vec_176_179);
    lv_xml_register_image(NULL, "image_vec_176_180", image_vec_176_180);
    lv_xml_register_image(NULL, "image_vec_176_187", image_vec_176_187);
    lv_xml_register_image(NULL, "image_vec_176_188", image_vec_176_188);
    lv_xml_register_image(NULL, "image_vec_96_7", image_vec_96_7);
    lv_xml_register_image(NULL, "image_vec_96_8", image_vec_96_8);
    lv_xml_register_image(NULL, "image_vec_96_9", image_vec_96_9);
    lv_xml_register_image(NULL, "image_vec_96_12", image_vec_96_12);
    lv_xml_register_image(NULL, "image_vec_96_13", image_vec_96_13);
    lv_xml_register_image(NULL, "image_vec_96_14", image_vec_96_14);
    lv_xml_register_image(NULL, "image_vec_96_17", image_vec_96_17);
    lv_xml_register_image(NULL, "image_vec_96_18", image_vec_96_18);
    lv_xml_register_image(NULL, "image_vec_96_19", image_vec_96_19);
    lv_xml_register_image(NULL, "image_vec_96_22", image_vec_96_22);
    lv_xml_register_image(NULL, "image_vec_96_23", image_vec_96_23);
    lv_xml_register_image(NULL, "image_vec_96_24", image_vec_96_24);
    lv_xml_register_image(NULL, "image_icon_182108", image_icon_182108);
    lv_xml_register_image(NULL, "image_icon_182206", image_icon_182206);
    lv_xml_register_image(NULL, "image_vec_180_18", image_vec_180_18);
    lv_xml_register_image(NULL, "image_vec_180_19", image_vec_180_19);
    lv_xml_register_image(NULL, "image_ant_design_calendar_filled", image_ant_design_calendar_filled);
    lv_xml_register_image(NULL, "image_gradient_ring", image_gradient_ring);
    lv_xml_register_image(NULL, "image_gradient_ring_f2", image_gradient_ring_f2);
    lv_xml_register_image(NULL, "image_gradient_ring_f3", image_gradient_ring_f3);
    lv_xml_register_image(NULL, "image_continue_pill", image_continue_pill);
#endif

#if !defined(LV_USE_XML) || LV_USE_XML == 0
    /*--------------------
     *  Permanent screens
     *-------------------*/
    /* If XML is enabled it's assumed that the permanent screens are created
     * manually from XML using lv_xml_create() */
#endif
}

void ui_set_target(uint32_t target)
{
    ui_target = target;
}

uint32_t ui_get_target(void)
{
    return ui_target;
}

bool ui_check_target(uint32_t target)
{
    return (ui_target & target) ? true : false;
}

/* Callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void check_font(lv_font_t ** font, const char * name)
{
    if (!(*font)) {
        *font = (lv_font_t *)LV_FONT_DEFAULT;
        LV_LOG_WARN("font `%s` was not set. Using `LV_FONT_DEFAULT` instead", name);
    }
}