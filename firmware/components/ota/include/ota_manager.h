/**
 * @file ota_manager.h
 * @brief OTA update management for ESP32 Tamagotchi
 *
 * REQ-SW-034: OTA Updates
 *
 * Provides HTTP-based OTA update functionality with automatic rollback
 * on boot failure.
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"
#include "esp_http_server.h"
#include "config_secrets.h"

//=============================================================================
// Configuration (from config/secrets.yaml via config_secrets.h)
//=============================================================================

#define OTA_PASSWORD            CONFIG_OTA_PASSWORD
#define OTA_RECV_TIMEOUT_MS     30000
#define OTA_BUF_SIZE            1024

//=============================================================================
// Types
//=============================================================================

typedef enum {
    OTA_STATE_IDLE,
    OTA_STATE_RECEIVING,
    OTA_STATE_VERIFYING,
    OTA_STATE_REBOOTING,
    OTA_STATE_ERROR
} ota_state_t;

typedef void (*ota_progress_cb_t)(int percent);

//=============================================================================
// Public API
//=============================================================================

/**
 * @brief Initialize OTA subsystem
 * @return ESP_OK on success
 */
esp_err_t ota_manager_init(void);

/**
 * @brief Register OTA HTTP handlers with a server
 * @param server HTTP server handle
 * @return ESP_OK on success
 */
esp_err_t ota_manager_register_handlers(httpd_handle_t server);

/**
 * @brief Mark current firmware as valid (call after successful boot)
 *
 * This must be called after the application has verified it's running
 * correctly. If not called before a crash/reboot, the bootloader will
 * roll back to the previous firmware.
 *
 * @return ESP_OK on success
 */
esp_err_t ota_manager_mark_valid(void);

/**
 * @brief Get current OTA state
 * @return Current OTA state
 */
ota_state_t ota_manager_get_state(void);

/**
 * @brief Set progress callback for OTA updates
 * @param cb Callback function (percent: 0-100)
 */
void ota_manager_set_progress_callback(ota_progress_cb_t cb);

/**
 * @brief Get running firmware version string
 * @return Version string (e.g., "v1.0.0")
 */
const char *ota_manager_get_version(void);

/**
 * @brief Check if this is first boot after OTA update
 * @return true if first boot after OTA
 */
bool ota_manager_is_first_boot(void);

#endif // OTA_MANAGER_H
