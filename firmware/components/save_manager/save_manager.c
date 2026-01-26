/**
 * @file save_manager.c
 * @brief NVS save/load implementation
 *
 * REQ-SW-020: Save State
 * REQ-SW-021: Time Tracking
 */

#include "save_manager.h"
#include "pet.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "save";

#define NVS_NAMESPACE       "tamagotchi"
#define NVS_KEY_PET_STATE   "pet_state"
#define NVS_KEY_TIMESTAMP   "last_save"
#define NVS_KEY_VERSION     "save_ver"

#define SAVE_VERSION        1

// Packed save structure (to minimize NVS usage)
typedef struct __attribute__((packed)) {
    uint8_t version;
    uint8_t hunger;
    uint8_t happiness;
    uint8_t health;
    uint8_t energy;
    uint8_t weight;
    uint8_t discipline;
    uint8_t stage;
    uint32_t age_minutes;
    uint8_t is_sick;
    uint8_t poop_count;
    uint8_t is_sleeping;
    uint16_t games_won;
    uint16_t games_played;
    uint16_t times_fed;
    uint16_t times_played;
    uint16_t times_cleaned;
    uint16_t times_medicated;
} save_data_t;

static nvs_handle_t s_nvs_handle = 0;
static uint32_t s_last_save_time = 0;

/**
 * @brief Get current time in milliseconds (approximate)
 */
static inline uint32_t get_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

esp_err_t save_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing NVS save manager");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS erase failed: %s", esp_err_to_name(ret));
            return ret;
        }
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Open NVS namespace
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &s_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_last_save_time = get_ms();

    ESP_LOGI(TAG, "Save manager initialized");
    return ESP_OK;
}

esp_err_t save_manager_save(void)
{
    if (s_nvs_handle == 0) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    const pet_state_t *pet = pet_get_state();

    // Pack pet state into save structure
    save_data_t save = {
        .version = SAVE_VERSION,
        .hunger = pet->hunger,
        .happiness = pet->happiness,
        .health = pet->health,
        .energy = pet->energy,
        .weight = pet->weight,
        .discipline = pet->discipline,
        .stage = (uint8_t)pet->stage,
        .age_minutes = pet->age_minutes,
        .is_sick = pet->is_sick ? 1 : 0,
        .poop_count = pet->poop_count,
        .is_sleeping = pet->is_sleeping ? 1 : 0,
        .games_won = pet->games_won,
        .games_played = pet->games_played,
        .times_fed = pet->times_fed,
        .times_played = pet->times_played,
        .times_cleaned = pet->times_cleaned,
        .times_medicated = pet->times_medicated,
    };

    // Write to NVS
    esp_err_t ret = nvs_set_blob(s_nvs_handle, NVS_KEY_PET_STATE, &save, sizeof(save));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write pet state: %s", esp_err_to_name(ret));
        return ret;
    }

    // Write timestamp (using esp_timer based time)
    uint32_t now = get_ms() / 1000;  // Seconds since boot
    ret = nvs_set_u32(s_nvs_handle, NVS_KEY_TIMESTAMP, now);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write timestamp: %s", esp_err_to_name(ret));
        return ret;
    }

    // Commit
    ret = nvs_commit(s_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_last_save_time = get_ms();
    ESP_LOGI(TAG, "Game saved (age: %lu min)", (unsigned long)pet->age_minutes);

    return ESP_OK;
}

esp_err_t save_manager_load(void)
{
    if (s_nvs_handle == 0) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Read save data
    save_data_t save;
    size_t size = sizeof(save);
    esp_err_t ret = nvs_get_blob(s_nvs_handle, NVS_KEY_PET_STATE, &save, &size);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No save data found");
        return ESP_ERR_NOT_FOUND;
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read save data: %s", esp_err_to_name(ret));
        return ret;
    }

    // Check version
    if (save.version != SAVE_VERSION) {
        ESP_LOGW(TAG, "Save version mismatch: %d vs %d", save.version, SAVE_VERSION);
        return ESP_ERR_INVALID_VERSION;
    }

    // Restore pet state
    pet_state_t *pet = pet_get_state_mutable();

    pet->hunger = save.hunger;
    pet->happiness = save.happiness;
    pet->health = save.health;
    pet->energy = save.energy;
    pet->weight = save.weight;
    pet->discipline = save.discipline;
    pet->stage = (pet_stage_t)save.stage;
    pet->age_minutes = save.age_minutes;
    pet->is_sick = save.is_sick != 0;
    pet->poop_count = save.poop_count;
    pet->has_poop = save.poop_count > 0;
    pet->is_sleeping = save.is_sleeping != 0;
    pet->games_won = save.games_won;
    pet->games_played = save.games_played;
    pet->times_fed = save.times_fed;
    pet->times_played = save.times_played;
    pet->times_cleaned = save.times_cleaned;
    pet->times_medicated = save.times_medicated;

    ESP_LOGI(TAG, "Game loaded (age: %lu min, stage: %d)",
             (unsigned long)pet->age_minutes, pet->stage);

    return ESP_OK;
}

bool save_manager_exists(void)
{
    if (s_nvs_handle == 0) return false;

    size_t size = 0;
    esp_err_t ret = nvs_get_blob(s_nvs_handle, NVS_KEY_PET_STATE, NULL, &size);
    return (ret == ESP_OK && size > 0);
}

esp_err_t save_manager_delete(void)
{
    if (s_nvs_handle == 0) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = nvs_erase_key(s_nvs_handle, NVS_KEY_PET_STATE);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to delete save: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_erase_key(s_nvs_handle, NVS_KEY_TIMESTAMP);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        // Non-fatal
    }

    nvs_commit(s_nvs_handle);

    ESP_LOGI(TAG, "Save data deleted");
    return ESP_OK;
}

uint32_t save_manager_get_offline_minutes(void)
{
    if (s_nvs_handle == 0) return 0;

    uint32_t last_timestamp = 0;
    esp_err_t ret = nvs_get_u32(s_nvs_handle, NVS_KEY_TIMESTAMP, &last_timestamp);
    if (ret != ESP_OK) {
        return 0;
    }

    // Note: Without RTC, we can't accurately track offline time
    // This is a simplified implementation that uses boot time as reference
    // In a real implementation, you'd use an RTC or NTP

    // For now, return 0 (no offline decay)
    // A real implementation would compare RTC time with saved timestamp
    return 0;
}

void save_manager_update_timestamp(void)
{
    s_last_save_time = get_ms();
}
