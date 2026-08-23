#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t lv_port_init(void);
bool lv_port_lock(uint32_t timeout_ms);
void lv_port_unlock(void);
