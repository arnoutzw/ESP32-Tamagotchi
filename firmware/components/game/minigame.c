/**
 * @file minigame.c
 * @brief "Jump the Wave" mini-game implementation
 *
 * REQ-SW-004: Play Mechanic
 * A wave scrolls across the screen. Press the button at the right time
 * to make the dolphin jump over it.
 */

#include "minigame.h"
#include "display.h"
#include "sprites.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "minigame";

//=============================================================================
// Constants
//=============================================================================

#define SCREEN_W            240
#define SCREEN_H            135

#define DOLPHIN_X           60
#define DOLPHIN_GROUND_Y    90
#define DOLPHIN_W           32
#define DOLPHIN_H           24

#define WAVE_START_X        (SCREEN_W + 20)
#define WAVE_GROUND_Y       95
#define WAVE_W              32
#define WAVE_H              16

#define JUMP_VELOCITY       -8
#define GRAVITY             1
#define WAVE_SPEED_MIN      3
#define WAVE_SPEED_MAX      5

#define JUMP_ZONE_START     (DOLPHIN_X - 10)
#define JUMP_ZONE_END       (DOLPHIN_X + DOLPHIN_W + 10)

#define RESULT_DISPLAY_MS   1500
#define MAX_ROUNDS          3

// Colors
#define COLOR_BG            0x5D9F  // Light ocean
#define COLOR_BG_DARK       0x2B4D  // Dark ocean
#define COLOR_WAVE          0xFFFF  // White foam
#define COLOR_WAVE_DARK     0x07FF  // Cyan water
#define COLOR_TEXT          0xFFFF
#define COLOR_SUCCESS       0x07E0
#define COLOR_FAIL          0xF800

//=============================================================================
// Static State
//=============================================================================

static minigame_t s_game = {0};

//=============================================================================
// Helper Functions
//=============================================================================

static inline uint32_t get_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static void start_round(void)
{
    s_game.state = MINIGAME_STATE_PLAYING;
    s_game.wave_x = WAVE_START_X;
    s_game.wave_speed = WAVE_SPEED_MIN + (esp_random() % (WAVE_SPEED_MAX - WAVE_SPEED_MIN + 1));
    s_game.wave_active = true;
    s_game.dolphin_y = DOLPHIN_GROUND_Y;
    s_game.dolphin_vy = 0;
    s_game.is_jumping = false;
    s_game.start_time_ms = get_ms();

    ESP_LOGI(TAG, "Round %d started, wave speed: %lu",
             s_game.round, (unsigned long)s_game.wave_speed);
}

static bool check_collision(void)
{
    // Simple box collision between dolphin and wave
    int dolphin_left = DOLPHIN_X;
    int dolphin_right = DOLPHIN_X + DOLPHIN_W;
    int dolphin_bottom = s_game.dolphin_y + DOLPHIN_H;

    int wave_left = s_game.wave_x;
    int wave_right = s_game.wave_x + WAVE_W;
    int wave_top = WAVE_GROUND_Y - WAVE_H;

    // Check overlap
    if (dolphin_right > wave_left && dolphin_left < wave_right) {
        if (dolphin_bottom > wave_top) {
            return true;  // Collision!
        }
    }
    return false;
}

//=============================================================================
// Public Functions
//=============================================================================

void minigame_init(void)
{
    memset(&s_game, 0, sizeof(s_game));
    s_game.state = MINIGAME_STATE_READY;
    s_game.max_rounds = MAX_ROUNDS;
}

void minigame_start(void)
{
    ESP_LOGI(TAG, "Starting mini-game");
    memset(&s_game, 0, sizeof(s_game));
    s_game.state = MINIGAME_STATE_READY;
    s_game.round = 1;
    s_game.max_rounds = MAX_ROUNDS;
    start_round();
}

bool minigame_update(uint32_t delta_ms)
{
    if (s_game.state == MINIGAME_STATE_RESULTS) {
        // Check if result display time is over
        if (get_ms() - s_game.result_time_ms > RESULT_DISPLAY_MS) {
            if (s_game.round >= s_game.max_rounds) {
                // Game over
                return false;
            }
            // Start next round
            s_game.round++;
            start_round();
        }
        return true;
    }

    if (s_game.state != MINIGAME_STATE_PLAYING) {
        return true;
    }

    // Update dolphin physics
    if (s_game.is_jumping) {
        s_game.dolphin_vy += GRAVITY;
        s_game.dolphin_y += s_game.dolphin_vy;

        // Land on ground
        if (s_game.dolphin_y >= DOLPHIN_GROUND_Y) {
            s_game.dolphin_y = DOLPHIN_GROUND_Y;
            s_game.dolphin_vy = 0;
            s_game.is_jumping = false;
        }
    }

    // Update wave position
    if (s_game.wave_active) {
        s_game.wave_x -= s_game.wave_speed;

        // Check if wave hit dolphin
        if (check_collision()) {
            // Fail!
            s_game.state = MINIGAME_STATE_FAIL;
            s_game.failures++;
            s_game.result_time_ms = get_ms();
            ESP_LOGI(TAG, "Round %d: FAIL", s_game.round);
            return true;
        }

        // Check if wave passed
        if (s_game.wave_x + WAVE_W < DOLPHIN_X) {
            // Success!
            s_game.state = MINIGAME_STATE_SUCCESS;
            s_game.successes++;
            s_game.result_time_ms = get_ms();
            ESP_LOGI(TAG, "Round %d: SUCCESS", s_game.round);
            return true;
        }
    }

    return true;
}

void minigame_handle_input(button_id_t button, button_event_t event)
{
    if (event != BUTTON_EVENT_CLICK) return;

    if (s_game.state == MINIGAME_STATE_PLAYING) {
        if (!s_game.is_jumping) {
            // Jump!
            s_game.is_jumping = true;
            s_game.dolphin_vy = JUMP_VELOCITY;
            ESP_LOGD(TAG, "Jump!");
        }
    }
}

void minigame_render(void)
{
    // Ocean background gradient
    for (int y = 0; y < SCREEN_H; y++) {
        uint16_t color = (y < SCREEN_H / 2) ? COLOR_BG : COLOR_BG_DARK;
        display_draw_hline(0, y, SCREEN_W, color);
    }

    // Water line
    display_draw_hline(0, WAVE_GROUND_Y + 5, SCREEN_W, COLOR_WAVE_DARK);

    // Round indicator
    char buf[16];
    snprintf(buf, sizeof(buf), "Round %d/%d", s_game.round, s_game.max_rounds);
    display_draw_string(5, 5, buf, COLOR_TEXT, COLOR_BG, 1);

    // Score
    snprintf(buf, sizeof(buf), "Score: %d", s_game.successes);
    display_draw_string(SCREEN_W - 70, 5, buf, COLOR_TEXT, COLOR_BG, 1);

    // Draw wave
    if (s_game.wave_active && s_game.wave_x < SCREEN_W && s_game.wave_x + WAVE_W > 0) {
        // Simple wave shape
        int wx = s_game.wave_x;
        int wy = WAVE_GROUND_Y - WAVE_H;

        // Wave body
        display_fill_rect(wx, wy + 8, WAVE_W, WAVE_H - 8, COLOR_WAVE_DARK);
        // Foam crest
        display_fill_rect(wx + 4, wy, WAVE_W - 8, 10, COLOR_WAVE);
        display_fill_rect(wx + 8, wy - 4, WAVE_W - 16, 6, COLOR_WAVE);
    }

    // Draw dolphin
    int w, h;
    const uint16_t *sprite = sprites_get_idle_frame(1, 0, &w, &h);  // Baby frame
    display_draw_sprite_scaled(DOLPHIN_X, s_game.dolphin_y, w, h, sprite, SPRITE_TRANSPARENT, 2);

    // Draw result overlay
    if (s_game.state == MINIGAME_STATE_SUCCESS) {
        display_draw_string(80, 50, "NICE!", COLOR_SUCCESS, COLOR_BG, 2);
    } else if (s_game.state == MINIGAME_STATE_FAIL) {
        display_draw_string(80, 50, "OOPS!", COLOR_FAIL, COLOR_BG, 2);
    }

    // Instructions
    if (s_game.state == MINIGAME_STATE_PLAYING) {
        display_draw_string(60, SCREEN_H - 15, "Press to JUMP!", COLOR_TEXT, COLOR_BG_DARK, 1);
    }
}

bool minigame_is_complete(void)
{
    return s_game.round >= s_game.max_rounds &&
           (s_game.state == MINIGAME_STATE_SUCCESS ||
            s_game.state == MINIGAME_STATE_FAIL);
}

bool minigame_is_win(void)
{
    return s_game.successes > s_game.failures;
}

const minigame_t *minigame_get_state(void)
{
    return &s_game;
}
