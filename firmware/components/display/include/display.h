/**
 * @file display.h
 * @brief ST7789 LCD display driver for TTGO T-Display
 *
 * REQ-SW-030: Display Driver
 * Provides hardware abstraction for the 240x135 TFT display.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Display dimensions
#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  135

/**
 * @brief Initialize the display hardware
 * @return ESP_OK on success
 */
esp_err_t display_init(void);

/**
 * @brief Fill entire screen with a color
 * @param color RGB565 color value
 */
void display_fill(uint16_t color);

/**
 * @brief Fill a rectangular region with a color
 * @param x Start X coordinate
 * @param y Start Y coordinate
 * @param w Width in pixels
 * @param h Height in pixels
 * @param color RGB565 color value
 */
void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color value
 */
void display_draw_pixel(int16_t x, int16_t y, uint16_t color);

/**
 * @brief Draw a horizontal line
 * @param x Start X coordinate
 * @param y Y coordinate
 * @param w Width in pixels
 * @param color RGB565 color value
 */
void display_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color);

/**
 * @brief Draw a vertical line
 * @param x X coordinate
 * @param y Start Y coordinate
 * @param h Height in pixels
 * @param color RGB565 color value
 */
void display_draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color);

/**
 * @brief Draw a rectangle outline
 * @param x Start X coordinate
 * @param y Start Y coordinate
 * @param w Width in pixels
 * @param h Height in pixels
 * @param color RGB565 color value
 */
void display_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Draw a sprite from RGB565 data
 * @param x Top-left X coordinate
 * @param y Top-left Y coordinate
 * @param w Sprite width
 * @param h Sprite height
 * @param data Pointer to RGB565 pixel data
 * @param transparent Transparent color (pixels with this color are skipped)
 */
void display_draw_sprite(int16_t x, int16_t y, int16_t w, int16_t h,
                         const uint16_t *data, uint16_t transparent);

/**
 * @brief Draw a sprite with scaling (2x)
 * @param x Top-left X coordinate
 * @param y Top-left Y coordinate
 * @param w Original sprite width
 * @param h Original sprite height
 * @param data Pointer to RGB565 pixel data
 * @param transparent Transparent color
 */
void display_draw_sprite_scaled(int16_t x, int16_t y, int16_t w, int16_t h,
                                const uint16_t *data, uint16_t transparent, uint8_t scale);

/**
 * @brief Draw a character using built-in font
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Character to draw
 * @param color Text color
 * @param bg Background color
 * @param size Font size multiplier (1 = 6x8 pixels)
 */
void display_draw_char(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Draw a string using built-in font
 * @param x Start X coordinate
 * @param y Start Y coordinate
 * @param str Null-terminated string
 * @param color Text color
 * @param bg Background color
 * @param size Font size multiplier
 */
void display_draw_string(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Draw a number
 * @param x Start X coordinate
 * @param y Start Y coordinate
 * @param num Number to display
 * @param color Text color
 * @param bg Background color
 * @param size Font size multiplier
 */
void display_draw_number(int16_t x, int16_t y, int32_t num, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Set display backlight brightness
 * @param brightness 0-255 (0 = off, 255 = max)
 */
void display_set_brightness(uint8_t brightness);

/**
 * @brief Get current backlight brightness
 * @return Current brightness 0-255
 */
uint8_t display_get_brightness(void);

/**
 * @brief Start a frame buffer transaction (for double buffering)
 */
void display_start_frame(void);

/**
 * @brief End frame buffer transaction and flush to display
 */
void display_end_frame(void);

/**
 * @brief Convert RGB values to RGB565 format
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 * @return RGB565 color value
 */
static inline uint16_t display_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#endif // DISPLAY_H
