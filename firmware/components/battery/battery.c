/**
 * @file battery.c
 * @brief Battery voltage monitoring implementation
 *
 * REQ-SW-035: Battery Indicator
 *
 * Uses ESP32 ADC to measure battery voltage through the voltage divider
 * on the TTGO T-Display board.
 */

#include "battery.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

static const char *TAG = "battery";

//=============================================================================
// Static Variables
//=============================================================================

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;
static bool s_calibrated = false;

// Cached values
static float s_voltage = 0.0f;
static uint8_t s_soc = 0;
static battery_level_t s_level = BATTERY_LEVEL_MEDIUM;

//=============================================================================
// Private Functions
//=============================================================================

/**
 * @brief Convert voltage to State of Charge percentage
 *
 * Uses a piecewise linear approximation of LiPo discharge curve:
 * - 4.2V = 100%
 * - 3.7V = 50% (nominal)
 * - 3.0V = 0%
 */
static uint8_t voltage_to_soc(float voltage)
{
    if (voltage >= BATTERY_VOLTAGE_FULL) {
        return 100;
    } else if (voltage <= BATTERY_VOLTAGE_EMPTY) {
        return 0;
    } else if (voltage >= BATTERY_VOLTAGE_NOMINAL) {
        // Upper half: 3.7V (50%) to 4.2V (100%)
        float range = BATTERY_VOLTAGE_FULL - BATTERY_VOLTAGE_NOMINAL;
        float offset = voltage - BATTERY_VOLTAGE_NOMINAL;
        return 50 + (uint8_t)(50.0f * offset / range);
    } else {
        // Lower half: 3.0V (0%) to 3.7V (50%)
        float range = BATTERY_VOLTAGE_NOMINAL - BATTERY_VOLTAGE_EMPTY;
        float offset = voltage - BATTERY_VOLTAGE_EMPTY;
        return (uint8_t)(50.0f * offset / range);
    }
}

/**
 * @brief Convert SoC to discrete level
 */
static battery_level_t soc_to_level(uint8_t soc)
{
    if (soc >= 80) return BATTERY_LEVEL_FULL;
    if (soc >= 60) return BATTERY_LEVEL_HIGH;
    if (soc >= 40) return BATTERY_LEVEL_MEDIUM;
    if (soc >= 20) return BATTERY_LEVEL_LOW;
    return BATTERY_LEVEL_CRITICAL;
}

/**
 * @brief Initialize ADC calibration
 */
static bool init_calibration(void)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = BATTERY_ADC_ATTEN,
        .bitwidth = BATTERY_ADC_WIDTH,
    };
    if (adc_cali_create_scheme_curve_fitting(&cali_config, &s_cali_handle) == ESP_OK) {
        ESP_LOGI(TAG, "ADC calibration: curve fitting");
        return true;
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = BATTERY_ADC_ATTEN,
        .bitwidth = BATTERY_ADC_WIDTH,
    };
    if (adc_cali_create_scheme_line_fitting(&cali_config, &s_cali_handle) == ESP_OK) {
        ESP_LOGI(TAG, "ADC calibration: line fitting");
        return true;
    }
#endif

    ESP_LOGW(TAG, "ADC calibration not supported");
    return false;
}

//=============================================================================
// Public Functions
//=============================================================================

esp_err_t battery_init(void)
{
    ESP_LOGI(TAG, "Initializing battery monitor");

    // Configure ADC oneshot mode
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    esp_err_t ret = adc_oneshot_new_unit(&init_cfg, &s_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC unit: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure ADC channel
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = BATTERY_ADC_ATTEN,
        .bitwidth = BATTERY_ADC_WIDTH,
    };
    ret = adc_oneshot_config_channel(s_adc_handle, BATTERY_ADC_CHANNEL, &chan_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to config ADC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize calibration
    s_calibrated = init_calibration();

    // Take initial reading
    s_voltage = battery_read_voltage();
    s_soc = voltage_to_soc(s_voltage);
    s_level = soc_to_level(s_soc);

    ESP_LOGI(TAG, "Battery: %.2fV, %d%%, level=%d", s_voltage, s_soc, s_level);

    return ESP_OK;
}

float battery_read_voltage(void)
{
    if (s_adc_handle == NULL) {
        return 0.0f;
    }

    // Take multiple samples and average
    int32_t raw_sum = 0;
    for (int i = 0; i < BATTERY_SAMPLES; i++) {
        int raw;
        if (adc_oneshot_read(s_adc_handle, BATTERY_ADC_CHANNEL, &raw) == ESP_OK) {
            raw_sum += raw;
        }
    }
    int raw_avg = raw_sum / BATTERY_SAMPLES;

    // Convert to voltage
    float voltage;
    if (s_calibrated && s_cali_handle != NULL) {
        int mv;
        adc_cali_raw_to_voltage(s_cali_handle, raw_avg, &mv);
        voltage = (float)mv / 1000.0f;
    } else {
        // Fallback: assume 3.3V reference, 12-bit ADC
        voltage = (float)raw_avg * 3.3f / 4095.0f;
    }

    // Apply voltage divider correction
    voltage *= BATTERY_VOLTAGE_DIVIDER;

    // Update cached values
    s_voltage = voltage;
    s_soc = voltage_to_soc(voltage);
    s_level = soc_to_level(s_soc);

    return voltage;
}

uint8_t battery_get_soc(void)
{
    return s_soc;
}

battery_level_t battery_get_level(void)
{
    return s_level;
}

void battery_get_status(battery_status_t *status)
{
    if (status == NULL) {
        return;
    }

    status->voltage = s_voltage;
    status->soc_percent = s_soc;
    status->level = s_level;
    status->charging = false;  // TODO: Detect USB power
}

bool battery_is_low(void)
{
    return (s_level == BATTERY_LEVEL_LOW || s_level == BATTERY_LEVEL_CRITICAL);
}
