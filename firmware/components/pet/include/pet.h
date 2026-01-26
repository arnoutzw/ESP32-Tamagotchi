/**
 * @file pet.h
 * @brief Pet state management for ESP32 Tamagotchi
 *
 * REQ-SW-001: Pet State System
 * REQ-SW-002: Pet Life Stages
 * Manages all pet attributes, stat decay, and life stage progression.
 */

#ifndef PET_H
#define PET_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

//=============================================================================
// Constants
//=============================================================================

#define PET_STAT_MIN    0
#define PET_STAT_MAX    100
#define PET_CRITICAL    20      // Below this, stat is critical
#define PET_OVERFEED    90      // Above this, overfeeding penalty

//=============================================================================
// Types
//=============================================================================

/**
 * @brief Pet life stages
 */
typedef enum {
    PET_STAGE_EGG = 0,      // Hatching phase (first few minutes)
    PET_STAGE_BABY,         // 0-2 days
    PET_STAGE_CHILD,        // 3-6 days
    PET_STAGE_TEEN,         // 7-13 days
    PET_STAGE_ADULT,        // 14+ days
    PET_STAGE_DEAD,         // Game over
} pet_stage_t;

/**
 * @brief Pet mood states (affects sprite/animation)
 */
typedef enum {
    PET_MOOD_HAPPY = 0,
    PET_MOOD_NORMAL,
    PET_MOOD_SAD,
    PET_MOOD_HUNGRY,
    PET_MOOD_SLEEPY,
    PET_MOOD_SICK,
    PET_MOOD_SLEEPING,
} pet_mood_t;

/**
 * @brief Pet activity states
 */
typedef enum {
    PET_ACTIVITY_IDLE = 0,
    PET_ACTIVITY_EATING,
    PET_ACTIVITY_PLAYING,
    PET_ACTIVITY_SLEEPING,
    PET_ACTIVITY_SICK,
    PET_ACTIVITY_HAPPY,     // Celebrating
    PET_ACTIVITY_HATCHING,
} pet_activity_t;

/**
 * @brief Food types
 */
typedef enum {
    FOOD_FISH = 0,      // Main meal
    FOOD_SHRIMP,        // Snack/treat
} food_type_t;

/**
 * @brief Complete pet state structure
 */
typedef struct {
    // Core stats (0-100)
    uint8_t hunger;         // 100 = full, 0 = starving
    uint8_t happiness;      // 100 = ecstatic, 0 = depressed
    uint8_t health;         // 100 = perfect, 0 = dead
    uint8_t energy;         // 100 = energetic, 0 = exhausted

    // Secondary stats
    uint8_t weight;         // 1-99, affects sprite variant
    uint8_t discipline;     // 0-100, affects behavior

    // Life tracking
    pet_stage_t stage;      // Current life stage
    uint32_t age_minutes;   // Total age in minutes
    uint32_t birth_time;    // Unix timestamp of birth

    // State flags
    pet_mood_t mood;
    pet_activity_t activity;
    bool is_sick;
    bool has_poop;          // Poop needs cleaning
    uint8_t poop_count;     // Number of poops
    bool is_sleeping;
    bool attention_needed;  // Something needs care

    // Timing
    uint32_t last_update_ms;    // Last stat update time
    uint32_t last_fed_ms;       // Last feeding time
    uint32_t last_played_ms;    // Last play time
    uint32_t last_poop_ms;      // Last poop time
    uint32_t sleep_start_ms;    // When sleep started

    // Mini-game state
    uint16_t games_won;
    uint16_t games_played;

    // Care tracking
    uint16_t times_fed;
    uint16_t times_played;
    uint16_t times_cleaned;
    uint16_t times_medicated;
} pet_state_t;

//=============================================================================
// Initialization
//=============================================================================

/**
 * @brief Initialize pet system with a new pet
 * @return ESP_OK on success
 */
esp_err_t pet_init(void);

/**
 * @brief Create a new pet (egg)
 */
void pet_new(void);

/**
 * @brief Get pointer to current pet state
 * @return Pointer to pet state (read-only recommended)
 */
const pet_state_t *pet_get_state(void);

/**
 * @brief Get mutable pointer to pet state (for save/load)
 * @return Pointer to pet state
 */
pet_state_t *pet_get_state_mutable(void);

//=============================================================================
// Core Update
//=============================================================================

/**
 * @brief Update pet state (call every game tick)
 *
 * Handles stat decay, poop generation, sickness, and death.
 * @param delta_ms Time since last update in milliseconds
 */
void pet_update(uint32_t delta_ms);

/**
 * @brief Apply time-away decay (for when device was off)
 * @param away_minutes Minutes the device was powered off
 */
void pet_apply_time_away(uint32_t away_minutes);

//=============================================================================
// Actions (REQ-SW-003, REQ-SW-004, REQ-SW-005, REQ-SW-006, REQ-SW-007)
//=============================================================================

/**
 * @brief Feed the pet
 * @param food Type of food to give
 * @return true if feeding was successful
 */
bool pet_feed(food_type_t food);

/**
 * @brief Start playing with the pet (mini-game)
 * @return true if play is allowed (enough energy)
 */
bool pet_play_start(void);

/**
 * @brief Complete play session with result
 * @param won Whether the mini-game was won
 */
void pet_play_complete(bool won);

/**
 * @brief Put pet to sleep
 * @return true if pet went to sleep
 */
bool pet_sleep(void);

/**
 * @brief Wake pet up
 * @return true if pet woke up
 */
bool pet_wake(void);

/**
 * @brief Toggle sleep state
 */
void pet_toggle_sleep(void);

/**
 * @brief Clean up poop
 * @return true if there was poop to clean
 */
bool pet_clean(void);

/**
 * @brief Give medicine to sick pet
 * @return true if medicine was given
 */
bool pet_give_medicine(void);

//=============================================================================
// Queries
//=============================================================================

/**
 * @brief Check if pet is alive
 * @return true if pet is alive
 */
bool pet_is_alive(void);

/**
 * @brief Check if any stat is critical (< 20%)
 * @return true if attention needed
 */
bool pet_needs_attention(void);

/**
 * @brief Check if pet can play (enough energy)
 * @return true if pet can play
 */
bool pet_can_play(void);

/**
 * @brief Get age in days
 * @return Age in days (0 = first day)
 */
uint32_t pet_get_age_days(void);

/**
 * @brief Get string name for current life stage
 * @return Stage name string
 */
const char *pet_get_stage_name(void);

/**
 * @brief Get string name for current mood
 * @return Mood name string
 */
const char *pet_get_mood_name(void);

/**
 * @brief Calculate overall happiness percentage
 * @return Combined happiness score 0-100
 */
uint8_t pet_get_overall_happiness(void);

#endif // PET_H
