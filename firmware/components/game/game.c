/**
 * @file game.c
 * @brief Main game logic implementation
 *
 * REQ-SW-010: Main Display
 * REQ-SW-011: Menu System
 * REQ-SW-038: Screen Orientation - Portrait mode (135x240)
 * REQ-SW-039: GUI Location - Bottom 20% menu bar
 * REQ-SW-043: Button Functions - Left=down/back, Right=up/confirm
 */

#include "game.h"
#include "minigame.h"
#include "display.h"
#include "pet.h"
#include "sprites.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "game";

//=============================================================================
// Colors
//=============================================================================

#define COLOR_BG            0x2B4D  // Dark ocean blue
#define COLOR_BG_LIGHT      0x5D9F  // Light ocean blue
#define COLOR_WHITE         0xFFFF
#define COLOR_BLACK         0x0000
#define COLOR_MENU_BG       0x1082  // Dark blue menu
#define COLOR_MENU_SELECT   0x07E0  // Green selection
#define COLOR_MENU_BORDER   0x4A69  // Subtle border
#define COLOR_TEXT          0xFFFF
#define COLOR_TEXT_DIM      0xBDF7
#define COLOR_CRITICAL      0xF800  // Red for critical stats
#define COLOR_GOOD          0x07E0  // Green for good stats

//=============================================================================
// Layout Constants - Portrait Mode (REQ-SW-038, REQ-SW-039)
//=============================================================================

#define SCREEN_W            135     // Portrait width
#define SCREEN_H            240     // Portrait height

// Status bar at top (icons + stats)
#define STATUS_BAR_Y        0
#define STATUS_BAR_H        24

// Main game area (pet display) - 80% of screen
#define GAME_AREA_Y         STATUS_BAR_H
#define GAME_AREA_H         (SCREEN_H - STATUS_BAR_H - MENU_BAR_H)  // 168 pixels

// Bottom menu bar - 20% of screen (REQ-SW-039)
#define MENU_BAR_Y          (SCREEN_H - MENU_BAR_H)  // 192
#define MENU_BAR_H          48                        // ~20% of 240

// Pet positioning (centered in game area)
#define PET_CENTER_X        (SCREEN_W / 2)           // 67
#define PET_CENTER_Y        (GAME_AREA_Y + GAME_AREA_H / 2)  // ~108

// Menu item layout
#define MENU_ICON_SIZE      24
#define MENU_VISIBLE_ITEMS  4       // Number of icons visible at once
#define MENU_ITEM_SPACING   4

// Stat icon layout (compact for portrait)
#define ICON_SIZE           12
#define STAT_BAR_W          16
#define STAT_BAR_H          4

//=============================================================================
// Static State
//=============================================================================

static game_state_t s_state = GAME_STATE_SPLASH;
static uint32_t s_state_time_ms = 0;
static uint8_t s_menu_selection = 0;
static uint8_t s_menu_scroll_offset = 0;  // For scrollable menu (REQ-SW-039)
static uint8_t s_food_selection = 0;
static uint32_t s_animation_frame = 0;
static uint32_t s_animation_timer = 0;
static uint32_t s_last_update_ms = 0;
static bool s_attention_flash = false;
static uint32_t s_flash_timer = 0;
static bool s_menu_active = false;  // Track if menu is activated

// Menu item labels (abbreviated for small icons)
static const char *s_menu_labels[] = {
    "FEED", "PLAY", "ZZZ", "CLEAN", "MED", "STAT", "SET"
};

static const char *s_food_labels[] = {
    "FISH", "SHRMP", "BACK"
};

//=============================================================================
// Helper Functions
//=============================================================================

static inline uint32_t get_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static void change_state(game_state_t new_state)
{
    ESP_LOGI(TAG, "State change: %d -> %d", s_state, new_state);
    s_state = new_state;
    s_state_time_ms = get_ms();

    // Reset menu state when entering menu
    if (new_state == GAME_STATE_MENU) {
        s_menu_active = true;
    } else if (new_state == GAME_STATE_MAIN) {
        s_menu_active = false;
    }
}

//=============================================================================
// Rendering Functions - Portrait Layout
//=============================================================================

/**
 * @brief Render compact status bar for portrait mode
 * Shows 4 stat icons with mini bars in a single row
 */
static void render_status_bar(void)
{
    const pet_state_t *pet = pet_get_state();

    // Background bar
    display_fill_rect(0, 0, SCREEN_W, STATUS_BAR_H, COLOR_MENU_BG);

    // Draw stat icons in a row (compact layout for 135px width)
    // Layout: [icon][bar] [icon][bar] [icon][bar] [icon][bar]
    int x_start = 4;
    int y_icon = 4;
    int y_bar = y_icon + ICON_SIZE + 2;
    int item_width = (SCREEN_W - 8) / 4;  // ~31px per stat

    // Hunger
    uint16_t hunger_color = (pet->hunger < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    const uint16_t *icon = sprites_get_stat_icon(0, pet->hunger);
    int x = x_start;
    display_draw_sprite(x + (item_width - ICON_SIZE) / 2, y_icon, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);
    // Mini bar below icon
    int bar_x = x + (item_width - STAT_BAR_W) / 2;
    display_fill_rect(bar_x, y_bar, STAT_BAR_W, STAT_BAR_H, COLOR_BLACK);
    int fill = (pet->hunger * STAT_BAR_W) / 100;
    display_fill_rect(bar_x, y_bar, fill, STAT_BAR_H, hunger_color);

    // Happiness
    x += item_width;
    uint16_t happy_color = (pet->happiness < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    icon = sprites_get_stat_icon(1, pet->happiness);
    display_draw_sprite(x + (item_width - ICON_SIZE) / 2, y_icon, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);
    bar_x = x + (item_width - STAT_BAR_W) / 2;
    display_fill_rect(bar_x, y_bar, STAT_BAR_W, STAT_BAR_H, COLOR_BLACK);
    fill = (pet->happiness * STAT_BAR_W) / 100;
    display_fill_rect(bar_x, y_bar, fill, STAT_BAR_H, happy_color);

    // Health
    x += item_width;
    uint16_t health_color = (pet->health < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    icon = sprites_get_stat_icon(2, pet->health);
    display_draw_sprite(x + (item_width - ICON_SIZE) / 2, y_icon, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);
    bar_x = x + (item_width - STAT_BAR_W) / 2;
    display_fill_rect(bar_x, y_bar, STAT_BAR_W, STAT_BAR_H, COLOR_BLACK);
    fill = (pet->health * STAT_BAR_W) / 100;
    display_fill_rect(bar_x, y_bar, fill, STAT_BAR_H, health_color);

    // Energy
    x += item_width;
    uint16_t energy_color = (pet->energy < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    icon = sprites_get_stat_icon(3, pet->energy);
    display_draw_sprite(x + (item_width - ICON_SIZE) / 2, y_icon, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);
    bar_x = x + (item_width - STAT_BAR_W) / 2;
    display_fill_rect(bar_x, y_bar, STAT_BAR_W, STAT_BAR_H, COLOR_BLACK);
    fill = (pet->energy * STAT_BAR_W) / 100;
    display_fill_rect(bar_x, y_bar, fill, STAT_BAR_H, energy_color);

    // Attention indicator (flashing) - top right corner
    if (pet->attention_needed && s_attention_flash) {
        display_draw_string(SCREEN_W - 8, 2, "!", COLOR_CRITICAL, COLOR_MENU_BG, 1);
    }
}

/**
 * @brief Render the pet sprite centered in game area
 */
static void render_pet(void)
{
    const pet_state_t *pet = pet_get_state();
    int w, h;

    const uint16_t *sprite = sprites_get_idle_frame(
        pet->stage, s_animation_frame, &w, &h);

    // Center pet in game area, accounting for 2x scale
    int scaled_w = w * 2;
    int scaled_h = h * 2;
    int x = PET_CENTER_X - scaled_w / 2;
    int y = PET_CENTER_Y - scaled_h / 2;

    // Scale up for better visibility
    display_draw_sprite_scaled(x, y, w, h, sprite, SPRITE_TRANSPARENT, 2);
}

/**
 * @brief Render poop indicator if present
 */
static void render_poop_indicator(void)
{
    const pet_state_t *pet = pet_get_state();
    if (pet->has_poop) {
        // Draw poop indicator in bottom-left of game area
        display_draw_string(4, MENU_BAR_Y - 16, "POO", COLOR_CRITICAL, COLOR_BG, 1);
    }
}

/**
 * @brief Render age display
 */
static void render_age_display(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%s %lud", pet_get_stage_name(), (unsigned long)pet_get_age_days());
    // Position in bottom-left of game area, above menu
    display_draw_string(4, MENU_BAR_Y - 10, buf, COLOR_TEXT_DIM, COLOR_BG, 1);
}

/**
 * @brief Render bottom menu bar (REQ-SW-039)
 * Scrollable horizontal menu with icons
 */
static void render_menu_bar(void)
{
    // Menu background with border
    display_fill_rect(0, MENU_BAR_Y, SCREEN_W, MENU_BAR_H, COLOR_MENU_BG);
    display_draw_hline(0, MENU_BAR_Y, SCREEN_W, COLOR_MENU_BORDER);

    // Calculate visible items
    int total_items = MENU_COUNT;
    int visible = MENU_VISIBLE_ITEMS;

    // Ensure scroll offset keeps selection visible
    if (s_menu_selection < s_menu_scroll_offset) {
        s_menu_scroll_offset = s_menu_selection;
    } else if (s_menu_selection >= s_menu_scroll_offset + visible) {
        s_menu_scroll_offset = s_menu_selection - visible + 1;
    }

    // Draw menu items
    int item_total_w = SCREEN_W / visible;  // ~33px per item
    int text_y = MENU_BAR_Y + (MENU_BAR_H - 8) / 2;  // Center text vertically

    for (int i = 0; i < visible && (i + s_menu_scroll_offset) < total_items; i++) {
        int idx = i + s_menu_scroll_offset;
        int x = i * item_total_w;
        int w = item_total_w;

        // Highlight selected item
        bool selected = (idx == s_menu_selection) && s_menu_active;
        if (selected) {
            display_fill_rect(x + 2, MENU_BAR_Y + 2, w - 4, MENU_BAR_H - 4, COLOR_MENU_SELECT);
        }

        // Draw label (centered in item area)
        uint16_t fg = selected ? COLOR_BLACK : COLOR_WHITE;
        uint16_t bg = selected ? COLOR_MENU_SELECT : COLOR_MENU_BG;

        // Calculate text centering
        int label_len = strlen(s_menu_labels[idx]);
        int text_w = label_len * 6;  // 6px per char at size 1
        int text_x = x + (w - text_w) / 2;

        display_draw_string(text_x, text_y, s_menu_labels[idx], fg, bg, 1);
    }

    // Scroll indicators
    if (s_menu_scroll_offset > 0) {
        // Left arrow indicator
        display_draw_string(2, text_y, "<", COLOR_TEXT_DIM, COLOR_MENU_BG, 1);
    }
    if (s_menu_scroll_offset + visible < total_items) {
        // Right arrow indicator
        display_draw_string(SCREEN_W - 8, text_y, ">", COLOR_TEXT_DIM, COLOR_MENU_BG, 1);
    }

    // Button hint at very bottom (if space)
    if (!s_menu_active) {
        display_draw_string(2, MENU_BAR_Y + MENU_BAR_H - 10, "R:Menu", COLOR_TEXT_DIM, COLOR_MENU_BG, 1);
    }
}

static void render_splash(void)
{
    display_fill(COLOR_BG);
    // Center text for portrait mode
    display_draw_string(10, 80, "DOLPHIN", COLOR_WHITE, COLOR_BG, 2);
    display_draw_string(30, 105, "PET", COLOR_WHITE, COLOR_BG, 2);
    display_draw_string(20, 150, "Press button", COLOR_TEXT_DIM, COLOR_BG, 1);
}

static void render_main(void)
{
    // Ocean gradient background for game area
    for (int y = STATUS_BAR_H; y < MENU_BAR_Y; y++) {
        // Gradient from light to dark blue
        uint16_t color;
        if (y < STATUS_BAR_H + GAME_AREA_H / 3) {
            color = COLOR_BG_LIGHT;
        } else if (y < STATUS_BAR_H + (GAME_AREA_H * 2) / 3) {
            color = 0x3B6D;  // Mid blue
        } else {
            color = COLOR_BG;
        }
        display_draw_hline(0, y, SCREEN_W, color);
    }

    render_status_bar();
    render_pet();
    render_poop_indicator();
    render_age_display();
    render_menu_bar();
}

static void render_menu(void)
{
    // Same as main but with menu highlighted
    render_main();
}

static void render_food_menu(void)
{
    render_main();

    // Overlay food selection menu (centered modal)
    int menu_w = 80;
    int menu_h = 60;
    int menu_x = (SCREEN_W - menu_w) / 2;
    int menu_y = (GAME_AREA_Y + GAME_AREA_H / 2) - menu_h / 2;

    display_fill_rect(menu_x, menu_y, menu_w, menu_h, COLOR_MENU_BG);
    display_draw_rect(menu_x, menu_y, menu_w, menu_h, COLOR_WHITE);

    display_draw_string(menu_x + 20, menu_y + 4, "FEED", COLOR_WHITE, COLOR_MENU_BG, 1);

    for (int i = 0; i < FOOD_MENU_COUNT; i++) {
        int y = menu_y + 16 + i * 14;
        uint16_t bg = (i == s_food_selection) ? COLOR_MENU_SELECT : COLOR_MENU_BG;
        uint16_t fg = (i == s_food_selection) ? COLOR_BLACK : COLOR_WHITE;

        display_fill_rect(menu_x + 8, y, menu_w - 16, 12, bg);
        display_draw_string(menu_x + 16, y + 2, s_food_labels[i], fg, bg, 1);
    }
}

static void render_stats(void)
{
    display_fill(COLOR_MENU_BG);

    const pet_state_t *pet = pet_get_state();
    char buf[32];

    display_draw_string(30, 5, "PET STATS", COLOR_WHITE, COLOR_MENU_BG, 1);
    display_draw_hline(10, 16, SCREEN_W - 20, COLOR_WHITE);

    int y = 22;
    int spacing = 12;

    snprintf(buf, sizeof(buf), "Stage: %s", pet_get_stage_name());
    display_draw_string(8, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Age: %lu days", (unsigned long)pet_get_age_days());
    display_draw_string(8, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Hunger: %d%%", pet->hunger);
    display_draw_string(8, y, buf, pet->hunger < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Happy: %d%%", pet->happiness);
    display_draw_string(8, y, buf, pet->happiness < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Health: %d%%", pet->health);
    display_draw_string(8, y, buf, pet->health < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Energy: %d%%", pet->energy);
    display_draw_string(8, y, buf, pet->energy < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Weight: %d", pet->weight);
    display_draw_string(8, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Games: %d/%d", pet->games_won, pet->games_played);
    display_draw_string(8, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Fed: %d times", pet->times_fed);
    display_draw_string(8, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);

    display_draw_string(20, SCREEN_H - 14, "Press to exit", COLOR_TEXT_DIM, COLOR_MENU_BG, 1);
}

static void render_death(void)
{
    display_fill(COLOR_BLACK);

    char buf[32];

    display_draw_string(15, 60, "GAME OVER", COLOR_CRITICAL, COLOR_BLACK, 2);

    snprintf(buf, sizeof(buf), "Lived %lu days", (unsigned long)pet_get_age_days());
    display_draw_string(20, 110, buf, COLOR_WHITE, COLOR_BLACK, 1);

    display_draw_string(15, 160, "Press button", COLOR_TEXT_DIM, COLOR_BLACK, 1);
    display_draw_string(20, 175, "for new pet", COLOR_TEXT_DIM, COLOR_BLACK, 1);
}

//=============================================================================
// Public Functions
//=============================================================================

esp_err_t game_init(void)
{
    ESP_LOGI(TAG, "Initializing game (portrait mode)");

    s_state = GAME_STATE_SPLASH;
    s_state_time_ms = get_ms();
    s_menu_selection = 0;
    s_menu_scroll_offset = 0;
    s_animation_frame = 0;
    s_last_update_ms = get_ms();
    s_menu_active = false;

    minigame_init();

    return ESP_OK;
}

void game_new(void)
{
    ESP_LOGI(TAG, "Starting new game");
    pet_new();
    s_menu_selection = 0;
    s_menu_scroll_offset = 0;
    s_menu_active = false;
    change_state(GAME_STATE_MAIN);
}

void game_update(uint32_t delta_ms)
{
    uint32_t now = get_ms();

    // Animation timer
    s_animation_timer += delta_ms;
    if (s_animation_timer >= 200) {  // 5 FPS animation
        s_animation_timer = 0;
        s_animation_frame = (s_animation_frame + 1) % 4;
    }

    // Attention flash timer
    s_flash_timer += delta_ms;
    if (s_flash_timer >= 500) {
        s_flash_timer = 0;
        s_attention_flash = !s_attention_flash;
    }

    // State-specific updates
    switch (s_state) {
        case GAME_STATE_MAIN:
        case GAME_STATE_MENU:
        case GAME_STATE_FEED:
        case GAME_STATE_STATS:
            pet_update(delta_ms);
            if (!pet_is_alive()) {
                change_state(GAME_STATE_DEATH);
            }
            break;

        case GAME_STATE_PLAY:
            if (!minigame_update(delta_ms)) {
                // Game complete
                bool won = minigame_is_win();
                pet_play_complete(won);
                change_state(GAME_STATE_MAIN);
            }
            break;

        case GAME_STATE_SLEEP:
            pet_update(delta_ms);
            if (!pet_get_state()->is_sleeping) {
                change_state(GAME_STATE_MAIN);
            }
            break;

        default:
            break;
    }

    s_last_update_ms = now;
}

void game_render(void)
{
    switch (s_state) {
        case GAME_STATE_SPLASH:
            render_splash();
            break;

        case GAME_STATE_MAIN:
            render_main();
            break;

        case GAME_STATE_MENU:
            render_menu();
            break;

        case GAME_STATE_FEED:
            render_food_menu();
            break;

        case GAME_STATE_PLAY:
            minigame_render();
            break;

        case GAME_STATE_STATS:
            render_stats();
            break;

        case GAME_STATE_SLEEP:
            render_main();
            // Draw sleep indicator
            display_draw_string(PET_CENTER_X - 18, PET_CENTER_Y - 40, "Zzz", COLOR_WHITE, COLOR_BG_LIGHT, 2);
            break;

        case GAME_STATE_DEATH:
            render_death();
            break;

        default:
            render_main();
            break;
    }
}

/**
 * @brief Handle button input (REQ-SW-043)
 *
 * Button mapping:
 * - Left (GPIO 0):  Short=Down/Previous, Long=Back
 * - Right (GPIO 35): Short=Up/Next, Long=Confirm/Select
 */
void game_handle_input(button_id_t button, button_event_t event)
{
    // Only process clicks and long presses
    if (event != BUTTON_EVENT_CLICK && event != BUTTON_EVENT_LONG_PRESS) {
        return;
    }

    switch (s_state) {
        case GAME_STATE_SPLASH:
            // Any button starts the game
            game_new();
            break;

        case GAME_STATE_MAIN:
            // REQ-SW-043: Right long press activates menu
            if (button == BUTTON_RIGHT && event == BUTTON_EVENT_LONG_PRESS) {
                s_menu_active = true;
                change_state(GAME_STATE_MENU);
            }
            // Short press can also activate menu for convenience
            else if (button == BUTTON_RIGHT && event == BUTTON_EVENT_CLICK) {
                s_menu_active = true;
                change_state(GAME_STATE_MENU);
            }
            break;

        case GAME_STATE_MENU:
            // REQ-SW-043: Left=scroll down (previous), Right=scroll up (next)
            if (event == BUTTON_EVENT_CLICK) {
                if (button == BUTTON_LEFT) {
                    // Scroll down (to higher index)
                    s_menu_selection = (s_menu_selection + 1) % MENU_COUNT;
                } else if (button == BUTTON_RIGHT) {
                    // Scroll up (to lower index)
                    if (s_menu_selection == 0) {
                        s_menu_selection = MENU_COUNT - 1;
                    } else {
                        s_menu_selection--;
                    }
                }
            }
            // REQ-SW-043: Right long press = confirm selection
            else if (button == BUTTON_RIGHT && event == BUTTON_EVENT_LONG_PRESS) {
                switch (s_menu_selection) {
                    case MENU_FEED:
                        change_state(GAME_STATE_FEED);
                        s_food_selection = 0;
                        break;
                    case MENU_PLAY:
                        if (pet_play_start()) {
                            minigame_start();
                            change_state(GAME_STATE_PLAY);
                        }
                        break;
                    case MENU_SLEEP:
                        pet_toggle_sleep();
                        change_state(pet_get_state()->is_sleeping ?
                                     GAME_STATE_SLEEP : GAME_STATE_MAIN);
                        break;
                    case MENU_CLEAN:
                        pet_clean();
                        change_state(GAME_STATE_MAIN);
                        break;
                    case MENU_MEDICINE:
                        pet_give_medicine();
                        change_state(GAME_STATE_MAIN);
                        break;
                    case MENU_STATS:
                        change_state(GAME_STATE_STATS);
                        break;
                    case MENU_SETTINGS:
                        change_state(GAME_STATE_MAIN);
                        break;
                }
            }
            // REQ-SW-043: Left long press = back
            else if (button == BUTTON_LEFT && event == BUTTON_EVENT_LONG_PRESS) {
                s_menu_active = false;
                change_state(GAME_STATE_MAIN);
            }
            break;

        case GAME_STATE_FEED:
            // REQ-SW-043: Left=down, Right=up for food selection
            if (event == BUTTON_EVENT_CLICK) {
                if (button == BUTTON_LEFT) {
                    s_food_selection = (s_food_selection + 1) % FOOD_MENU_COUNT;
                } else if (button == BUTTON_RIGHT) {
                    if (s_food_selection == 0) {
                        s_food_selection = FOOD_MENU_COUNT - 1;
                    } else {
                        s_food_selection--;
                    }
                }
            }
            // Right long press = confirm
            else if (button == BUTTON_RIGHT && event == BUTTON_EVENT_LONG_PRESS) {
                switch (s_food_selection) {
                    case FOOD_MENU_FISH:
                        pet_feed(FOOD_FISH);
                        change_state(GAME_STATE_MAIN);
                        break;
                    case FOOD_MENU_SHRIMP:
                        pet_feed(FOOD_SHRIMP);
                        change_state(GAME_STATE_MAIN);
                        break;
                    case FOOD_MENU_BACK:
                        change_state(GAME_STATE_MENU);
                        break;
                }
            }
            // Left long press = back
            else if (button == BUTTON_LEFT && event == BUTTON_EVENT_LONG_PRESS) {
                change_state(GAME_STATE_MENU);
            }
            break;

        case GAME_STATE_PLAY:
            minigame_handle_input(button, event);
            break;

        case GAME_STATE_STATS:
            // Any button exits stats
            change_state(GAME_STATE_MAIN);
            break;

        case GAME_STATE_SLEEP:
            // Right button wakes pet
            if (button == BUTTON_RIGHT) {
                pet_wake();
                change_state(GAME_STATE_MAIN);
            }
            break;

        case GAME_STATE_DEATH:
            // Any button starts new game
            game_new();
            break;

        default:
            change_state(GAME_STATE_MAIN);
            break;
    }
}

game_state_t game_get_state(void)
{
    return s_state;
}

bool game_is_running(void)
{
    return s_state == GAME_STATE_MAIN || s_state == GAME_STATE_SLEEP;
}
