#include "lv_port_disp.h"

#include "lcd.h"
#include "board_pins.h"
#include "lvgl.h"

static lv_disp_drv_t s_disp_drv;

/* 将 LVGL 请求刷新的区域写入 LCD，并通知 LVGL 刷新完成。 */
static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    const int x = area->x1;
    const int y = area->y1;
    const int w = area->x2 - area->x1 + 1;
    const int h = area->y2 - area->y1 + 1;
    lcd_draw_bitmap(x, y, w, h, (const uint16_t *)color_map);
    lv_disp_flush_ready(drv);
}

void lv_port_disp_init(lv_disp_draw_buf_t *draw_buf)
{
    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = BOARD_LCD_H_RES;
    s_disp_drv.ver_res = BOARD_LCD_V_RES;
    s_disp_drv.flush_cb = disp_flush;
    s_disp_drv.draw_buf = draw_buf;
    lv_disp_drv_register(&s_disp_drv);
}
