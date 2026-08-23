#!/usr/bin/env python3
from pathlib import Path

W = H = 48
pixels = []
cx = cy = 23.5
r = 18.0

for y in range(H):
    for x in range(W):
        dx, dy = x - cx, y - cy
        d = (dx * dx + dy * dy) ** 0.5
        R, G, B = 26, 26, 46
        if d <= r:
            R, G, B = 0, 200, 255
        if 16 <= x <= 30 and 14 <= y <= 34:
            if (16 <= x <= 20) or (14 <= y <= 18) or (22 <= y <= 26) or (30 <= y <= 34):
                R, G, B = 255, 255, 255
        c = ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | ((B & 0xF8) >> 3)
        pixels.append(c & 0xFF)
        pixels.append((c >> 8) & 0xFF)

out = Path(__file__).resolve().parents[1] / "main" / "ui" / "assets" / "images"
out.mkdir(parents=True, exist_ok=True)

lines = []
for i in range(0, len(pixels), 16):
    chunk = ", ".join(f"0x{b:02x}" for b in pixels[i : i + 16])
    lines.append(f"  {chunk},")

(out / "img_logo.c").write_text(
    "#include \"lvgl.h\"\n\n"
    "#ifndef LV_ATTRIBUTE_MEM_ALIGN\n"
    "#define LV_ATTRIBUTE_MEM_ALIGN\n"
    "#endif\n\n"
    "const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST uint8_t img_logo_map[] = {\n"
    + "\n".join(lines)
    + "\n};\n\n"
    "const lv_img_dsc_t img_logo = {\n"
    "    .header.cf = LV_IMG_CF_TRUE_COLOR,\n"
    "    .header.always_zero = 0,\n"
    "    .header.reserved = 0,\n"
    "    .header.w = 48,\n"
    "    .header.h = 48,\n"
    "    .data_size = 4608,\n"
    "    .data = img_logo_map,\n"
    "};\n",
    encoding="utf-8",
)
(out / "img_logo.h").write_text(
    "#pragma once\n#include \"lvgl.h\"\nextern const lv_img_dsc_t img_logo;\n",
    encoding="utf-8",
)
print(f"Wrote {out / 'img_logo.c'} ({len(pixels)} bytes)")
