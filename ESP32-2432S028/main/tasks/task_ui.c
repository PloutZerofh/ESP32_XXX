#include "task_ui.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lv_port.h"

static const char *TAG = "task_ui";

#define LV_TASK_STACK   4096
#define LV_TASK_PRIO    5

static void task_ui_loop(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "LVGL task start");
    while (1) {
        if (lv_port_lock(0)) {
            uint32_t delay_ms = lv_timer_handler();
            lv_port_unlock();
            if (delay_ms > 500) {
                delay_ms = 500;
            } else if (delay_ms < 5) {
                delay_ms = 5;
            }
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
}

esp_err_t task_ui_start(void)
{
    BaseType_t ok = xTaskCreatePinnedToCore(task_ui_loop, "lvgl", LV_TASK_STACK, NULL, LV_TASK_PRIO, NULL, 1);
    if (ok != pdPASS) {
        return ESP_FAIL;
    }
    return ESP_OK;
}
