/**
 * @file input.c
 * @brief Button input handling implementation
 *
 * REQ-SW-012: Button Input
 * REQ-SW-043: Button Functions
 * Implements debounced button input with short/long press detection.
 *
 * Button mapping (REQ-SW-043):
 * - Left (GPIO 0):  Short press = Down/Previous, Long press (2s) = Back
 * - Right (GPIO 35): Short press = Up/Next, Long press (2s) = Confirm/Select
 */

#include "input.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "input";

// Hardware configuration
#define BUTTON_LEFT_GPIO    0
#define BUTTON_RIGHT_GPIO   35

// Timing configuration
#define DEBOUNCE_MS         50
#define LONG_PRESS_MS       2000
#define REPEAT_DELAY_MS     500
#define REPEAT_RATE_MS      150

// Button GPIO mapping
static const gpio_num_t s_button_gpio[BUTTON_COUNT] = {
    [BUTTON_LEFT] = BUTTON_LEFT_GPIO,
    [BUTTON_RIGHT] = BUTTON_RIGHT_GPIO,
};

// Static state
static button_state_t s_buttons[BUTTON_COUNT] = {0};
static button_callback_t s_callback = NULL;
static uint32_t s_last_update_ms = 0;

/**
 * @brief Get current time in milliseconds
 */
static inline uint32_t get_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

/**
 * @brief Read raw button state (active LOW)
 */
static inline bool read_button_raw(button_id_t button)
{
    return gpio_get_level(s_button_gpio[button]) == 0;
}

esp_err_t input_init(void)
{
    ESP_LOGI(TAG, "Initializing button input");

    // Configure button GPIOs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_LEFT_GPIO) | (1ULL << BUTTON_RIGHT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize button states
    for (int i = 0; i < BUTTON_COUNT; i++) {
        s_buttons[i].is_pressed = false;
        s_buttons[i].was_pressed = false;
        s_buttons[i].press_start_ms = 0;
        s_buttons[i].last_event_ms = 0;
        s_buttons[i].long_press_fired = false;
    }

    s_last_update_ms = get_ms();

    ESP_LOGI(TAG, "Buttons initialized: LEFT=GPIO%d, RIGHT=GPIO%d",
             BUTTON_LEFT_GPIO, BUTTON_RIGHT_GPIO);
    return ESP_OK;
}

void input_register_callback(button_callback_t callback)
{
    s_callback = callback;
}

void input_update(void)
{
    uint32_t now = get_ms();

    for (int i = 0; i < BUTTON_COUNT; i++) {
        button_state_t *btn = &s_buttons[i];

        // Save previous state
        btn->was_pressed = btn->is_pressed;

        // Read current state with debounce
        bool raw_state = read_button_raw(i);

        // Simple debounce: only change state if stable for DEBOUNCE_MS
        static uint32_t s_debounce_start[BUTTON_COUNT] = {0};
        static bool s_debounce_state[BUTTON_COUNT] = {false};

        if (raw_state != s_debounce_state[i]) {
            s_debounce_start[i] = now;
            s_debounce_state[i] = raw_state;
        } else if ((now - s_debounce_start[i]) >= DEBOUNCE_MS) {
            btn->is_pressed = raw_state;
        }

        // Detect press event
        if (btn->is_pressed && !btn->was_pressed) {
            btn->press_start_ms = now;
            btn->long_press_fired = false;
            btn->last_event_ms = now;

            if (s_callback) {
                s_callback((button_id_t)i, BUTTON_EVENT_PRESSED);
            }
        }

        // Detect release event
        if (!btn->is_pressed && btn->was_pressed) {
            if (!btn->long_press_fired) {
                // Short press completed - send click event
                if (s_callback) {
                    s_callback((button_id_t)i, BUTTON_EVENT_CLICK);
                }
            }
            if (s_callback) {
                s_callback((button_id_t)i, BUTTON_EVENT_RELEASED);
            }
        }

        // Detect long press
        if (btn->is_pressed && !btn->long_press_fired) {
            uint32_t hold_time = now - btn->press_start_ms;
            if (hold_time >= LONG_PRESS_MS) {
                btn->long_press_fired = true;
                btn->last_event_ms = now;

                if (s_callback) {
                    s_callback((button_id_t)i, BUTTON_EVENT_LONG_PRESS);
                }
            }
        }

        // Detect repeat (after long press)
        if (btn->is_pressed && btn->long_press_fired) {
            uint32_t since_last = now - btn->last_event_ms;
            if (since_last >= REPEAT_RATE_MS) {
                btn->last_event_ms = now;

                if (s_callback) {
                    s_callback((button_id_t)i, BUTTON_EVENT_REPEAT);
                }
            }
        }
    }

    s_last_update_ms = now;
}

bool input_is_pressed(button_id_t button)
{
    if (button >= BUTTON_COUNT) return false;
    return s_buttons[button].is_pressed;
}

bool input_just_pressed(button_id_t button)
{
    if (button >= BUTTON_COUNT) return false;
    return s_buttons[button].is_pressed && !s_buttons[button].was_pressed;
}

bool input_just_released(button_id_t button)
{
    if (button >= BUTTON_COUNT) return false;
    return !s_buttons[button].is_pressed && s_buttons[button].was_pressed;
}

uint32_t input_get_hold_time(button_id_t button)
{
    if (button >= BUTTON_COUNT) return 0;
    if (!s_buttons[button].is_pressed) return 0;
    return get_ms() - s_buttons[button].press_start_ms;
}

void input_clear_events(void)
{
    for (int i = 0; i < BUTTON_COUNT; i++) {
        s_buttons[i].was_pressed = s_buttons[i].is_pressed;
        s_buttons[i].long_press_fired = true;  // Prevent pending events
    }
}
