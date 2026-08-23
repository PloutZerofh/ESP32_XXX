#include "touch.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "board_pins.h"

static const char *TAG = "touch";

#define XPT_CMD_X   0x90
#define XPT_CMD_Y   0xD0
#define XPT_SAMPLES 5

static spi_device_handle_t s_spi;
static bool s_ready;

static uint16_t xpt_read_raw(uint8_t cmd)
{
    uint8_t tx[3] = {cmd, 0x00, 0x00};
    uint8_t rx[3] = {0};

    spi_transaction_t t = {
        .length = 24,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };
    if (spi_device_transmit(s_spi, &t) != ESP_OK) {
        return 0;
    }
    return (uint16_t)(((rx[1] << 8) | rx[2]) >> 3);
}

static uint16_t median5(uint16_t *v)
{
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            if (v[j] < v[i]) {
                uint16_t tmp = v[i];
                v[i] = v[j];
                v[j] = tmp;
            }
        }
    }
    return v[2];
}

static bool read_raw_xy(uint16_t *x, uint16_t *y)
{
    uint16_t xs[XPT_SAMPLES];
    uint16_t ys[XPT_SAMPLES];

    for (int i = 0; i < XPT_SAMPLES; ++i) {
        xs[i] = xpt_read_raw(XPT_CMD_X);
        ys[i] = xpt_read_raw(XPT_CMD_Y);
    }

    *x = median5(xs);
    *y = median5(ys);

    /* Discard obviously invalid / noise samples */
    if (*x < 50 || *y < 50 || *x > 4090 || *y > 4090) {
        return false;
    }
    return true;
}

static int16_t map_range(int32_t v, int32_t in_min, int32_t in_max, int32_t out_max)
{
    if (v < in_min) {
        v = in_min;
    }
    if (v > in_max) {
        v = in_max;
    }
    if (in_max <= in_min) {
        return 0;
    }
    return (int16_t)(((v - in_min) * out_max) / (in_max - in_min));
}

esp_err_t touch_init(void)
{
    gpio_config_t irq_cfg = {
        .pin_bit_mask = 1ULL << BOARD_PIN_TOUCH_IRQ,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&irq_cfg));

    spi_bus_config_t buscfg = {
        .mosi_io_num = BOARD_PIN_TOUCH_MOSI,
        .miso_io_num = BOARD_PIN_TOUCH_MISO,
        .sclk_io_num = BOARD_PIN_TOUCH_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    esp_err_t err = spi_bus_initialize(BOARD_TOUCH_HOST, &buscfg, SPI_DMA_DISABLED);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi bus init failed: %s", esp_err_to_name(err));
        return err;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = BOARD_TOUCH_SPI_CLOCK_HZ,
        .mode = 0,
        .spics_io_num = BOARD_PIN_TOUCH_CS,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL,
    };
    err = spi_bus_add_device(BOARD_TOUCH_HOST, &devcfg, &s_spi);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi device add failed: %s", esp_err_to_name(err));
        return err;
    }

    /* Dummy read to power the ADC / settle IRQ */
    (void)xpt_read_raw(XPT_CMD_X);

    s_ready = true;
    ESP_LOGI(TAG, "XPT2046 ready (IRQ=%d CS=%d)", BOARD_PIN_TOUCH_IRQ, BOARD_PIN_TOUCH_CS);
    return ESP_OK;
}

bool touch_read(int16_t *x, int16_t *y)
{
    if (!s_ready || !x || !y) {
        return false;
    }

    /* IRQ low means pressed on this board */
    if (gpio_get_level(BOARD_PIN_TOUCH_IRQ) != 0) {
        return false;
    }

    uint16_t raw_x = 0, raw_y = 0;
    if (!read_raw_xy(&raw_x, &raw_y)) {
        return false;
    }

    /* Map onto the correct screen axis first, then optional invert */
    int16_t tx;
    int16_t ty;
#if BOARD_TOUCH_SWAP_XY
    tx = map_range(raw_y, BOARD_TOUCH_RAW_Y_MIN, BOARD_TOUCH_RAW_Y_MAX, BOARD_LCD_H_RES - 1);
    ty = map_range(raw_x, BOARD_TOUCH_RAW_X_MIN, BOARD_TOUCH_RAW_X_MAX, BOARD_LCD_V_RES - 1);
#else
    tx = map_range(raw_x, BOARD_TOUCH_RAW_X_MIN, BOARD_TOUCH_RAW_X_MAX, BOARD_LCD_H_RES - 1);
    ty = map_range(raw_y, BOARD_TOUCH_RAW_Y_MIN, BOARD_TOUCH_RAW_Y_MAX, BOARD_LCD_V_RES - 1);
#endif

#if BOARD_TOUCH_INVERT_X
    tx = (BOARD_LCD_H_RES - 1) - tx;
#endif
#if BOARD_TOUCH_INVERT_Y
    ty = (BOARD_LCD_V_RES - 1) - ty;
#endif

    if (tx < 0) {
        tx = 0;
    } else if (tx >= BOARD_LCD_H_RES) {
        tx = BOARD_LCD_H_RES - 1;
    }
    if (ty < 0) {
        ty = 0;
    } else if (ty >= BOARD_LCD_V_RES) {
        ty = BOARD_LCD_V_RES - 1;
    }

    *x = tx;
    *y = ty;
    return true;
}
