"""Convert the original Figma Default 2/3/4 SVG assets to LVGL pixel data.

Requires Pillow and resvg-py. Native dimensions are preserved; no artwork edits.
"""
from io import BytesIO
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image
import resvg_py

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "ui/assets/backgrounds"


def main():
    for number in (2, 3, 4):
        source = SOURCE / f"default_{number}.svg"
        root = ET.parse(source).getroot()
        assert float(root.attrib["width"]) == 360
        assert float(root.attrib["height"]) == 360
        png = resvg_py.svg_to_bytes(svg_path=str(source), skip_system_fonts=True)
        image = Image.open(BytesIO(png)).convert("RGBA")
        assert image.size == (360, 360)
        if number == 4:
            # The user requests a clear viewport: remove only the uniform
            # white 20% design-frame veil, retaining ring and exterior mask.
            # Original SVG remains intact. Uncompose that white underlay
            # algebraically to preserve antialiasing on the ring's edge.
            adjusted = []
            base = 51 / 255
            for r, g, b, alpha in image.getdata():
                a = alpha / 255
                foreground = max(0.0, (a - base) / (1 - base))
                if foreground == 0:
                    adjusted.append((0, 0, 0, 0))
                    continue
                channels = [round(max(0, min(255, (c * a - 255 * base * (1 - foreground)) / foreground)))
                            for c in (r, g, b)]
                adjusted.append((*channels, round(foreground * 255)))
            image.putdata(adjusted)
            pixels = image.tobytes("raw", "BGRA")
            target = SOURCE / "default_4.argb8888"
        else:
            opaque = Image.new("RGBA", image.size, (0, 0, 0, 255))
            opaque.alpha_composite(image)
            pixels = bytearray()
            for r, g, b, _ in opaque.getdata():
                value = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                pixels.extend(value.to_bytes(2, "little"))
            target = SOURCE / f"default_{number}.rgb565"
        target.write_bytes(pixels)
        print(f"{target.name}: {len(pixels)} bytes, center={image.getpixel((180, 180))}, corner={image.getpixel((0, 0))}")


if __name__ == "__main__":
    main()
