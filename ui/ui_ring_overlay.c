#include "lvgl.h"
#include "ui_ring_overlay.h"
#include <stdint.h>
#if defined(ESP_PLATFORM)
#include "esp_timer.h"
#endif

static ui_ring_overlay_stats_t s_ring_stats;

void ui_ring_overlay_reset_stats(void)
{
    s_ring_stats.calls = 0;
    s_ring_stats.last_us = 0;
    s_ring_stats.max_us = 0;
    s_ring_stats.total_us = 0;
}

void ui_ring_overlay_get_stats(ui_ring_overlay_stats_t * stats)
{
    if(stats) *stats = s_ring_stats;
}

static uint32_t ring_clock_us(void)
{
#if defined(ESP_PLATFORM)
    return (uint32_t)esp_timer_get_time();
#else
    return lv_tick_get() * 1000U;
#endif
}

static lv_obj_t * find_design_ring(lv_obj_t * parent)
{
    if(lv_obj_get_width(parent) == 350 && lv_obj_get_height(parent) == 350) return parent;
    uint32_t count = lv_obj_get_child_count(parent);
    for(uint32_t i = 0; i < count; i++) {
        lv_obj_t * found = find_design_ring(lv_obj_get_child(parent, i));
        if(found) return found;
    }
    return NULL;
}

static uint32_t blend(uint32_t top, uint32_t bottom, uint32_t step, uint32_t total)
{
    return (top * (total - step) + bottom * step + total / 2) / total;
}

static void draw_gradient_ring(lv_event_t * event)
{
    const uint32_t started = ring_clock_us();
    lv_obj_t * ring = lv_event_get_target_obj(event);
    lv_layer_t * layer = lv_event_get_layer(event);
    if(!ring || !layer) return;
    lv_area_t area;
    lv_obj_get_coords(ring, &area);
    lv_point_t center = { .x = (area.x1 + area.x2) / 2, .y = (area.y1 + area.y2) / 2 };

    const int32_t segment_count = 72;
    for(int32_t i = 0; i < segment_count; i++) {
        int32_t mid = i * 5 + 2;
        uint32_t vertical;
        if(mid < 90) vertical = 36 + (uint32_t)mid * 36 / 90;
        else if(mid < 270) vertical = 72 - (uint32_t)(mid - 90) * 72 / 180;
        else vertical = (uint32_t)(mid - 270) * 36 / 90;
        vertical = 72 - vertical;

        lv_draw_arc_dsc_t arc;
        lv_draw_arc_dsc_init(&arc);
        arc.base.layer = layer;
        arc.center = center;
        arc.radius = 174;
        arc.width = 8;
        arc.opa = LV_OPA_COVER;
        arc.start_angle = i * 5;
        arc.end_angle = (i == segment_count - 1) ? 360 : (i + 1) * 5 + 1;
        arc.color = lv_color_make(
            (uint8_t)blend(119, 115, vertical, 72),
            (uint8_t)blend(183, 155, vertical, 72),
            (uint8_t)blend(219, 112, vertical, 72));
        lv_draw_arc(layer, &arc);
    }
    const uint32_t elapsed = ring_clock_us() - started;
    s_ring_stats.calls++;
    s_ring_stats.last_us = elapsed;
    s_ring_stats.total_us += elapsed;
    if(elapsed > s_ring_stats.max_us) s_ring_stats.max_us = elapsed;
}

void ui_ring_overlay(lv_obj_t * screen)
{
    lv_obj_update_layout(screen);
    lv_obj_t * ring = find_design_ring(screen);
    if(!ring) return;
    lv_obj_set_style_border_opa(ring, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_event_cb(ring, draw_gradient_ring, LV_EVENT_DRAW_POST, NULL);
}
