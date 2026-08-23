#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t touch_init(void);

/* Returns true when pressed; x/y are landscape LVGL coordinates. */
bool touch_read(int16_t *x, int16_t *y);
