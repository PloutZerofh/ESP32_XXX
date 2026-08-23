#!/usr/bin/env python3
"""Convert PNG to LVGL 8 C array (RGB565, LV_COLOR_16_SWAP=0).

Usage:
  python tools/img2lvgl.py logo.png main/assets/img_logo.c img_logo
"""
from __future__ import annotations

import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Need Pillow: pip install pillow")
    sys.exit(1)


def main() -> None:
    if len(sys.argv) < 4:
        print(__doc__)
        sys.exit(1)

    src = Path(sys.argv[1])
    dst = Path(sys.argv[2])
    name = sys.argv[3]

    im = Image.open(src).convert("RGBA")
    w, h = im.size
    data = bytearray()
    for y in range(h):
        for x in range(w):
            r, g, b, a = im.getpixel((x, y))
            if a < 128:
                r, g, b = 0x00, 0xFF, 0x00  # chroma key green
            c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)
            data.append(c & 0xFF)
            data.append((c >> 8) & 0xFF)

    lines = []
    for i in range(0, len(data), 16):
        chunk = ", ".join(f"0x{b:02x}" for b in data[i : i + 16])
        lines.append(f"  {chunk},")

    text = f"""#include "lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t {name}_map[] = {{
{chr(10).join(lines)}
}};

const lv_img_dsc_t {name} = {{
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = {w},
    .header.h = {h},
    .data_size = {w * h * 2},
    .data = {name}_map,
}};
"""
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_text(text, encoding="utf-8")
    print(f"Wrote {dst} ({w}x{h}, {len(data)} bytes RAM/Flash)")


if __name__ == "__main__":
    main()
