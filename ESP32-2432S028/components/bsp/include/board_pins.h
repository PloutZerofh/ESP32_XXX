#pragma once

/* ESP32-2432S028R (2.8" Cheap Yellow Display) pin map */

#define BOARD_LCD_HOST              SPI2_HOST
#define BOARD_LCD_PIXEL_CLOCK_HZ    (40 * 1000 * 1000)

#define BOARD_PIN_LCD_SCLK          14
#define BOARD_PIN_LCD_MOSI          13
#define BOARD_PIN_LCD_MISO          12
#define BOARD_PIN_LCD_CS            15
#define BOARD_PIN_LCD_DC            2
#define BOARD_PIN_LCD_RST           -1
#define BOARD_PIN_LCD_BL            21

#define BOARD_LCD_BK_LIGHT_ON_LEVEL 1

/*
 * Portrait — match panel GRAM (MADCTL=0x48 proven full-bleed).
 * Hold with USB at the bottom for upright UI.
 */
#define BOARD_LCD_MADCTL            0x48
#define BOARD_LCD_H_RES             240
#define BOARD_LCD_V_RES             320

/* XPT2046 on a dedicated SPI bus (VSPI / SPI3) */
#define BOARD_TOUCH_HOST            SPI3_HOST
#define BOARD_TOUCH_SPI_CLOCK_HZ    (2 * 1000 * 1000)
#define BOARD_PIN_TOUCH_SCLK        25
#define BOARD_PIN_TOUCH_MOSI        32
#define BOARD_PIN_TOUCH_MISO        39
#define BOARD_PIN_TOUCH_CS          33
#define BOARD_PIN_TOUCH_IRQ         36

/* Raw ADC calibration (12-bit). Tweak if edges feel off. */
#define BOARD_TOUCH_RAW_X_MIN       300
#define BOARD_TOUCH_RAW_X_MAX       3800
#define BOARD_TOUCH_RAW_Y_MIN       200
#define BOARD_TOUCH_RAW_Y_MAX       3800

/*
 * Portrait LVGL space (derived from working landscape mapping):
 *   screen_x = map(raw_y)
 *   screen_y = invert(map(raw_x))
 */
#define BOARD_TOUCH_SWAP_XY         1
#define BOARD_TOUCH_INVERT_X        0
#define BOARD_TOUCH_INVERT_Y        1
