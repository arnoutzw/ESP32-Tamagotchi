/**
 * @file wifi_manager.h
 * @brief WiFi management for ESP32 Tamagotchi
 *
 * REQ-SW-034: OTA Updates - WiFi connectivity for OTA
 * REQ-SW-041: WiFi Features (stretch goal)
 *
 * Supports both AP mode (for initial setup/OTA) and STA mode (for network access).
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"
#include "config_secrets.h"

//=============================================================================
// Configuration (from config/secrets.yaml via config_secrets.h)
//=============================================================================

#define WIFI_AP_SSID            CONFIG_WIFI_AP_SSID
#define WIFI_AP_PASS            CONFIG_WIFI_AP_PASSWORD
#define WIFI_AP_CHANNEL         1
#define WIFI_AP_MAX_CONN        2

#define WIFI_STA_SSID           CONFIG_WIFI_STA_SSID
#define WIFI_STA_PASS           CONFIG_WIFI_STA_PASSWORD
#define WIFI_STA_CONFIGURED     CONFIG_WIFI_STA_CONFIGURED

#define WIFI_STA_MAX_RETRY      5
#define WIFI_CONNECT_TIMEOUT_MS 10000

//=============================================================================
// Types
//=============================================================================

typedef enum {
    WIFI_STATE_DISABLED,
    WIFI_STATE_AP_ACTIVE,
    WIFI_STATE_STA_CONNECTING,
    WIFI_STATE_STA_CONNECTED,
    WIFI_STATE_STA_DISCONNECTED,
    WIFI_STATE_ERROR
} wifi_state_t;

typedef struct {
    char ssid[33];
    char password[65];
} wifi_credentials_t;

//=============================================================================
// Public API
//=============================================================================

/**
 * @brief Initialize WiFi subsystem
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Start WiFi in Access Point mode
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_start_ap(void);

/**
 * @brief Start WiFi in Station mode
 * @param ssid Network SSID to connect to
 * @param password Network password
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_start_sta(const char *ssid, const char *password);

/**
 * @brief Stop WiFi
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_stop(void);

/**
 * @brief Get current WiFi state
 * @return Current WiFi state
 */
wifi_state_t wifi_manager_get_state(void);

/**
 * @brief Check if WiFi is connected (STA mode)
 * @return true if connected
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Get current IP address (as string)
 * @param ip_str Buffer to store IP string (min 16 bytes)
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_get_ip(char *ip_str);

/**
 * @brief Save WiFi credentials to NVS
 * @param creds Credentials to save
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_save_credentials(const wifi_credentials_t *creds);

/**
 * @brief Load WiFi credentials from NVS
 * @param creds Buffer to store loaded credentials
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no saved credentials
 */
esp_err_t wifi_manager_load_credentials(wifi_credentials_t *creds);

#endif // WIFI_MANAGER_H
