
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#elif defined(LV_LVGL_H_INCLUDE_SYSTEM)
#include <lvgl.h>
#elif defined(LV_BUILD_TEST)
#include "../lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMAGE_VEC_96_23_DATA
#define LV_ATTRIBUTE_IMAGE_VEC_96_23_DATA
#endif

static const
LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMAGE_VEC_96_23_DATA
uint8_t image_vec_96_23_data_map[] = {

    0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,
    0xbb,0x7d,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x75,0xbb,0x7d,
    0xdf,0xff,0xff,0xff,0xff,0xff,0xff,0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80,0x20,

};

const lv_image_dsc_t image_vec_96_23_data = {
  .header = {
    .magic = LV_IMAGE_HEADER_MAGIC,
    .cf = LV_COLOR_FORMAT_RGB565A8,
    .flags = 0,
    .w = 8,
    .h = 2,
    .stride = 16,
    .reserved_2 = 0,
  },
  .data_size = sizeof(image_vec_96_23_data_map),
  .data = image_vec_96_23_data_map,
  .reserved = NULL,
};

