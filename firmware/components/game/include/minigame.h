/**
 * @file minigame.h
 * @brief "Jump the Wave" mini-game for ESP32 Tamagotchi
 *
 * REQ-SW-004: Play Mechanic
 * Simple reaction-based game where the dolphin jumps over waves.
 */

#ifndef MINIGAME_H
#define MINIGAME_H

#include <stdint.h>
#include <stdbool.h>
#include "input.h"

//=============================================================================
// Mini-game State
//=============================================================================

typedef enum {
    MINIGAME_STATE_READY,       // Waiting to start
    MINIGAME_STATE_PLAYING,     // Game in progress
    MINIGAME_STATE_SUCCESS,     // Successful jump
    MINIGAME_STATE_FAIL,        // Missed the wave
    MINIGAME_STATE_RESULTS,     // Showing results
} minigame_state_t;

typedef struct {
    minigame_state_t state;
    uint8_t round;              // Current round (1-3)
    uint8_t max_rounds;         // Total rounds
    uint8_t successes;          // Successful jumps
    uint8_t failures;           // Missed jumps

    // Wave timing
    uint32_t wave_x;            // Wave X position (moves left)
    uint32_t wave_speed;        // Pixels per update
    bool wave_active;           // Wave is on screen

    // Dolphin state
    int32_t dolphin_y;          // Dolphin Y position
    int32_t dolphin_vy;         // Dolphin vertical velocity
    bool is_jumping;            // Currently in jump

    // Timing
    uint32_t start_time_ms;     // Round start time
    uint32_t result_time_ms;    // Time showing result
} minigame_t;

//=============================================================================
// Public Functions
//=============================================================================

/**
 * @brief Initialize mini-game state
 */
void minigame_init(void);

/**
 * @brief Start a new mini-game session
 */
void minigame_start(void);

/**
 * @brief Update mini-game state
 * @param delta_ms Time since last update
 * @return true if game is still running, false if complete
 */
bool minigame_update(uint32_t delta_ms);

/**
 * @brief Handle input during mini-game
 * @param button Which button
 * @param event What event
 */
void minigame_handle_input(button_id_t button, button_event_t event);

/**
 * @brief Render mini-game to display
 */
void minigame_render(void);

/**
 * @brief Check if game is complete
 * @return true if all rounds finished
 */
bool minigame_is_complete(void);

/**
 * @brief Check if player won overall
 * @return true if more successes than failures
 */
bool minigame_is_win(void);

/**
 * @brief Get current mini-game state
 * @return Pointer to state structure
 */
const minigame_t *minigame_get_state(void);

#endif // MINIGAME_H
