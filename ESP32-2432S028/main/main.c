#include "esp_log.h"
#include "lcd.h"
#include "lv_port.h"
#include "tasks/task_ui.h"
#include "ui/screen_display/screen_main.h"

static const char *TAG = "app";

void app_main(void)
{
    ESP_ERROR_CHECK(lcd_init());
    ESP_ERROR_CHECK(lv_port_init());
    ESP_ERROR_CHECK(task_ui_start());

    if (lv_port_lock(0)) {
        screen_main_create();
        lv_port_unlock();
    }

    ESP_LOGI(TAG, "UI running with XPT2046 touch");
}
