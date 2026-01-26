/**
 * @file battery.h
 * @brief Battery voltage monitoring for ESP32 Tamagotchi
 *
 * REQ-SW-035: Battery Indicator
 *
 * Monitors battery voltage via ADC and estimates State of Charge (SoC)
 * for LiPo batteries on TTGO T-Display hardware.
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>
#include "esp_err.h"

//=============================================================================
// Configuration
//=============================================================================

// TTGO T-Display battery ADC pin (GPIO 34 with voltage divider)
#define BATTERY_ADC_GPIO        34
#define BATTERY_ADC_CHANNEL     ADC1_CHANNEL_6  // GPIO 34

// Voltage divider ratio (100K + 100K divider on T-Display)
// Measured voltage = ADC voltage * 2
#define BATTERY_VOLTAGE_DIVIDER 2.0f

// LiPo battery voltage thresholds
#define BATTERY_VOLTAGE_FULL    4.2f    // 100% SoC
#define BATTERY_VOLTAGE_NOMINAL 3.7f    // ~50% SoC
#define BATTERY_VOLTAGE_LOW     3.4f    // ~20% SoC (warning)
#define BATTERY_VOLTAGE_EMPTY   3.0f    // 0% SoC (critical)

// ADC configuration
#define BATTERY_ADC_ATTEN       ADC_ATTEN_DB_12   // 0-3.3V range
#define BATTERY_ADC_WIDTH       ADC_WIDTH_BIT_12  // 12-bit resolution
#define BATTERY_SAMPLES         16                // Averaging samples

// Update interval
#define BATTERY_UPDATE_MS       5000    // Update every 5 seconds

//=============================================================================
// Types
//=============================================================================

typedef enum {
    BATTERY_LEVEL_FULL,     // 80-100%
    BATTERY_LEVEL_HIGH,     // 60-80%
    BATTERY_LEVEL_MEDIUM,   // 40-60%
    BATTERY_LEVEL_LOW,      // 20-40%
    BATTERY_LEVEL_CRITICAL  // 0-20%
} battery_level_t;

typedef struct {
    float voltage;          // Current voltage in volts
    uint8_t soc_percent;    // State of charge (0-100%)
    battery_level_t level;  // Discrete battery level
    bool charging;          // True if USB power detected (future)
} battery_status_t;

//=============================================================================
// Public API
//=============================================================================

/**
 * @brief Initialize battery monitoring
 * @return ESP_OK on success
 */
esp_err_t battery_init(void);

/**
 * @brief Read current battery voltage
 * @return Battery voltage in volts
 */
float battery_read_voltage(void);

/**
 * @brief Get battery state of charge
 * @return SoC percentage (0-100)
 */
uint8_t battery_get_soc(void);

/**
 * @brief Get discrete battery level
 * @return Battery level enum
 */
battery_level_t battery_get_level(void);

/**
 * @brief Get full battery status
 * @param status Pointer to status structure to fill
 */
void battery_get_status(battery_status_t *status);

/**
 * @brief Check if battery is low (needs warning)
 * @return true if battery level is LOW or CRITICAL
 */
bool battery_is_low(void);

#endif // BATTERY_H
