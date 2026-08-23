#include "lv_port.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "board_pins.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "touch.h"

static const char *TAG = "lv_port";

#define LV_TICK_PERIOD_MS   2
#define LV_BUF_LINES        20

static SemaphoreHandle_t s_lvgl_mux;
static lv_disp_draw_buf_t s_draw_buf;
static lv_color_t *s_buf1;

/* ESP 定时器回调：按固定周期递增 LVGL 的系统时基。 */
static void lv_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

esp_err_t lv_port_init(void)
{
    s_lvgl_mux = xSemaphoreCreateMutex();
    if (!s_lvgl_mux) {
        return ESP_ERR_NO_MEM;
    }

    ESP_ERROR_CHECK(touch_init());

    lv_init();

    const size_t buf_pixels = BOARD_LCD_H_RES * LV_BUF_LINES;
    s_buf1 = heap_caps_malloc(buf_pixels * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!s_buf1) {
        s_buf1 = heap_caps_malloc(buf_pixels * sizeof(lv_color_t), MALLOC_CAP_DEFAULT);
    }
    if (!s_buf1) {
        ESP_LOGE(TAG, "draw buffer alloc fail");
        return ESP_ERR_NO_MEM;
    }

    lv_disp_draw_buf_init(&s_draw_buf, s_buf1, NULL, buf_pixels);
    lv_port_disp_init(&s_draw_buf);
    lv_port_indev_init();

    const esp_timer_create_args_t tick_args = {
        .callback = &lv_tick_cb,
        .name = "lv_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, LV_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "LVGL ready portrait %dx%d + XPT2046", BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    return ESP_OK;
}

bool lv_port_lock(uint32_t timeout_ms)
{
    const TickType_t ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(s_lvgl_mux, ticks) == pdTRUE;
}

void lv_port_unlock(void)
{
    xSemaphoreGive(s_lvgl_mux);
}
