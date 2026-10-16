"""Rasterize the local Figma arrow's three round strokes for native LVGL."""
from pathlib import Path
import math

root = Path(__file__).resolve().parents[1]
width, height, scale = 29, 17, 4
segments = [(1.5, 8.5, 26.5, 8.5), (19.5, 15.5, 26.5, 8.5),
            (26.5, 8.5, 19.5, 1.5)]

def distance(px, py, ax, ay, bx, by):
    dx, dy = bx - ax, by - ay
    t = max(0, min(1, ((px - ax)*dx + (py - ay)*dy)/(dx*dx + dy*dy)))
    return math.hypot(px - ax - t*dx, py - ay - t*dy)

data = []
for y in range(height):
    for x in range(width):
        covered = sum(
            min(distance((x + (sx + .5)/scale)*28.6213/width,
                         y + (sy + .5)/scale, *segment) for segment in segments) <= 1.5
            for sy in range(scale) for sx in range(scale))
        data.extend([254, 254, 254, round(covered * 255 / (scale*scale))])
lines = ['#include "lvgl.h"', '', 'static const uint8_t pixels[] = {']
for i in range(0, len(data), 24):
    lines.append('    ' + ', '.join(f'0x{v:02x}' for v in data[i:i+24]) + ',')
lines += ['};', '', 'const lv_image_dsc_t havit_continue_arrow = {',
          '    .header = {.magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_ARGB8888,',
          '               .w = 29, .h = 17, .stride = 116},',
          '    .data_size = sizeof(pixels), .data = pixels,', '};', '']
(root/'ui/assets/login1_continue_arrow.c').write_text('\n'.join(lines), encoding='utf-8')
