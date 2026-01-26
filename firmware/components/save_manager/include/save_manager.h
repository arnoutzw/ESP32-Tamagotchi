/**
 * @file save_manager.h
 * @brief Save/Load game state to NVS for ESP32 Tamagotchi
 *
 * REQ-SW-020: Save State
 * REQ-SW-021: Time Tracking
 * Persists pet state across power cycles.
 */

#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Initialize save manager and NVS
 * @return ESP_OK on success
 */
esp_err_t save_manager_init(void);

/**
 * @brief Save current pet state to NVS
 * @return ESP_OK on success
 */
esp_err_t save_manager_save(void);

/**
 * @brief Load pet state from NVS
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no save exists
 */
esp_err_t save_manager_load(void);

/**
 * @brief Check if a save file exists
 * @return true if save exists
 */
bool save_manager_exists(void);

/**
 * @brief Delete save data (for new game)
 * @return ESP_OK on success
 */
esp_err_t save_manager_delete(void);

/**
 * @brief Get time since last save (for offline decay)
 * @return Minutes since last save, 0 if no save or error
 */
uint32_t save_manager_get_offline_minutes(void);

/**
 * @brief Update the last save timestamp to now
 */
void save_manager_update_timestamp(void);

#endif // SAVE_MANAGER_H
