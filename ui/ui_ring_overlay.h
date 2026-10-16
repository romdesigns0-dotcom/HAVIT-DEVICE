#ifndef UI_RING_OVERLAY_H
#define UI_RING_OVERLAY_H
#include "lvgl.h"

typedef struct {
    uint32_t calls;
    uint32_t last_us;
    uint32_t max_us;
    uint64_t total_us;
} ui_ring_overlay_stats_t;

void ui_ring_overlay(lv_obj_t * screen);
void ui_ring_overlay_reset_stats(void);
void ui_ring_overlay_get_stats(ui_ring_overlay_stats_t * stats);
#endif
