#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "board_pins.h"

#define LCD_COLOR_BLACK   0x0000
#define LCD_COLOR_WHITE   0xFFFF
#define LCD_COLOR_RED     0xF800
#define LCD_COLOR_GREEN   0x07E0
#define LCD_COLOR_BLUE    0x001F

esp_err_t lcd_init(void);
void lcd_fill_screen(uint16_t color);
void lcd_fill_rect(int x, int y, int w, int h, uint16_t color);
/* RGB565 little-endian, landscape row-major — used by LVGL flush */
void lcd_draw_bitmap(int x, int y, int w, int h, const uint16_t *data);
int lcd_width(void);
int lcd_height(void);
