/**
 * @file input.h
 * @brief Button input handling for TTGO T-Display
 *
 * REQ-SW-012: Button Input
 * Two-button control scheme with debouncing and long press detection.
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Button identifiers
 */
typedef enum {
    BUTTON_LEFT = 0,    // GPIO 0 - Navigate/Scroll
    BUTTON_RIGHT = 1,   // GPIO 35 - Select/Confirm
    BUTTON_COUNT
} button_id_t;

/**
 * @brief Button event types
 */
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESSED,       // Initial press
    BUTTON_EVENT_RELEASED,      // Released after short press
    BUTTON_EVENT_LONG_PRESS,    // Held for long press threshold
    BUTTON_EVENT_CLICK,         // Short press completed
    BUTTON_EVENT_REPEAT,        // Repeated while held (after long press)
} button_event_t;

/**
 * @brief Button state information
 */
typedef struct {
    bool is_pressed;            // Current state
    bool was_pressed;           // Previous state
    uint32_t press_start_ms;    // When button was pressed
    uint32_t last_event_ms;     // Last event time
    bool long_press_fired;      // Long press event already sent
} button_state_t;

/**
 * @brief Button event callback function type
 * @param button Which button triggered the event
 * @param event Type of button event
 */
typedef void (*button_callback_t)(button_id_t button, button_event_t event);

/**
 * @brief Initialize button input system
 * @return ESP_OK on success
 */
esp_err_t input_init(void);

/**
 * @brief Register callback for button events
 * @param callback Function to call on button events (NULL to unregister)
 */
void input_register_callback(button_callback_t callback);

/**
 * @brief Update button state (call from main loop or task)
 *
 * This processes button state changes and fires callbacks.
 * Should be called at regular intervals (e.g., every 20ms).
 */
void input_update(void);

/**
 * @brief Check if button is currently pressed
 * @param button Which button to check
 * @return true if pressed
 */
bool input_is_pressed(button_id_t button);

/**
 * @brief Check if button was just pressed (edge detection)
 * @param button Which button to check
 * @return true if just pressed this update cycle
 */
bool input_just_pressed(button_id_t button);

/**
 * @brief Check if button was just released (edge detection)
 * @param button Which button to check
 * @return true if just released this update cycle
 */
bool input_just_released(button_id_t button);

/**
 * @brief Get how long button has been held (in ms)
 * @param button Which button to check
 * @return Hold time in milliseconds, 0 if not pressed
 */
uint32_t input_get_hold_time(button_id_t button);

/**
 * @brief Clear all pending button events
 *
 * Useful when transitioning between game states.
 */
void input_clear_events(void);

#endif // INPUT_H
