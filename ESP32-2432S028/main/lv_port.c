#include "lv_port.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "lcd.h"
#include "touch.h"
#include "board_pins.h"

static const char *TAG = "lv_port";

#define LV_TICK_PERIOD_MS   2
#define LV_TASK_STACK       4096
#define LV_TASK_PRIO        5
#define LV_BUF_LINES        20

static SemaphoreHandle_t s_lvgl_mux;
static lv_disp_draw_buf_t s_draw_buf;
static lv_color_t *s_buf1;
static lv_disp_drv_t s_disp_drv;
static lv_indev_drv_t s_indev_drv;

static void lv_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    (void)drv;
    const int x = area->x1;
    const int y = area->y1;
    const int w = area->x2 - area->x1 + 1;
    const int h = area->y2 - area->y1 + 1;
    lcd_draw_bitmap(x, y, w, h, (const uint16_t *)color_map);
    lv_disp_flush_ready(drv);
}

static void touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    static int16_t last_x;
    static int16_t last_y;
    int16_t x = 0;
    int16_t y = 0;

    if (touch_read(&x, &y)) {
        last_x = x;
        last_y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    data->point.x = last_x;
    data->point.y = last_y;
}

static void lvgl_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "LVGL task start");
    while (1) {
        if (xSemaphoreTake(s_lvgl_mux, portMAX_DELAY) == pdTRUE) {
            uint32_t delay_ms = lv_timer_handler();
            xSemaphoreGive(s_lvgl_mux);
            if (delay_ms > 500) {
                delay_ms = 500;
            } else if (delay_ms < 5) {
                delay_ms = 5;
            }
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
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

    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = BOARD_LCD_H_RES;
    s_disp_drv.ver_res = BOARD_LCD_V_RES;
    s_disp_drv.flush_cb = disp_flush;
    s_disp_drv.draw_buf = &s_draw_buf;
    lv_disp_drv_register(&s_disp_drv);

    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&s_indev_drv);

    const esp_timer_create_args_t tick_args = {
        .callback = &lv_tick_cb,
        .name = "lv_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, LV_TICK_PERIOD_MS * 1000));

    BaseType_t ok = xTaskCreatePinnedToCore(lvgl_task, "lvgl", LV_TASK_STACK, NULL, LV_TASK_PRIO, NULL, 1);
    if (ok != pdPASS) {
        return ESP_FAIL;
    }

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
