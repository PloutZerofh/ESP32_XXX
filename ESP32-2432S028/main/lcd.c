#include "lcd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_commands.h"

static const char *TAG = "lcd";
static esp_lcd_panel_io_handle_t s_io;
/* Persistent DMA line buffer — must outlive async SPI color TX */
static uint16_t *s_line;
static int s_line_cap;

static inline uint16_t color_to_be(uint16_t color)
{
    return (uint16_t)((color << 8) | (color >> 8));
}

static void lcd_cmd(uint8_t cmd)
{
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(s_io, cmd, NULL, 0));
}

static void lcd_cmd_data(uint8_t cmd, const void *data, size_t len)
{
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(s_io, cmd, data, len));
}

static esp_err_t line_buf_init(void)
{
    const int need = BOARD_LCD_H_RES;
    if (s_line && s_line_cap >= need) {
        return ESP_OK;
    }
    s_line = heap_caps_malloc((size_t)need * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!s_line) {
        return ESP_ERR_NO_MEM;
    }
    s_line_cap = need;
    return ESP_OK;
}

static void ili9341_init(void)
{
    lcd_cmd(LCD_CMD_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));

    lcd_cmd(0xEF);
    lcd_cmd_data(0xEB, (uint8_t[]){0x14}, 1);

    lcd_cmd(0xFE);
    lcd_cmd(0xEF);

    lcd_cmd_data(0xCF, (uint8_t[]){0x00, 0xC1, 0x30}, 3);
    lcd_cmd_data(0xED, (uint8_t[]){0x64, 0x03, 0x12, 0x81}, 4);
    lcd_cmd_data(0xE8, (uint8_t[]){0x85, 0x00, 0x78}, 3);
    lcd_cmd_data(0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5);
    lcd_cmd_data(0xF7, (uint8_t[]){0x20}, 1);
    lcd_cmd_data(0xEA, (uint8_t[]){0x00, 0x00}, 2);

    lcd_cmd_data(0xC0, (uint8_t[]){0x23}, 1);
    lcd_cmd_data(0xC1, (uint8_t[]){0x10}, 1);
    lcd_cmd_data(0xC5, (uint8_t[]){0x3E, 0x28}, 2);
    lcd_cmd_data(0xC7, (uint8_t[]){0x86}, 1);

    lcd_cmd_data(LCD_CMD_MADCTL, (uint8_t[]){BOARD_LCD_MADCTL}, 1);
    lcd_cmd_data(LCD_CMD_COLMOD, (uint8_t[]){0x55}, 1);

    lcd_cmd_data(0xB1, (uint8_t[]){0x00, 0x18}, 2);
    lcd_cmd_data(0xB6, (uint8_t[]){0x08, 0x82, 0x27}, 3);
    lcd_cmd_data(0xF2, (uint8_t[]){0x00}, 1);
    lcd_cmd_data(LCD_CMD_GAMSET, (uint8_t[]){0x01}, 1);

    lcd_cmd_data(0xE0, (uint8_t[]){
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00
    }, 15);
    lcd_cmd_data(0xE1, (uint8_t[]){
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
        0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F
    }, 15);

    lcd_cmd(LCD_CMD_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    lcd_cmd(LCD_CMD_DISPON);
    vTaskDelay(pdMS_TO_TICKS(20));
}

static void set_window(int x0, int y0, int x1, int y1)
{
    uint8_t caset[] = {
        (uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF),
        (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF)
    };
    uint8_t raset[] = {
        (uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF),
        (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF)
    };
    lcd_cmd_data(LCD_CMD_CASET, caset, sizeof(caset));
    lcd_cmd_data(LCD_CMD_RASET, raset, sizeof(raset));
    lcd_cmd(LCD_CMD_RAMWR);
}

esp_err_t lcd_init(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << BOARD_PIN_LCD_BL,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_set_level(BOARD_PIN_LCD_BL, !BOARD_LCD_BK_LIGHT_ON_LEVEL);

    spi_bus_config_t buscfg = {
        .sclk_io_num = BOARD_PIN_LCD_SCLK,
        .mosi_io_num = BOARD_PIN_LCD_MOSI,
        .miso_io_num = BOARD_PIN_LCD_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = BOARD_LCD_H_RES * 40 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(BOARD_LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BOARD_PIN_LCD_DC,
        .cs_gpio_num = BOARD_PIN_LCD_CS,
        .pclk_hz = BOARD_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 1,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BOARD_LCD_HOST, &io_config, &s_io));
    ESP_ERROR_CHECK(line_buf_init());

    ili9341_init();
    lcd_fill_screen(LCD_COLOR_BLACK);
    gpio_set_level(BOARD_PIN_LCD_BL, BOARD_LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "LCD ready portrait %dx%d", BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    return ESP_OK;
}

void lcd_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    if (w <= 0 || h <= 0 || !s_line) {
        return;
    }
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x >= BOARD_LCD_H_RES || y >= BOARD_LCD_V_RES) {
        return;
    }
    if (x + w > BOARD_LCD_H_RES) {
        w = BOARD_LCD_H_RES - x;
    }
    if (y + h > BOARD_LCD_V_RES) {
        h = BOARD_LCD_V_RES - y;
    }

    const size_t line_bytes = (size_t)w * sizeof(uint16_t);
    uint16_t be = color_to_be(color);
    for (int i = 0; i < w; ++i) {
        s_line[i] = be;
    }

    set_window(x, y, x + w - 1, y + h - 1);
    for (int row = 0; row < h; ++row) {
        ESP_ERROR_CHECK(esp_lcd_panel_io_tx_color(s_io, -1, s_line, line_bytes));
    }
}

void lcd_fill_screen(uint16_t color)
{
    lcd_fill_rect(0, 0, BOARD_LCD_H_RES, BOARD_LCD_V_RES, color);
}

void lcd_draw_bitmap(int x, int y, int w, int h, const uint16_t *data)
{
    if (!data || w <= 0 || h <= 0 || !s_line) {
        return;
    }

    int x0 = x;
    int y0 = y;
    int x1 = x + w - 1;
    int y1 = y + h - 1;

    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 >= BOARD_LCD_H_RES) {
        x1 = BOARD_LCD_H_RES - 1;
    }
    if (y1 >= BOARD_LCD_V_RES) {
        y1 = BOARD_LCD_V_RES - 1;
    }
    if (x0 > x1 || y0 > y1) {
        return;
    }

    const int copy_w = x1 - x0 + 1;
    set_window(x0, y0, x1, y1);

    for (int row = y0; row <= y1; ++row) {
        const uint16_t *src = data + (row - y) * w + (x0 - x);
        for (int i = 0; i < copy_w; ++i) {
            s_line[i] = color_to_be(src[i]);
        }
        ESP_ERROR_CHECK(esp_lcd_panel_io_tx_color(s_io, -1, s_line, (size_t)copy_w * sizeof(uint16_t)));
    }
}

int lcd_width(void)
{
    return BOARD_LCD_H_RES;
}

int lcd_height(void)
{
    return BOARD_LCD_V_RES;
}
