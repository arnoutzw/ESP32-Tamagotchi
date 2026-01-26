/**
 * @file game.c
 * @brief Main game logic implementation
 *
 * REQ-SW-010: Main Display
 * REQ-SW-011: Menu System
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
#define COLOR_TEXT          0xFFFF
#define COLOR_TEXT_DIM      0xBDF7
#define COLOR_CRITICAL      0xF800  // Red for critical stats
#define COLOR_GOOD          0x07E0  // Green for good stats

//=============================================================================
// Layout Constants
//=============================================================================

#define SCREEN_W            240
#define SCREEN_H            135

#define ICON_SIZE           16
#define ICON_SPACING        4
#define STATUS_BAR_Y        2
#define STATUS_BAR_H        20

#define PET_CENTER_X        (SCREEN_W / 2)
#define PET_CENTER_Y        (SCREEN_H / 2 + 10)

#define MENU_X              10
#define MENU_Y              25
#define MENU_ITEM_H         18
#define MENU_COLS           4
#define MENU_ROWS           2

//=============================================================================
// Static State
//=============================================================================

static game_state_t s_state = GAME_STATE_SPLASH;
static uint32_t s_state_time_ms = 0;
static uint8_t s_menu_selection = 0;
static uint8_t s_food_selection = 0;
static uint32_t s_animation_frame = 0;
static uint32_t s_animation_timer = 0;
static uint32_t s_last_update_ms = 0;
static bool s_attention_flash = false;
static uint32_t s_flash_timer = 0;

// Menu item labels
static const char *s_menu_labels[] = {
    "FEED", "PLAY", "SLEEP", "CLEAN", "MED", "STATS", "SET"
};

static const char *s_food_labels[] = {
    "FISH", "SHRIMP", "BACK"
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
}

//=============================================================================
// Rendering Functions
//=============================================================================

static void render_status_bar(void)
{
    const pet_state_t *pet = pet_get_state();

    // Background bar
    display_fill_rect(0, 0, SCREEN_W, STATUS_BAR_H, COLOR_MENU_BG);

    // Draw stat icons
    int x = 8;
    int y = 2;

    // Hunger icon (with color based on level)
    uint16_t hunger_color = (pet->hunger < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    const uint16_t *icon = sprites_get_stat_icon(0, pet->hunger);
    display_draw_sprite(x, y, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);

    // Small stat bar next to icon
    int bar_w = 20;
    int bar_h = 4;
    int bar_x = x + ICON_SIZE + 2;
    int bar_y = y + 6;
    display_fill_rect(bar_x, bar_y, bar_w, bar_h, COLOR_BLACK);
    int fill = (pet->hunger * bar_w) / 100;
    display_fill_rect(bar_x, bar_y, fill, bar_h, hunger_color);

    // Happiness
    x += ICON_SIZE + bar_w + 10;
    uint16_t happy_color = (pet->happiness < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    icon = sprites_get_stat_icon(1, pet->happiness);
    display_draw_sprite(x, y, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);
    bar_x = x + ICON_SIZE + 2;
    display_fill_rect(bar_x, bar_y, bar_w, bar_h, COLOR_BLACK);
    fill = (pet->happiness * bar_w) / 100;
    display_fill_rect(bar_x, bar_y, fill, bar_h, happy_color);

    // Health
    x += ICON_SIZE + bar_w + 10;
    uint16_t health_color = (pet->health < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    icon = sprites_get_stat_icon(2, pet->health);
    display_draw_sprite(x, y, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);
    bar_x = x + ICON_SIZE + 2;
    display_fill_rect(bar_x, bar_y, bar_w, bar_h, COLOR_BLACK);
    fill = (pet->health * bar_w) / 100;
    display_fill_rect(bar_x, bar_y, fill, bar_h, health_color);

    // Energy
    x += ICON_SIZE + bar_w + 10;
    uint16_t energy_color = (pet->energy < 20) ? COLOR_CRITICAL : COLOR_GOOD;
    icon = sprites_get_stat_icon(3, pet->energy);
    display_draw_sprite(x, y, ICON_SIZE, ICON_SIZE, icon, SPRITE_TRANSPARENT);
    bar_x = x + ICON_SIZE + 2;
    display_fill_rect(bar_x, bar_y, bar_w, bar_h, COLOR_BLACK);
    fill = (pet->energy * bar_w) / 100;
    display_fill_rect(bar_x, bar_y, fill, bar_h, energy_color);

    // Attention indicator (flashing exclamation)
    if (pet->attention_needed && s_attention_flash) {
        display_draw_sprite(SCREEN_W - 20, y, ICON_SIZE, ICON_SIZE,
                           icon_attention, SPRITE_TRANSPARENT);
    }
}

static void render_pet(void)
{
    const pet_state_t *pet = pet_get_state();
    int w, h;

    const uint16_t *sprite = sprites_get_idle_frame(
        pet->stage, s_animation_frame, &w, &h);

    int x = PET_CENTER_X - w / 2;
    int y = PET_CENTER_Y - h / 2;

    // Scale up for better visibility
    display_draw_sprite_scaled(x, y, w, h, sprite, SPRITE_TRANSPARENT, 2);
}

static void render_poop_indicator(void)
{
    const pet_state_t *pet = pet_get_state();
    if (pet->has_poop) {
        // Draw poop icon in corner
        display_draw_string(SCREEN_W - 30, SCREEN_H - 20, "POO", COLOR_CRITICAL, COLOR_BG, 1);
    }
}

static void render_age_display(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%s %lud", pet_get_stage_name(), (unsigned long)pet_get_age_days());
    display_draw_string(4, SCREEN_H - 12, buf, COLOR_TEXT_DIM, COLOR_BG, 1);
}

static void render_splash(void)
{
    display_fill(COLOR_BG);
    display_draw_string(50, 40, "DOLPHIN PET", COLOR_WHITE, COLOR_BG, 2);
    display_draw_string(60, 80, "Press any button", COLOR_TEXT_DIM, COLOR_BG, 1);
}

static void render_main(void)
{
    // Ocean gradient background
    for (int y = STATUS_BAR_H; y < SCREEN_H; y++) {
        uint16_t color = (y < SCREEN_H / 2) ? COLOR_BG_LIGHT : COLOR_BG;
        display_draw_hline(0, y, SCREEN_W, color);
    }

    render_status_bar();
    render_pet();
    render_poop_indicator();
    render_age_display();
}

static void render_menu(void)
{
    // Darken background
    render_main();

    // Menu panel
    int menu_w = SCREEN_W - 20;
    int menu_h = 60;
    int menu_x = 10;
    int menu_y = 35;

    display_fill_rect(menu_x, menu_y, menu_w, menu_h, COLOR_MENU_BG);
    display_draw_rect(menu_x, menu_y, menu_w, menu_h, COLOR_WHITE);

    // Draw menu items in grid
    int item_w = (menu_w - 20) / MENU_COLS;
    int item_h = 24;

    for (int i = 0; i < MENU_COUNT; i++) {
        int row = i / MENU_COLS;
        int col = i % MENU_COLS;
        int x = menu_x + 10 + col * item_w;
        int y = menu_y + 8 + row * item_h;

        uint16_t bg = (i == s_menu_selection) ? COLOR_MENU_SELECT : COLOR_MENU_BG;
        uint16_t fg = (i == s_menu_selection) ? COLOR_BLACK : COLOR_WHITE;

        display_fill_rect(x, y, item_w - 4, item_h - 4, bg);
        display_draw_string(x + 4, y + 6, s_menu_labels[i], fg, bg, 1);
    }

    // Instructions
    display_draw_string(menu_x + 5, menu_y + menu_h + 5,
                       "L:Select  R:Confirm", COLOR_TEXT_DIM, COLOR_BG, 1);
}

static void render_food_menu(void)
{
    render_main();

    int menu_w = 100;
    int menu_h = 70;
    int menu_x = (SCREEN_W - menu_w) / 2;
    int menu_y = (SCREEN_H - menu_h) / 2;

    display_fill_rect(menu_x, menu_y, menu_w, menu_h, COLOR_MENU_BG);
    display_draw_rect(menu_x, menu_y, menu_w, menu_h, COLOR_WHITE);

    display_draw_string(menu_x + 20, menu_y + 5, "FEED", COLOR_WHITE, COLOR_MENU_BG, 1);

    for (int i = 0; i < FOOD_MENU_COUNT; i++) {
        int y = menu_y + 20 + i * 16;
        uint16_t bg = (i == s_food_selection) ? COLOR_MENU_SELECT : COLOR_MENU_BG;
        uint16_t fg = (i == s_food_selection) ? COLOR_BLACK : COLOR_WHITE;

        display_fill_rect(menu_x + 10, y, menu_w - 20, 14, bg);
        display_draw_string(menu_x + 20, y + 3, s_food_labels[i], fg, bg, 1);
    }
}

static void render_stats(void)
{
    display_fill(COLOR_MENU_BG);

    const pet_state_t *pet = pet_get_state();
    char buf[32];

    display_draw_string(80, 5, "PET STATS", COLOR_WHITE, COLOR_MENU_BG, 1);
    display_draw_hline(10, 18, SCREEN_W - 20, COLOR_WHITE);

    int y = 25;
    int spacing = 14;

    snprintf(buf, sizeof(buf), "Stage:  %s", pet_get_stage_name());
    display_draw_string(10, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Age:    %lu days", (unsigned long)pet_get_age_days());
    display_draw_string(10, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Hunger: %d%%", pet->hunger);
    display_draw_string(10, y, buf, pet->hunger < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Happy:  %d%%", pet->happiness);
    display_draw_string(10, y, buf, pet->happiness < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Health: %d%%", pet->health);
    display_draw_string(10, y, buf, pet->health < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Energy: %d%%", pet->energy);
    display_draw_string(10, y, buf, pet->energy < 20 ? COLOR_CRITICAL : COLOR_WHITE, COLOR_MENU_BG, 1);

    // Right column
    y = 25;
    snprintf(buf, sizeof(buf), "Weight: %d", pet->weight);
    display_draw_string(130, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Games:  %d/%d", pet->games_won, pet->games_played);
    display_draw_string(130, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);
    y += spacing;

    snprintf(buf, sizeof(buf), "Fed:    %d", pet->times_fed);
    display_draw_string(130, y, buf, COLOR_WHITE, COLOR_MENU_BG, 1);

    display_draw_string(80, SCREEN_H - 12, "Press any button", COLOR_TEXT_DIM, COLOR_MENU_BG, 1);
}

static void render_death(void)
{
    display_fill(COLOR_BLACK);

    char buf[32];

    display_draw_string(60, 30, "GAME OVER", COLOR_CRITICAL, COLOR_BLACK, 2);

    snprintf(buf, sizeof(buf), "Your dolphin lived %lu days", (unsigned long)pet_get_age_days());
    display_draw_string(30, 70, buf, COLOR_WHITE, COLOR_BLACK, 1);

    display_draw_string(50, 100, "Press any button", COLOR_TEXT_DIM, COLOR_BLACK, 1);
    display_draw_string(60, 115, "for new pet", COLOR_TEXT_DIM, COLOR_BLACK, 1);
}

//=============================================================================
// Public Functions
//=============================================================================

esp_err_t game_init(void)
{
    ESP_LOGI(TAG, "Initializing game");

    s_state = GAME_STATE_SPLASH;
    s_state_time_ms = get_ms();
    s_menu_selection = 0;
    s_animation_frame = 0;
    s_last_update_ms = get_ms();

    minigame_init();

    return ESP_OK;
}

void game_new(void)
{
    ESP_LOGI(TAG, "Starting new game");
    pet_new();
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
            display_draw_string(100, 60, "Zzz...", COLOR_WHITE, COLOR_BG, 2);
            break;

        case GAME_STATE_DEATH:
            render_death();
            break;

        default:
            render_main();
            break;
    }
}

void game_handle_input(button_id_t button, button_event_t event)
{
    if (event != BUTTON_EVENT_CLICK && event != BUTTON_EVENT_LONG_PRESS) {
        return;
    }

    switch (s_state) {
        case GAME_STATE_SPLASH:
            game_new();
            break;

        case GAME_STATE_MAIN:
            if (button == BUTTON_LEFT || button == BUTTON_RIGHT) {
                change_state(GAME_STATE_MENU);
                s_menu_selection = 0;
            }
            break;

        case GAME_STATE_MENU:
            if (button == BUTTON_LEFT) {
                s_menu_selection = (s_menu_selection + 1) % MENU_COUNT;
            } else if (button == BUTTON_RIGHT) {
                // Execute menu action
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
            if (event == BUTTON_EVENT_LONG_PRESS) {
                change_state(GAME_STATE_MAIN);
            }
            break;

        case GAME_STATE_FEED:
            if (button == BUTTON_LEFT) {
                s_food_selection = (s_food_selection + 1) % FOOD_MENU_COUNT;
            } else if (button == BUTTON_RIGHT) {
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
            break;

        case GAME_STATE_PLAY:
            minigame_handle_input(button, event);
            break;

        case GAME_STATE_STATS:
            change_state(GAME_STATE_MAIN);
            break;

        case GAME_STATE_SLEEP:
            if (button == BUTTON_RIGHT) {
                pet_wake();
                change_state(GAME_STATE_MAIN);
            }
            break;

        case GAME_STATE_DEATH:
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
