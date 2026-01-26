/**
 * @file sprites.h
 * @brief Sprite data for ESP32 Tamagotchi
 *
 * REQ-SW-002: Pet Life Stages (different sprites per stage)
 * REQ-SW-013: Animations (animation frames)
 * REQ-SW-014: Status Icons
 *
 * All sprites are stored in Flash (PROGMEM equivalent) as RGB565.
 */

#ifndef SPRITES_H
#define SPRITES_H

#include <stdint.h>

//=============================================================================
// Transparency Color
//=============================================================================

#define SPRITE_TRANSPARENT  0xF81F  // Magenta as transparent

//=============================================================================
// Sprite Dimensions
//=============================================================================

// Dolphin sprites (different sizes per life stage)
#define DOLPHIN_EGG_W       24
#define DOLPHIN_EGG_H       28
#define DOLPHIN_BABY_W      32
#define DOLPHIN_BABY_H      24
#define DOLPHIN_CHILD_W     40
#define DOLPHIN_CHILD_H     32
#define DOLPHIN_TEEN_W      48
#define DOLPHIN_TEEN_H      36
#define DOLPHIN_ADULT_W     56
#define DOLPHIN_ADULT_H     40

// Icon sprites
#define ICON_SIZE           16

// Animation frames per state
#define ANIM_FRAMES_IDLE    4
#define ANIM_FRAMES_EAT     3
#define ANIM_FRAMES_PLAY    5
#define ANIM_FRAMES_SLEEP   3
#define ANIM_FRAMES_HAPPY   4
#define ANIM_FRAMES_SAD     3

// Food sprites
#define FOOD_FISH_W         16
#define FOOD_FISH_H         12
#define FOOD_SHRIMP_W       14
#define FOOD_SHRIMP_H       10

// Poop sprite
#define POOP_W              12
#define POOP_H              10

// Wave sprite (for mini-game)
#define WAVE_W              32
#define WAVE_H              16

//=============================================================================
// Sprite Data Declarations (defined in sprites.c)
//=============================================================================

// Egg animation (3 frames - wobble)
extern const uint16_t sprite_egg_1[];
extern const uint16_t sprite_egg_2[];
extern const uint16_t sprite_egg_3[];

// Baby dolphin (idle animation, 4 frames)
extern const uint16_t sprite_baby_idle_1[];
extern const uint16_t sprite_baby_idle_2[];
extern const uint16_t sprite_baby_idle_3[];
extern const uint16_t sprite_baby_idle_4[];

// Adult dolphin (idle animation, 4 frames) - pointers to baby sprites
extern const uint16_t *sprite_dolphin_idle_1;
extern const uint16_t *sprite_dolphin_idle_2;
extern const uint16_t *sprite_dolphin_idle_3;
extern const uint16_t *sprite_dolphin_idle_4;

// Eating animation - pointers to baby sprites
extern const uint16_t *sprite_dolphin_eat_1;
extern const uint16_t *sprite_dolphin_eat_2;
extern const uint16_t *sprite_dolphin_eat_3;

// Sleeping animation - pointers to baby sprites
extern const uint16_t *sprite_dolphin_sleep_1;
extern const uint16_t *sprite_dolphin_sleep_2;
extern const uint16_t *sprite_dolphin_sleep_3;

// Happy animation (jump/flip) - pointers to baby sprites
extern const uint16_t *sprite_dolphin_happy_1;
extern const uint16_t *sprite_dolphin_happy_2;
extern const uint16_t *sprite_dolphin_happy_3;
extern const uint16_t *sprite_dolphin_happy_4;

// Sad sprite - pointer to baby sprite
extern const uint16_t *sprite_dolphin_sad;

// Sick sprite - pointer to baby sprite
extern const uint16_t *sprite_dolphin_sick;

// Dead/angel sprite - pointer to baby sprite
extern const uint16_t *sprite_dolphin_dead;

// Status icons (16x16) - actual arrays
extern const uint16_t icon_hunger_full[];
extern const uint16_t icon_happy_full[];
extern const uint16_t icon_health_full[];
extern const uint16_t icon_energy_full[];
extern const uint16_t icon_attention[];

// Status icons - pointers (aliases)
extern const uint16_t *icon_hunger_mid;
extern const uint16_t *icon_hunger_low;
extern const uint16_t *icon_hunger_empty;
extern const uint16_t *icon_happy_mid;
extern const uint16_t *icon_happy_low;
extern const uint16_t *icon_happy_empty;
extern const uint16_t *icon_health_mid;
extern const uint16_t *icon_health_low;
extern const uint16_t *icon_health_empty;
extern const uint16_t *icon_energy_mid;
extern const uint16_t *icon_energy_low;
extern const uint16_t *icon_energy_empty;
extern const uint16_t *icon_sleep;
extern const uint16_t *icon_poop;

// Menu icons (16x16) - pointers (aliases)
extern const uint16_t *icon_menu_feed;
extern const uint16_t *icon_menu_play;
extern const uint16_t *icon_menu_sleep;
extern const uint16_t *icon_menu_clean;
extern const uint16_t *icon_menu_medicine;
extern const uint16_t *icon_menu_stats;
extern const uint16_t *icon_menu_settings;

// Food sprites - pointers (aliases)
extern const uint16_t *sprite_food_fish;
extern const uint16_t *sprite_food_shrimp;

// Poop sprite - pointer (alias)
extern const uint16_t *sprite_poop;

// Wave for mini-game - pointer (alias)
extern const uint16_t *sprite_wave;

// Z's for sleep animation - pointer (alias)
extern const uint16_t *sprite_zzz;

//=============================================================================
// Helper Functions
//=============================================================================

/**
 * @brief Get idle animation frame for current pet stage
 * @param stage Pet life stage
 * @param frame Frame index (0-3)
 * @param width Output: sprite width
 * @param height Output: sprite height
 * @return Pointer to sprite data
 */
const uint16_t *sprites_get_idle_frame(int stage, int frame, int *width, int *height);

/**
 * @brief Get status icon for stat level
 * @param stat_type 0=hunger, 1=happy, 2=health, 3=energy
 * @param level Stat value 0-100
 * @return Pointer to icon sprite
 */
const uint16_t *sprites_get_stat_icon(int stat_type, int level);

/**
 * @brief Get menu icon
 * @param menu_item Menu item index
 * @return Pointer to icon sprite
 */
const uint16_t *sprites_get_menu_icon(int menu_item);

#endif // SPRITES_H
