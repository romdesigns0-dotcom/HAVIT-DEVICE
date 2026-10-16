"""Convert the existing Figma G path to a small LVGL ARGB8888 resource.

Uses only Python's standard library; the original SVG remains the source.
"""
import math
import pathlib
import re
import xml.etree.ElementTree as ET

root = pathlib.Path(__file__).resolve().parents[1]
svg = ET.parse(root / 'ui/assets/login2_google_g.svg').getroot()
path = svg.find('{http://www.w3.org/2000/svg}path').attrib['d']
tokens = re.findall(r'[A-Za-z]|[-+]?(?:\d*\.\d+|\d+)', path)
points = []
i = 0
x = y = 0.0
while i < len(tokens):
    command = tokens[i]
    i += 1
    count = {'M': 2, 'H': 1, 'V': 1, 'L': 2, 'C': 6, 'Z': 0}[command]
    values = list(map(float, tokens[i:i + count]))
    i += count
    if command in ('M', 'L'):
        x, y = values
        points.append((x, y))
    elif command == 'H':
        x = values[0]
        points.append((x, y))
    elif command == 'V':
        y = values[0]
        points.append((x, y))
    elif command == 'C':
        ax, ay, bx, by, cx, cy = values
        for step in range(1, 25):
            t = step / 24
            u = 1 - t
            points.append((u**3*x + 3*u*u*t*ax + 3*u*t*t*bx + t**3*cx,
                           u**3*y + 3*u*u*t*ay + 3*u*t*t*by + t**3*cy))
        x, y = cx, cy

width, height = 35, 36
alpha = [0] * (width * height)
scale = 4
for row in range(height * scale):
    py = (row + 0.5) / scale * 35.5934 / height
    crossings = []
    for (ax, ay), (bx, by) in zip(points, points[1:] + points[:1]):
        if (ay > py) != (by > py):
            crossings.append(ax + (py - ay) * (bx - ax) / (by - ay))
    crossings.sort()
    for left, right in zip(crossings[::2], crossings[1::2]):
        for column in range(width * scale):
            px = (column + 0.5) / scale * 34.8606 / width
            if left <= px < right:
                alpha[(row // scale) * width + column // scale] += 1
data = []
for coverage in alpha:
    data.extend([255, 255, 255, round(coverage * 255 / (scale * scale))])
lines = ['#include "lvgl.h"', '', 'static const uint8_t pixels[] = {']
for offset in range(0, len(data), 24):
    lines.append('    ' + ', '.join(f'0x{v:02x}' for v in data[offset:offset+24]) + ',')
lines.extend(['};', '', 'const lv_image_dsc_t havit_google_g = {',
              '    .header = {.magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_ARGB8888,',
              '               .w = 35, .h = 36, .stride = 140},',
              '    .data_size = sizeof(pixels), .data = pixels,', '};', ''])
(root / 'ui/assets/login2_google_g.c').write_text('\n'.join(lines), encoding='utf-8')
