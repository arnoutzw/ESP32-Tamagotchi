/**
 * @file wifi_manager.c
 * @brief WiFi management implementation
 *
 * REQ-SW-034: OTA Updates - WiFi connectivity for OTA
 */

#include "wifi_manager.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "wifi_mgr";

//=============================================================================
// Private Variables
//=============================================================================

static wifi_state_t s_wifi_state = WIFI_STATE_DISABLED;
static esp_netif_t *s_netif_ap = NULL;
static esp_netif_t *s_netif_sta = NULL;
static EventGroupHandle_t s_wifi_event_group = NULL;
static int s_retry_count = 0;

// Event bits
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

// NVS namespace for WiFi credentials
#define NVS_WIFI_NAMESPACE  "wifi"
#define NVS_KEY_SSID        "ssid"
#define NVS_KEY_PASS        "pass"

//=============================================================================
// Event Handlers
//=============================================================================

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
                ESP_LOGI(TAG, "Station connected, AID=%d", event->aid);
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
                ESP_LOGI(TAG, "Station disconnected, AID=%d", event->aid);
                break;
            }
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "STA started, connecting...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if (s_retry_count < WIFI_STA_MAX_RETRY) {
                    esp_wifi_connect();
                    s_retry_count++;
                    ESP_LOGW(TAG, "Retry connection (%d/%d)", s_retry_count, WIFI_STA_MAX_RETRY);
                } else {
                    if (s_wifi_event_group) {
                        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                    }
                    s_wifi_state = WIFI_STATE_STA_DISCONNECTED;
                    ESP_LOGE(TAG, "Connection failed after %d retries", WIFI_STA_MAX_RETRY);
                }
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            s_retry_count = 0;
            s_wifi_state = WIFI_STATE_STA_CONNECTED;
            if (s_wifi_event_group) {
                xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            }
        }
    }
}

//=============================================================================
// Public Functions
//=============================================================================

esp_err_t wifi_manager_init(void)
{
    esp_err_t ret;

    // Create event group
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    // Initialize TCP/IP stack
    ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create default event loop
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Event loop creation failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize WiFi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register event handlers
    ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &wifi_event_handler, NULL, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WIFI_EVENT handler: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler, NULL, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register IP_EVENT handler: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "WiFi manager initialized");
    return ESP_OK;
}

esp_err_t wifi_manager_start_ap(void)
{
    esp_err_t ret;

    // Create AP netif if not exists
    if (s_netif_ap == NULL) {
        s_netif_ap = esp_netif_create_default_wifi_ap();
        if (s_netif_ap == NULL) {
            ESP_LOGE(TAG, "Failed to create AP netif");
            return ESP_FAIL;
        }
    }

    // Configure AP
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_AP_CHANNEL,
            .password = WIFI_AP_PASS,
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    // Use open auth if no password
    if (strlen(WIFI_AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_wifi_state = WIFI_STATE_AP_ACTIVE;
    ESP_LOGI(TAG, "AP started: SSID=%s, Channel=%d", WIFI_AP_SSID, WIFI_AP_CHANNEL);

    return ESP_OK;
}

esp_err_t wifi_manager_start_sta(const char *ssid, const char *password)
{
    if (ssid == NULL || strlen(ssid) == 0) {
        ESP_LOGE(TAG, "Invalid SSID");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;

    // Create STA netif if not exists
    if (s_netif_sta == NULL) {
        s_netif_sta = esp_netif_create_default_wifi_sta();
        if (s_netif_sta == NULL) {
            ESP_LOGE(TAG, "Failed to create STA netif");
            return ESP_FAIL;
        }
    }

    // Configure STA
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password) {
        strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_retry_count = 0;
    s_wifi_state = WIFI_STATE_STA_CONNECTING;

    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);

    // Wait for connection or failure
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                            pdFALSE, pdFALSE,
                                            pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to %s", ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to %s", ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "Connection timeout");
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t wifi_manager_stop(void)
{
    esp_err_t ret = esp_wifi_stop();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_stop failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_wifi_state = WIFI_STATE_DISABLED;
    ESP_LOGI(TAG, "WiFi stopped");
    return ESP_OK;
}

wifi_state_t wifi_manager_get_state(void)
{
    return s_wifi_state;
}

bool wifi_manager_is_connected(void)
{
    return s_wifi_state == WIFI_STATE_STA_CONNECTED;
}

esp_err_t wifi_manager_get_ip(char *ip_str)
{
    if (ip_str == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = (s_wifi_state == WIFI_STATE_AP_ACTIVE) ? s_netif_ap : s_netif_sta;

    if (netif == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
    if (ret != ESP_OK) {
        return ret;
    }

    sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));
    return ESP_OK;
}

esp_err_t wifi_manager_save_credentials(const wifi_credentials_t *creds)
{
    if (creds == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_WIFI_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_str(handle, NVS_KEY_SSID, creds->ssid);
    if (ret != ESP_OK) {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_set_str(handle, NVS_KEY_PASS, creds->password);
    if (ret != ESP_OK) {
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    nvs_close(handle);

    ESP_LOGI(TAG, "WiFi credentials saved");
    return ret;
}

esp_err_t wifi_manager_load_credentials(wifi_credentials_t *creds)
{
    if (creds == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_WIFI_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        return ret;
    }

    size_t ssid_len = sizeof(creds->ssid);
    ret = nvs_get_str(handle, NVS_KEY_SSID, creds->ssid, &ssid_len);
    if (ret != ESP_OK) {
        nvs_close(handle);
        return ret;
    }

    size_t pass_len = sizeof(creds->password);
    ret = nvs_get_str(handle, NVS_KEY_PASS, creds->password, &pass_len);
    if (ret != ESP_OK) {
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    ESP_LOGI(TAG, "WiFi credentials loaded: SSID=%s", creds->ssid);
    return ESP_OK;
}
