#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/* 初始化 LVGL、显示与输入驱动及时基定时器（不启动 UI 任务）。 */
esp_err_t lv_port_init(void);
/* 获取 LVGL 互斥锁；timeout_ms 为零时无限等待。 */
bool lv_port_lock(uint32_t timeout_ms);
/* 释放 LVGL 互斥锁。 */
void lv_port_unlock(void);
