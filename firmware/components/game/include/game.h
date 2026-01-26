/**
 * @file game.h
 * @brief Main game logic and state machine for ESP32 Tamagotchi
 *
 * REQ-SW-010: Main Display
 * REQ-SW-011: Menu System
 * Manages game screens, menu navigation, and rendering.
 */

#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "input.h"

//=============================================================================
// Game States
//=============================================================================

typedef enum {
    GAME_STATE_SPLASH,      // Startup splash screen
    GAME_STATE_MAIN,        // Main pet view
    GAME_STATE_MENU,        // Menu overlay
    GAME_STATE_FEED,        // Food selection submenu
    GAME_STATE_PLAY,        // Mini-game
    GAME_STATE_STATS,       // Status details screen
    GAME_STATE_SETTINGS,    // Settings menu
    GAME_STATE_SLEEP,       // Sleep animation
    GAME_STATE_DEATH,       // Game over screen
    GAME_STATE_NEW_GAME,    // New game confirmation
} game_state_t;

//=============================================================================
// Menu Items
//=============================================================================

typedef enum {
    MENU_FEED = 0,
    MENU_PLAY,
    MENU_SLEEP,
    MENU_CLEAN,
    MENU_MEDICINE,
    MENU_STATS,
    MENU_SETTINGS,
    MENU_COUNT
} menu_item_t;

typedef enum {
    FOOD_MENU_FISH = 0,
    FOOD_MENU_SHRIMP,
    FOOD_MENU_BACK,
    FOOD_MENU_COUNT
} food_menu_item_t;

//=============================================================================
// Public Functions
//=============================================================================

/**
 * @brief Initialize game system
 * @return ESP_OK on success
 */
esp_err_t game_init(void);

/**
 * @brief Start a new game with a new pet
 */
void game_new(void);

/**
 * @brief Main game update (call every frame)
 * @param delta_ms Time since last update in milliseconds
 */
void game_update(uint32_t delta_ms);

/**
 * @brief Render current game state to display
 */
void game_render(void);

/**
 * @brief Handle button input
 * @param button Which button
 * @param event What event occurred
 */
void game_handle_input(button_id_t button, button_event_t event);

/**
 * @brief Get current game state
 * @return Current game state
 */
game_state_t game_get_state(void);

/**
 * @brief Check if game is running (not paused/menu)
 * @return true if main game is active
 */
bool game_is_running(void);

#endif // GAME_H
