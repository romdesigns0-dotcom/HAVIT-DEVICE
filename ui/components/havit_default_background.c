#include "havit_default_background.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include <math.h>

/* Preserve the eyes' original RGB565 ring in PSRAM. */
static lv_image_dsc_t s_background;
static lv_obj_t *s_app_ring_layer;
static lv_obj_t *s_scroll_ring_layer;

/* Figma artwork at native 360x360. Default 4's runtime conversion clears only
 * the frame veil requested by the user; original SVGs remain unchanged.
 * Link pixels from flash rather than allocating another PSRAM copy. */
extern const uint8_t default_2_pixels[] asm("_binary_default_2_rgb565_start");
extern const uint8_t default_3_pixels[] asm("_binary_default_3_rgb565_start");
extern const uint8_t default_4_pixels[] asm("_binary_default_4_argb8888_start");

#define BACKGROUND_IMAGE(format, bytes_per_pixel, pixels) { \
    .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = format, \
                .w = 360, .h = 360, .stride = 360 * bytes_per_pixel }, \
    .data_size = 360 * 360 * bytes_per_pixel, .data = pixels \
}
static const lv_image_dsc_t s_app_background =
    BACKGROUND_IMAGE(LV_COLOR_FORMAT_RGB565, 2, default_2_pixels);
static const lv_image_dsc_t s_scroll_base =
    BACKGROUND_IMAGE(LV_COLOR_FORMAT_RGB565, 2, default_3_pixels);
static const lv_image_dsc_t s_scroll_overlay =
    BACKGROUND_IMAGE(LV_COLOR_FORMAT_ARGB8888, 4, default_4_pixels);

static lv_obj_t *create_image_layer(lv_obj_t *parent, const lv_image_dsc_t *asset)
{
    lv_obj_t *layer = lv_image_create(parent);
    lv_image_set_src(layer, asset);
    lv_obj_set_size(layer, 360, 360);
    lv_obj_set_pos(layer, 0, 0);
    lv_obj_clear_flag(layer, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return layer;
}

lv_obj_t *havit_default_background_create(lv_obj_t *parent,
                                           havit_default_background_t variant)
{
    if(parent == NULL) return NULL;

    lv_obj_set_size(parent, 360, 360);
    lv_obj_set_style_bg_color(parent, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    if(variant == HAVIT_DEFAULT_BACKGROUND_3_SCROLL_BASE) {
        lv_obj_t *base = create_image_layer(parent, &s_scroll_base);
        lv_obj_move_background(base);
        return base;
    }

    if(variant == HAVIT_DEFAULT_BACKGROUND_4_SCROLL_RING) {
        /* A screen view can move to the front of its siblings during a fade.
         * The top layer remains above all screen children and ignores touch. */
        s_scroll_ring_layer = create_image_layer(
            lv_display_get_layer_top(lv_obj_get_display(parent)), &s_scroll_overlay);
        lv_obj_add_flag(s_scroll_ring_layer, LV_OBJ_FLAG_HIDDEN);
        return s_scroll_ring_layer;
    }

    if(variant == HAVIT_DEFAULT_BACKGROUND_2_APP) {
        s_app_ring_layer = create_image_layer(parent, &s_app_background);
        return s_app_ring_layer;
    }

    if(s_background.data == NULL) {
        uint16_t *pixels = heap_caps_calloc(360 * 360, sizeof(uint16_t),
                                           MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if(pixels == NULL) {
            ESP_LOGE("havit_background", "Cannot allocate ring image");
            return NULL;
        }
        for(int y = 0; y < 360; y++) {
            float blend = (y + 0.5f - 5.0f) / 350.0f;
            if(blend < 0) blend = 0;
            if(blend > 1) blend = 1;
            for(int x = 0; x < 360; x++) {
                float dx = x + 0.5f - 180;
                float dy = y + 0.5f - 180;
                float coverage = 4.5f - fabsf(sqrtf(dx * dx + dy * dy) - 171.0f);
                if(coverage <= 0) continue;
                if(coverage > 1) coverage = 1;
                uint8_t r = (uint8_t)((119 - 14 * blend) * coverage);
                uint8_t g = (uint8_t)((183 - 32 * blend) * coverage);
                uint8_t b = (uint8_t)((219 - 115 * blend) * coverage);
                pixels[y * 360 + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            }
        }
        s_background.header.magic = LV_IMAGE_HEADER_MAGIC;
        s_background.header.cf = LV_COLOR_FORMAT_RGB565;
        s_background.header.w = 360;
        s_background.header.h = 360;
        s_background.header.stride = 360 * sizeof(uint16_t);
        s_background.data_size = 360 * 360 * sizeof(uint16_t);
        s_background.data = (const uint8_t *)pixels;
    }

    return create_image_layer(parent, &s_background);
}

void havit_default_background_set_scroll_active(bool active)
{
    if(s_app_ring_layer != NULL) {
        if(active) lv_obj_add_flag(s_app_ring_layer, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(s_app_ring_layer, LV_OBJ_FLAG_HIDDEN);
    }
    if(s_scroll_ring_layer != NULL) {
        if(active) {
            lv_obj_clear_flag(s_scroll_ring_layer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(s_scroll_ring_layer);
        }
        else lv_obj_add_flag(s_scroll_ring_layer, LV_OBJ_FLAG_HIDDEN);
    }
    ESP_LOGI("havit_background", "Selected %s background layers",
             active ? "Default 3 + content + Default 4" : "Default 2");
}
