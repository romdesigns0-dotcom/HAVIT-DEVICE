#include "ui_gen.h"
#include "src/font/binfont_loader/lv_binfont_loader.h"

#include <stdint.h>

#define DECLARE_FONT(id, file_name) \
    extern const uint8_t id##_start[] asm("_binary_" file_name "_start"); \
    extern const uint8_t id##_end[] asm("_binary_" file_name "_end")

DECLARE_FONT(f_isr_22, "font_instrument_sans_regular_22_bin");
DECLARE_FONT(f_isr_15, "font_instrument_sans_regular_15_bin");
DECLARE_FONT(f_isr_20, "font_instrument_sans_regular_20_bin");
DECLARE_FONT(f_isr_24, "font_instrument_sans_regular_24_bin");
DECLARE_FONT(f_isr_12, "font_instrument_sans_regular_12_bin");
DECLARE_FONT(f_isr_16, "font_instrument_sans_regular_16_bin");
DECLARE_FONT(f_isr_40, "font_instrument_sans_regular_40_bin");
DECLARE_FONT(f_isr_32, "font_instrument_sans_regular_32_bin");
DECLARE_FONT(f_ism_12, "font_instrument_sans_medium_12_bin");
DECLARE_FONT(f_ism_16, "font_instrument_sans_medium_16_bin");
DECLARE_FONT(f_isr_14, "font_instrument_sans_regular_14_bin");
DECLARE_FONT(f_ism_48, "font_instrument_sans_medium_48_bin");
DECLARE_FONT(f_isr_13, "font_instrument_sans_regular_13_bin");
DECLARE_FONT(f_isr_11, "font_instrument_sans_regular_11_bin");
DECLARE_FONT(f_isr_8, "font_instrument_sans_regular_8_bin");
DECLARE_FONT(f_isr_6, "font_instrument_sans_regular_6_bin");
DECLARE_FONT(f_isr_10, "font_instrument_sans_regular_10_bin");
DECLARE_FONT(f_isr_7, "font_instrument_sans_regular_7_bin");
DECLARE_FONT(f_isr_5, "font_instrument_sans_regular_5_bin");
DECLARE_FONT(f_ib_32, "font_inter_bold_32_bin");
DECLARE_FONT(f_im_32, "font_inter_medium_32_bin");
DECLARE_FONT(f_ib_48, "font_inter_bold_48_bin");
DECLARE_FONT(f_ir_20, "font_inter_regular_20_bin");
DECLARE_FONT(f_isb_24, "font_inter_semi_bold_24_bin");
DECLARE_FONT(f_isb_24_2, "font_instrument_sans_bold_24_bin");
DECLARE_FONT(f_iss_16, "font_instrument_sans_semibold_16_bin");

#define LOAD_FONT(destination, id) \
    (destination) = lv_binfont_create_from_buffer((void *)id##_start, \
                                                   (uint32_t)(id##_end - id##_start))

void havit_ui_fonts_init(void)
{
    LOAD_FONT(font_instrument_sans_regular_22, f_isr_22);
    LOAD_FONT(font_instrument_sans_regular_15, f_isr_15);
    LOAD_FONT(font_instrument_sans_regular_20, f_isr_20);
    LOAD_FONT(font_instrument_sans_regular_24, f_isr_24);
    LOAD_FONT(font_instrument_sans_regular_12, f_isr_12);
    LOAD_FONT(font_instrument_sans_regular_16, f_isr_16);
    LOAD_FONT(font_instrument_sans_regular_40, f_isr_40);
    LOAD_FONT(font_instrument_sans_regular_32, f_isr_32);
    LOAD_FONT(font_instrument_sans_medium_12, f_ism_12);
    LOAD_FONT(font_instrument_sans_medium_16, f_ism_16);
    LOAD_FONT(font_instrument_sans_regular_14, f_isr_14);
    LOAD_FONT(font_instrument_sans_medium_48, f_ism_48);
    LOAD_FONT(font_instrument_sans_regular_13, f_isr_13);
    LOAD_FONT(font_instrument_sans_regular_11, f_isr_11);
    LOAD_FONT(font_instrument_sans_regular_8, f_isr_8);
    LOAD_FONT(font_instrument_sans_regular_6, f_isr_6);
    LOAD_FONT(font_instrument_sans_regular_10, f_isr_10);
    LOAD_FONT(font_instrument_sans_regular_7, f_isr_7);
    LOAD_FONT(font_instrument_sans_regular_5, f_isr_5);
    LOAD_FONT(font_inter_bold_32, f_ib_32);
    LOAD_FONT(font_inter_medium_32, f_im_32);
    LOAD_FONT(font_inter_bold_48, f_ib_48);
    LOAD_FONT(font_inter_regular_20, f_ir_20);
    LOAD_FONT(font_inter_semi_bold_24, f_isb_24);
    LOAD_FONT(font_instrument_sans_bold_24, f_isb_24_2);
    LOAD_FONT(font_instrument_sans_semibold_16, f_iss_16);
}
