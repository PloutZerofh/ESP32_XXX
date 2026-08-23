#pragma once

#include "esp_err.h"

/* 启动 LVGL 后台任务，周期性调用 lv_timer_handler()。 */
esp_err_t task_ui_start(void);
