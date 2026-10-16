#include "havit_default_background.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include <math.h>

/* Cache the static ring in PSRAM; partial redraws use native RGB565 images. */
static lv_image_dsc_t s_background;

void havit_default_background_create(lv_obj_t *parent)
{
    if(parent == NULL) return;

    lv_obj_set_size(parent, 360, 360);
    lv_obj_set_style_bg_color(parent, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    if(s_background.data == NULL) {
        uint16_t *pixels = heap_caps_calloc(360 * 360, sizeof(uint16_t),
                                           MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if(pixels == NULL) {
            ESP_LOGE("havit_background", "Cannot allocate ring image");
            return;
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

    lv_obj_t *ring_layer = lv_image_create(parent);
    lv_image_set_src(ring_layer, &s_background);
    lv_obj_set_size(ring_layer, 360, 360);
    lv_obj_set_pos(ring_layer, 0, 0);
    lv_obj_clear_flag(ring_layer, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
}
