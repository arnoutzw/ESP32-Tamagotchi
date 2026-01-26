/**
 * @file main.c
 * @brief ESP32 Tamagotchi - Main entry point
 *
 * A virtual pet game featuring a colorful baby dolphin on the TTGO T-Display.
 * Take care of your dolphin by feeding it, playing with it, and keeping it healthy!
 *
 * Hardware: TTGO T-Display ESP32
 * - 240x135 ST7789 TFT display
 * - 2 push buttons (GPIO 0 and GPIO 35)
 *
 * Controls:
 * - Left button (GPIO 0): Navigate menus
 * - Right button (GPIO 35): Select/Confirm
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

#include "display.h"
#include "input.h"
#include "pet.h"
#include "game.h"
#include "save_manager.h"
#include "sprites.h"
#include "wifi_manager.h"
#include "ota_manager.h"
#include "battery.h"

static const char *TAG = "main";

//=============================================================================
// Configuration
//=============================================================================

#define GAME_TICK_MS        33      // ~30 FPS
#define SAVE_INTERVAL_MS    (5 * 60 * 1000)  // Auto-save every 5 minutes
#define INPUT_POLL_MS       20      // Button polling rate

//=============================================================================
// Static State
//=============================================================================

static uint32_t s_last_save_ms = 0;
static httpd_handle_t s_http_server = NULL;

//=============================================================================
// Button Callback
//=============================================================================

static void button_callback(button_id_t button, button_event_t event)
{
    game_handle_input(button, event);
}

//=============================================================================
// OTA Progress Callback
//=============================================================================

static void ota_progress_callback(int percent)
{
    // Show OTA progress on display
    char buf[32];
    snprintf(buf, sizeof(buf), "Updating: %d%%", percent);
    display_fill_rect(0, 60, 240, 20, 0x0000);
    display_draw_string(50, 65, buf, 0xFFFF, 0x0000, 1);
}

//=============================================================================
// HTTP Server
//=============================================================================

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register OTA handlers
        ota_manager_register_handlers(server);
        ESP_LOGI(TAG, "HTTP server started");
        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
}

//=============================================================================
// Task Functions
//=============================================================================

/**
 * @brief Main game loop task
 */
static void game_task(void *param)
{
    ESP_LOGI(TAG, "Game task started");

    uint32_t last_ms = (uint32_t)(esp_timer_get_time() / 1000);

    while (1) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        uint32_t delta = now - last_ms;
        last_ms = now;

        // Update input
        input_update();

        // Update game state
        game_update(delta);

        // Render frame
        game_render();

        // Auto-save check
        if (game_is_running() && (now - s_last_save_ms) > SAVE_INTERVAL_MS) {
            save_manager_save();
            s_last_save_ms = now;
        }

        // Maintain frame rate
        int32_t sleep_time = GAME_TICK_MS - (int32_t)delta;
        if (sleep_time > 0) {
            vTaskDelay(pdMS_TO_TICKS(sleep_time));
        } else {
            vTaskDelay(1);  // Yield at minimum
        }
    }
}

//=============================================================================
// Main Entry Point
//=============================================================================

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "   ESP32 Tamagotchi - Dolphin");
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Free heap: %lu bytes", (unsigned long)esp_get_free_heap_size());

    // Initialize NVS (required for WiFi and save manager)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing...");
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
    }

    // Initialize OTA manager (check for first boot after OTA)
    ESP_LOGI(TAG, "Initializing OTA manager...");
    ret = ota_manager_init();
    if (ret == ESP_OK) {
        if (ota_manager_is_first_boot()) {
            ESP_LOGI(TAG, "First boot after OTA update");
        }
        ota_manager_set_progress_callback(ota_progress_callback);
    }

    // Initialize battery monitor (REQ-SW-035)
    ESP_LOGI(TAG, "Initializing battery monitor...");
    ret = battery_init();
    if (ret == ESP_OK) {
        battery_status_t batt;
        battery_get_status(&batt);
        ESP_LOGI(TAG, "Battery: %.2fV (%d%%)", batt.voltage, batt.soc_percent);
    } else {
        ESP_LOGW(TAG, "Battery monitor init failed: %s", esp_err_to_name(ret));
    }

    // Initialize display
    ESP_LOGI(TAG, "Initializing display...");
    ret = display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display init failed!");
        return;
    }

    // Show startup message
    display_fill(0x0000);
    display_draw_string(30, 50, "DOLPHIN PET", 0xFFFF, 0x0000, 2);
    display_draw_string(60, 90, "Loading...", 0xBDF7, 0x0000, 1);

    // Initialize input
    ESP_LOGI(TAG, "Initializing input...");
    ret = input_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Input init failed!");
        return;
    }
    input_register_callback(button_callback);

    // Initialize pet system
    ESP_LOGI(TAG, "Initializing pet system...");
    ret = pet_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Pet init failed!");
        return;
    }

    // Initialize save manager
    ESP_LOGI(TAG, "Initializing save manager...");
    ret = save_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Save manager init failed, saves disabled");
    }

    // Initialize game
    ESP_LOGI(TAG, "Initializing game...");
    ret = game_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Game init failed!");
        return;
    }

    // Try to load saved game
    if (save_manager_exists()) {
        ESP_LOGI(TAG, "Loading saved game...");
        ret = save_manager_load();
        if (ret == ESP_OK) {
            // Calculate offline time decay
            uint32_t offline_min = save_manager_get_offline_minutes();
            if (offline_min > 0) {
                ESP_LOGI(TAG, "Applying %lu minutes of offline time", (unsigned long)offline_min);
                pet_apply_time_away(offline_min);
            }

            // Skip splash, go directly to game
            game_handle_input(BUTTON_RIGHT, BUTTON_EVENT_CLICK);
        } else {
            ESP_LOGW(TAG, "Failed to load save, starting new game");
        }
    }

    s_last_save_ms = (uint32_t)(esp_timer_get_time() / 1000);

    // Initialize WiFi with STA mode preferred, AP mode as fallback
    // REQ-SW-036: WiFi Connectivity with AP Fallback
    ESP_LOGI(TAG, "Initializing WiFi...");
    ret = wifi_manager_init();
    if (ret == ESP_OK) {
        bool wifi_connected = false;

        // Try STA mode first if credentials are configured
        if (WIFI_STA_CONFIGURED) {
            ESP_LOGI(TAG, "STA credentials configured, attempting connection to %s...", WIFI_STA_SSID);
            ret = wifi_manager_start_sta(WIFI_STA_SSID, WIFI_STA_PASS);
            if (ret == ESP_OK && wifi_manager_is_connected()) {
                char ip[16];
                wifi_manager_get_ip(ip);
                ESP_LOGI(TAG, "WiFi STA connected, IP: %s", ip);
                wifi_connected = true;
            } else {
                ESP_LOGW(TAG, "STA connection failed, falling back to AP mode");
                wifi_manager_stop();
            }
        }

        // Fall back to AP mode if STA not configured or failed
        if (!wifi_connected) {
            ret = wifi_manager_start_ap();
            if (ret == ESP_OK) {
                char ip[16];
                wifi_manager_get_ip(ip);
                ESP_LOGI(TAG, "WiFi AP started, IP: %s", ip);
                ESP_LOGI(TAG, "Connect to SSID: %s, Password: %s", WIFI_AP_SSID, WIFI_AP_PASS);
            }
        }

        // Start HTTP server for OTA (works in both modes)
        s_http_server = start_webserver();
    }

    // Mark firmware as valid after successful initialization
    // This prevents rollback if we get this far
    ota_manager_mark_valid();

    ESP_LOGI(TAG, "Free heap after init: %lu bytes", (unsigned long)esp_get_free_heap_size());
    ESP_LOGI(TAG, "Starting game loop...");

    // Create game task
    xTaskCreatePinnedToCore(
        game_task,
        "game_task",
        8192,           // Stack size
        NULL,
        5,              // Priority
        NULL,
        0               // Core 0
    );

    // Main task can exit, FreeRTOS will keep running
    ESP_LOGI(TAG, "Main task complete, game running");
}
