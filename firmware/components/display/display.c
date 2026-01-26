/**
 * @file display.c
 * @brief ST7789 LCD display driver implementation
 *
 * REQ-SW-030: Display Driver
 * REQ-SW-038: Screen Orientation - Portrait mode (135x240)
 * Optimized for TTGO T-Display with ST7789 panel in portrait orientation.
 */

#include "display.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "display";

// Hardware configuration - TTGO T-Display
// REQ-SW-038: Portrait mode dimensions (135 wide x 240 tall)
#define LCD_WIDTH           135
#define LCD_HEIGHT          240
#define LCD_PIN_MOSI        19
#define LCD_PIN_SCLK        18
#define LCD_PIN_CS          5
#define LCD_PIN_DC          16
#define LCD_PIN_RST         23
#define LCD_PIN_BL          4
#define LCD_SPI_CLOCK_HZ    (40 * 1000 * 1000)

// ST7789 display offset (240x320 panel showing 135x240 window in portrait)
// For portrait mode, column offset adjusts for the 135px width centered in 240px panel
#define LCD_COL_OFFSET      52
#define LCD_ROW_OFFSET      40

// ST7789 Commands
#define ST7789_NOP          0x00
#define ST7789_SWRESET      0x01
#define ST7789_SLPIN        0x10
#define ST7789_SLPOUT       0x11
#define ST7789_NORON        0x13
#define ST7789_INVOFF       0x20
#define ST7789_INVON        0x21
#define ST7789_DISPOFF      0x28
#define ST7789_DISPON       0x29
#define ST7789_CASET        0x2A
#define ST7789_RASET        0x2B
#define ST7789_RAMWR        0x2C
#define ST7789_MADCTL       0x36
#define ST7789_COLMOD       0x3A

// MADCTL bits
#define MADCTL_MY           0x80
#define MADCTL_MX           0x40
#define MADCTL_MV           0x20
#define MADCTL_ML           0x10
#define MADCTL_RGB          0x00
#define MADCTL_BGR          0x08

// Basic 6x8 font data (ASCII 32-127)
static const uint8_t s_font_6x8[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00, // $
    0x23, 0x13, 0x08, 0x64, 0x62, 0x00, // %
    0x36, 0x49, 0x55, 0x22, 0x50, 0x00, // &
    0x00, 0x05, 0x03, 0x00, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, 0x00, // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, // +
    0x00, 0x50, 0x30, 0x00, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x00, // -
    0x00, 0x60, 0x60, 0x00, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, 0x00, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, 0x00, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, 0x00, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, 0x00, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, 0x00, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, 0x00, // =
    0x41, 0x22, 0x14, 0x08, 0x00, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, 0x00, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, 0x00, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, 0x00, // F
    0x3E, 0x41, 0x41, 0x51, 0x32, 0x00, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, // R
    0x46, 0x49, 0x49, 0x49, 0x31, 0x00, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00, // W
    0x63, 0x14, 0x08, 0x14, 0x63, 0x00, // X
    0x03, 0x04, 0x78, 0x04, 0x03, 0x00, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, 0x00, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, // [
    0x02, 0x04, 0x08, 0x10, 0x20, 0x00, // backslash
    0x41, 0x41, 0x7F, 0x00, 0x00, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, 0x00, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, 0x00, // _
    0x00, 0x01, 0x02, 0x04, 0x00, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, 0x00, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, 0x00, // b
    0x38, 0x44, 0x44, 0x44, 0x20, 0x00, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, 0x00, // d
    0x38, 0x54, 0x54, 0x54, 0x18, 0x00, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, 0x00, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, 0x00, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, 0x00, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, 0x00, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, 0x00, // n
    0x38, 0x44, 0x44, 0x44, 0x38, 0x00, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, 0x00, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, 0x00, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, 0x00, // r
    0x48, 0x54, 0x54, 0x54, 0x20, 0x00, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, 0x00, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00, // w
    0x44, 0x28, 0x10, 0x28, 0x44, 0x00, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, // z
    0x00, 0x08, 0x36, 0x41, 0x00, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08, 0x00, // ~
};

// Static variables
static spi_device_handle_t s_spi = NULL;
static uint8_t s_brightness = 200;

// DMA-capable buffer for SPI transfers
#define SPI_MAX_TRANSFER_SIZE   (LCD_WIDTH * 32 * 2)  // 32 rows at a time
static DRAM_ATTR uint8_t s_spi_buffer[SPI_MAX_TRANSFER_SIZE];

//-----------------------------------------------------------------------------
// Low-level SPI functions
//-----------------------------------------------------------------------------

static void lcd_cmd(uint8_t cmd)
{
    gpio_set_level(LCD_PIN_DC, 0);  // Command mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_polling_transmit(s_spi, &t);
}

static void lcd_data(const uint8_t *data, size_t len)
{
    if (len == 0) return;
    gpio_set_level(LCD_PIN_DC, 1);  // Data mode
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
    };
    spi_device_polling_transmit(s_spi, &t);
}

static void lcd_data_byte(uint8_t data)
{
    lcd_data(&data, 1);
}

static void lcd_set_window(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    // Apply ST7789 offset for 135x240 on 240x320 panel
    // The display is rotated, so we swap x/y offsets
    uint16_t xa = x0 + LCD_COL_OFFSET;
    uint16_t xb = x1 + LCD_COL_OFFSET;
    uint16_t ya = y0 + LCD_ROW_OFFSET;
    uint16_t yb = y1 + LCD_ROW_OFFSET;

    lcd_cmd(ST7789_CASET);
    uint8_t col_data[] = {xa >> 8, xa & 0xFF, xb >> 8, xb & 0xFF};
    lcd_data(col_data, 4);

    lcd_cmd(ST7789_RASET);
    uint8_t row_data[] = {ya >> 8, ya & 0xFF, yb >> 8, yb & 0xFF};
    lcd_data(row_data, 4);

    lcd_cmd(ST7789_RAMWR);
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------

esp_err_t display_init(void)
{
    ESP_LOGI(TAG, "Initializing ST7789 display");

    // Configure GPIO for DC and RST
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LCD_PIN_DC) | (1ULL << LCD_PIN_RST),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // Hardware reset
    gpio_set_level(LCD_PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LCD_PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Configure SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = LCD_PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = LCD_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_MAX_TRANSFER_SIZE,
    };
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure SPI device
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = LCD_SPI_CLOCK_HZ,
        .mode = 0,
        .spics_io_num = LCD_PIN_CS,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &s_spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize display with ST7789 commands
    lcd_cmd(ST7789_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));

    lcd_cmd(ST7789_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));

    lcd_cmd(ST7789_COLMOD);
    lcd_data_byte(0x55);  // 16-bit color

    // REQ-SW-038: Portrait mode - no row/column swap (MV=0)
    // MY=1 for correct vertical orientation, MX=0
    lcd_cmd(ST7789_MADCTL);
    lcd_data_byte(MADCTL_MY | MADCTL_RGB);  // Portrait mode orientation

    lcd_cmd(ST7789_INVON);  // Inversion on (normal for this panel)

    lcd_cmd(ST7789_NORON);
    vTaskDelay(pdMS_TO_TICKS(10));

    lcd_cmd(ST7789_DISPON);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Configure backlight PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LCD_PIN_BL,
        .duty = s_brightness,
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);

    // Clear screen to black
    display_fill(0x0000);

    ESP_LOGI(TAG, "Display initialized: %dx%d", LCD_WIDTH, LCD_HEIGHT);
    return ESP_OK;
}

//-----------------------------------------------------------------------------
// Drawing functions
//-----------------------------------------------------------------------------

void display_fill(uint16_t color)
{
    display_fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // Clip to screen bounds
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT || x + w <= 0 || y + h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    lcd_set_window(x, y, x + w - 1, y + h - 1);

    // Swap bytes for SPI (big-endian)
    uint16_t color_swapped = (color >> 8) | (color << 8);

    // Fill buffer with color
    size_t total_pixels = w * h;
    size_t pixels_per_batch = SPI_MAX_TRANSFER_SIZE / 2;

    // Prepare buffer
    uint16_t *buf16 = (uint16_t *)s_spi_buffer;
    size_t fill_count = (pixels_per_batch < total_pixels) ? pixels_per_batch : total_pixels;
    for (size_t i = 0; i < fill_count; i++) {
        buf16[i] = color_swapped;
    }

    gpio_set_level(LCD_PIN_DC, 1);  // Data mode

    size_t remaining = total_pixels;
    while (remaining > 0) {
        size_t batch = (remaining > pixels_per_batch) ? pixels_per_batch : remaining;
        spi_transaction_t t = {
            .length = batch * 16,
            .tx_buffer = s_spi_buffer,
        };
        spi_device_polling_transmit(s_spi, &t);
        remaining -= batch;
    }
}

void display_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;

    lcd_set_window(x, y, x, y);
    uint8_t data[] = {color >> 8, color & 0xFF};
    lcd_data(data, 2);
}

void display_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    display_fill_rect(x, y, w, 1, color);
}

void display_draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    display_fill_rect(x, y, 1, h, color);
}

void display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    display_draw_hline(x, y, w, color);
    display_draw_hline(x, y + h - 1, w, color);
    display_draw_vline(x, y, h, color);
    display_draw_vline(x + w - 1, y, h, color);
}

void display_draw_sprite(int16_t x, int16_t y, int16_t w, int16_t h,
                         const uint16_t *data, uint16_t transparent)
{
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            uint16_t pixel = data[j * w + i];
            if (pixel != transparent) {
                display_draw_pixel(x + i, y + j, pixel);
            }
        }
    }
}

void display_draw_sprite_scaled(int16_t x, int16_t y, int16_t w, int16_t h,
                                const uint16_t *data, uint16_t transparent, uint8_t scale)
{
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            uint16_t pixel = data[j * w + i];
            if (pixel != transparent) {
                display_fill_rect(x + i * scale, y + j * scale, scale, scale, pixel);
            }
        }
    }
}

void display_draw_char(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size)
{
    if (c < 32 || c > 126) c = '?';
    const uint8_t *glyph = &s_font_6x8[(c - 32) * 6];

    for (int8_t i = 0; i < 6; i++) {
        uint8_t line = glyph[i];
        for (int8_t j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                if (size == 1) {
                    display_draw_pixel(x + i, y + j, color);
                } else {
                    display_fill_rect(x + i * size, y + j * size, size, size, color);
                }
            } else if (bg != color) {
                if (size == 1) {
                    display_draw_pixel(x + i, y + j, bg);
                } else {
                    display_fill_rect(x + i * size, y + j * size, size, size, bg);
                }
            }
        }
    }
}

void display_draw_string(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size)
{
    while (*str) {
        display_draw_char(x, y, *str, color, bg, size);
        x += 6 * size;
        str++;
    }
}

void display_draw_number(int16_t x, int16_t y, int32_t num, uint16_t color, uint16_t bg, uint8_t size)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%ld", (long)num);
    display_draw_string(x, y, buf, color, bg, size);
}

void display_set_brightness(uint8_t brightness)
{
    s_brightness = brightness;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

uint8_t display_get_brightness(void)
{
    return s_brightness;
}

void display_start_frame(void)
{
    // Placeholder for double buffering implementation
}

void display_end_frame(void)
{
    // Placeholder for double buffering implementation
}
