#pragma once

#include "lvgl.h"

/* 注册 LVGL 显示驱动并将 flush 回调绑定到 LCD。 */
void lv_port_disp_init(lv_disp_draw_buf_t *draw_buf);
