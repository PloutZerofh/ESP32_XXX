#include "lv_port_indev.h"

#include "touch.h"
#include "lvgl.h"

static lv_indev_drv_t s_indev_drv;

/* 将触摸控制器状态转换为 LVGL 指针输入数据，并保留最后有效坐标。 */
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

void lv_port_indev_init(void)
{
    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type = LV_INDEV_TYPE_POINTER;
    s_indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&s_indev_drv);
}
