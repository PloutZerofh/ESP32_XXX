#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lcd.h"
#include "lv_port.h"
#include "assets/img_logo.h"

static const char *TAG = "app";
static lv_obj_t *s_label;
static int s_count;

/* Ignore CLICKED if finger moved more than this (px) while pressed */
#define CLICK_DRAG_THRESH_PX  12

static lv_point_t s_press_pt;
static bool s_dragged;

static void on_btn_event(lv_event_t *e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_get_act();

    if (code == LV_EVENT_PRESSED) {
        s_dragged = false;
        if (indev) {
            lv_indev_get_point(indev, &s_press_pt);
        }
        return;
    }

    if (code == LV_EVENT_PRESSING && indev) {
        lv_point_t now;
        lv_indev_get_point(indev, &now);
        if (abs(now.x - s_press_pt.x) > CLICK_DRAG_THRESH_PX ||
            abs(now.y - s_press_pt.y) > CLICK_DRAG_THRESH_PX) {
            s_dragged = true;
        }
        return;
    }

    if (code == LV_EVENT_CLICKED) {
        /* LVGL fires CLICKED on release even after a slide; filter those out */
        if (s_dragged) {
            return;
        }
        s_count++;
        lv_label_set_text_fmt(s_label, "Count: %d", s_count);
    }
}

static void ui_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), 0);

    /* 贴图：用 lv_img + 编译进固件的 lv_img_dsc_t */
    lv_obj_t *logo = lv_img_create(scr);
    lv_img_set_src(logo, &img_logo);
    lv_obj_set_style_border_width(logo, 0, 0);
    lv_obj_set_style_outline_width(logo, 0, 0);
    lv_obj_align(logo, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *title = lv_label_create(scr);
    /* 240px 竖屏一行放不下，拆成两行 */
    lv_label_set_text(title, "ESP32-2432S028");
    lv_obj_set_style_text_color(title, lv_color_hex(0xeeeeee), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(title, 2, 0);
    lv_obj_set_width(title, BOARD_LCD_H_RES - 16);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 64);

    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 180, 64);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(btn, false, LV_PART_MAIN);
    /* Enough pad so glyph bottoms aren't clipped; disable press "grow" shrink effect */
    lv_obj_set_style_pad_hor(btn, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(btn, 14, LV_PART_MAIN);
    lv_obj_set_style_transform_width(btn, 0, LV_STATE_PRESSED);
    lv_obj_set_style_transform_height(btn, 0, LV_STATE_PRESSED);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_event_cb(btn, on_btn_event, LV_EVENT_ALL, NULL);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click me");
    lv_obj_set_style_text_align(btn_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(btn_label, LV_ALIGN_CENTER, 0, 0);

    s_label = lv_label_create(scr);
    lv_label_set_text(s_label, "Count: 0");
    lv_obj_set_style_text_color(s_label, lv_color_hex(0x00d9ff), 0);
    lv_obj_align(s_label, LV_ALIGN_CENTER, 0, 70);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Touch the button");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    /* Keep clear of the bottom edge — glyph bottoms (e/n) get clipped otherwise */
    lv_obj_set_style_pad_bottom(hint, 4, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -24);
}

void app_main(void)
{
    ESP_ERROR_CHECK(lcd_init());
    ESP_ERROR_CHECK(lv_port_init());

    if (lv_port_lock(0)) {
        ui_create();
        lv_port_unlock();
    }

    ESP_LOGI(TAG, "UI running with XPT2046 touch");
}
