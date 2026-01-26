/**
 * @file sprites.c
 * @brief Sprite data for ESP32 Tamagotchi
 *
 * Hand-crafted pixel art sprites for the baby dolphin virtual pet.
 * All sprites are stored in Flash to save RAM.
 *
 * Color palette (RGB565):
 * - Transparent: 0xF81F (magenta)
 * - Ocean Blue:  0x5D9F (light), 0x2B4D (dark)
 * - Dolphin:     0x7BEF (body), 0xC618 (belly), 0x31A6 (shadow)
 * - White:       0xFFFF
 * - Black:       0x0000
 * - Pink:        0xFE19 (blush)
 * - Yellow:      0xFFE0 (star)
 */

#include "sprites.h"

// Transparency shorthand
#define T   0xF81F  // Transparent (magenta)
#define W   0xFFFF  // White
#define B   0x0000  // Black
#define DB  0x7BEF  // Dolphin body (gray-blue)
#define DL  0xC618  // Dolphin light (belly)
#define DS  0x31A6  // Dolphin shadow
#define PK  0xFE19  // Pink
#define YL  0xFFE0  // Yellow
#define RD  0xF800  // Red
#define GN  0x07E0  // Green
#define OR  0xFD20  // Orange
#define CY  0x07FF  // Cyan
#define BL  0x001F  // Blue

//=============================================================================
// Egg Sprites (24x28)
//=============================================================================

const uint16_t sprite_egg_1[] = {
    // Row 0-7: Top of egg
    T,T,T,T,T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,CY,CY,W,W,W,W,CY,CY,W,W,T,T,T,T,T,T,
    T,T,T,T,T,W,W,CY,CY,CY,W,W,W,W,CY,CY,CY,W,W,T,T,T,T,T,
    T,T,T,T,W,W,CY,CY,W,W,W,W,W,W,W,W,CY,CY,W,W,T,T,T,T,
    T,T,T,T,W,W,CY,W,W,W,W,W,W,W,W,W,W,CY,W,W,T,T,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    // Row 8-15: Middle of egg
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    // Row 16-23: Bottom of egg
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,
    T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,
    T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,T,T,T,T,
    // Row 24-27: Base
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

// Egg wobble frames 2 and 3 (same for now, can customize)
const uint16_t sprite_egg_2[] = {
    // Same as egg_1 but shifted slightly right (simulated wobble)
    T,T,T,T,T,T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,T,T,W,W,CY,CY,W,W,W,W,CY,CY,W,W,T,T,T,T,T,
    T,T,T,T,T,T,W,W,CY,CY,CY,W,W,W,W,CY,CY,CY,W,W,T,T,T,T,
    T,T,T,T,T,W,W,CY,CY,W,W,W,W,W,W,W,W,CY,CY,W,W,T,T,T,
    T,T,T,T,T,W,W,CY,W,W,W,W,W,W,W,W,W,W,CY,W,W,T,T,T,
    T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,
    T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,
    T,T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,
    T,T,T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

const uint16_t sprite_egg_3[] = {
    // Same as egg_1 but shifted slightly left
    T,T,T,T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,T,T,
    T,T,T,T,T,W,W,CY,CY,W,W,W,W,CY,CY,W,W,T,T,T,T,T,T,T,
    T,T,T,T,W,W,CY,CY,CY,W,W,W,W,CY,CY,CY,W,W,T,T,T,T,T,T,
    T,T,T,W,W,CY,CY,W,W,W,W,W,W,W,W,CY,CY,W,W,T,T,T,T,T,
    T,T,T,W,W,CY,W,W,W,W,W,W,W,W,W,W,CY,W,W,T,T,T,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,
    T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

//=============================================================================
// Baby Dolphin Sprites (32x24) - Cute small dolphin
//=============================================================================

const uint16_t sprite_baby_idle_1[] = {
    // Simple cute baby dolphin facing right
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,W,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,B,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,DB,DB,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,DB,DB,DB,DB,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,T,T,T,T,T,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

// Baby idle frame 2 - tail up
const uint16_t sprite_baby_idle_2[] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,W,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,B,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

// Baby idle frames 3 and 4 (variations)
const uint16_t sprite_baby_idle_3[] = {
    // Same as frame 1 for now - can be customized
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,W,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,B,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,DB,DB,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,DB,DB,DB,DB,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,T,T,T,T,T,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

const uint16_t sprite_baby_idle_4[] = {
    // Same as frame 2 for now
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,W,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,B,B,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,
    T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,DB,DB,DB,DB,DL,DL,DL,DL,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,DB,DB,DB,DB,DB,DL,DL,DL,DL,DL,DB,DB,DB,DB,DB,T,T,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,DB,DB,DB,DB,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,DB,DB,DB,DB,DB,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,DB,DB,DB,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

// Use baby sprites as placeholders for adult sprites
const uint16_t *sprite_dolphin_idle_1 = sprite_baby_idle_1;
const uint16_t *sprite_dolphin_idle_2 = sprite_baby_idle_2;
const uint16_t *sprite_dolphin_idle_3 = sprite_baby_idle_3;
const uint16_t *sprite_dolphin_idle_4 = sprite_baby_idle_4;

// Placeholder for other animation states
const uint16_t *sprite_dolphin_eat_1 = sprite_baby_idle_1;
const uint16_t *sprite_dolphin_eat_2 = sprite_baby_idle_2;
const uint16_t *sprite_dolphin_eat_3 = sprite_baby_idle_3;

const uint16_t *sprite_dolphin_sleep_1 = sprite_baby_idle_1;
const uint16_t *sprite_dolphin_sleep_2 = sprite_baby_idle_1;
const uint16_t *sprite_dolphin_sleep_3 = sprite_baby_idle_1;

const uint16_t *sprite_dolphin_happy_1 = sprite_baby_idle_1;
const uint16_t *sprite_dolphin_happy_2 = sprite_baby_idle_2;
const uint16_t *sprite_dolphin_happy_3 = sprite_baby_idle_3;
const uint16_t *sprite_dolphin_happy_4 = sprite_baby_idle_4;

const uint16_t *sprite_dolphin_sad = sprite_baby_idle_1;
const uint16_t *sprite_dolphin_sick = sprite_baby_idle_1;
const uint16_t *sprite_dolphin_dead = sprite_baby_idle_1;

//=============================================================================
// Status Icons (16x16)
//=============================================================================

// Hunger icon (fish bone)
const uint16_t icon_hunger_full[] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,OR,OR,T,T,T,OR,OR,T,T,T,T,
    T,T,T,T,OR,OR,OR,OR,OR,OR,OR,OR,T,T,T,T,
    T,T,T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,T,T,
    T,T,OR,OR,OR,B,B,OR,OR,B,B,OR,OR,OR,T,T,
    T,T,OR,OR,OR,B,B,OR,OR,B,B,OR,OR,OR,T,T,
    T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,
    T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,
    T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,
    T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,
    T,T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,T,
    T,T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,T,
    T,T,T,OR,OR,OR,OR,OR,OR,OR,OR,OR,OR,T,T,T,
    T,T,T,T,OR,OR,OR,OR,OR,OR,OR,OR,T,T,T,T,
    T,T,T,T,T,OR,OR,T,T,OR,OR,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

const uint16_t *icon_hunger_mid = icon_hunger_full;
const uint16_t *icon_hunger_low = icon_hunger_full;
const uint16_t *icon_hunger_empty = icon_hunger_full;

// Happiness icon (heart)
const uint16_t icon_happy_full[] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,RD,RD,T,T,T,T,T,RD,RD,T,T,T,T,
    T,T,RD,RD,RD,RD,T,T,T,RD,RD,RD,RD,T,T,T,
    T,RD,RD,RD,RD,RD,RD,T,RD,RD,RD,RD,RD,RD,T,T,
    T,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,T,T,
    T,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,T,T,
    T,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,T,T,
    T,T,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,T,T,T,
    T,T,T,RD,RD,RD,RD,RD,RD,RD,RD,RD,T,T,T,T,
    T,T,T,T,RD,RD,RD,RD,RD,RD,RD,T,T,T,T,T,
    T,T,T,T,T,RD,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,RD,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

const uint16_t *icon_happy_mid = icon_happy_full;
const uint16_t *icon_happy_low = icon_happy_full;
const uint16_t *icon_happy_empty = icon_happy_full;

// Health icon (cross)
const uint16_t icon_health_full[] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,T,T,
    T,T,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,T,T,
    T,T,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,T,T,
    T,T,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,GN,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,T,T,T,T,GN,GN,GN,GN,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

const uint16_t *icon_health_mid = icon_health_full;
const uint16_t *icon_health_low = icon_health_full;
const uint16_t *icon_health_empty = icon_health_full;

// Energy icon (lightning bolt)
const uint16_t icon_energy_full[] = {
    T,T,T,T,T,T,T,T,T,T,YL,YL,T,T,T,T,
    T,T,T,T,T,T,T,T,T,YL,YL,T,T,T,T,T,
    T,T,T,T,T,T,T,T,YL,YL,T,T,T,T,T,T,
    T,T,T,T,T,T,T,YL,YL,T,T,T,T,T,T,T,
    T,T,T,T,T,T,YL,YL,T,T,T,T,T,T,T,T,
    T,T,T,T,T,YL,YL,T,T,T,T,T,T,T,T,T,
    T,T,T,T,YL,YL,YL,YL,YL,YL,YL,T,T,T,T,T,
    T,T,T,T,YL,YL,YL,YL,YL,YL,YL,T,T,T,T,T,
    T,T,T,T,T,T,T,YL,YL,T,T,T,T,T,T,T,
    T,T,T,T,T,T,YL,YL,T,T,T,T,T,T,T,T,
    T,T,T,T,T,YL,YL,T,T,T,T,T,T,T,T,T,
    T,T,T,T,YL,YL,T,T,T,T,T,T,T,T,T,T,
    T,T,T,YL,YL,T,T,T,T,T,T,T,T,T,T,T,
    T,T,YL,YL,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

const uint16_t *icon_energy_mid = icon_energy_full;
const uint16_t *icon_energy_low = icon_energy_full;
const uint16_t *icon_energy_empty = icon_energy_full;

// Attention icon (exclamation)
const uint16_t icon_attention[] = {
    T,T,T,T,T,T,T,RD,RD,T,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,RD,RD,RD,RD,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
};

const uint16_t *icon_sleep = icon_energy_full;
const uint16_t *icon_poop = icon_attention;

// Menu icons (placeholders - all same for now)
const uint16_t *icon_menu_feed = icon_hunger_full;
const uint16_t *icon_menu_play = icon_happy_full;
const uint16_t *icon_menu_sleep = icon_energy_full;
const uint16_t *icon_menu_clean = icon_health_full;
const uint16_t *icon_menu_medicine = icon_health_full;
const uint16_t *icon_menu_stats = icon_attention;
const uint16_t *icon_menu_settings = icon_attention;

// Food sprites (placeholders)
const uint16_t *sprite_food_fish = icon_hunger_full;
const uint16_t *sprite_food_shrimp = icon_hunger_full;
const uint16_t *sprite_poop = icon_attention;
const uint16_t *sprite_wave = icon_energy_full;
const uint16_t *sprite_zzz = icon_energy_full;

//=============================================================================
// Helper Functions
//=============================================================================

const uint16_t *sprites_get_idle_frame(int stage, int frame, int *width, int *height)
{
    static const uint16_t *baby_frames[] = {
        sprite_baby_idle_1, sprite_baby_idle_2,
        sprite_baby_idle_3, sprite_baby_idle_4
    };

    // For now, use baby sprites for all stages
    // In production, you'd have separate arrays per stage
    *width = DOLPHIN_BABY_W;
    *height = DOLPHIN_BABY_H;

    frame = frame % 4;
    return baby_frames[frame];
}

const uint16_t *sprites_get_stat_icon(int stat_type, int level)
{
    // Return appropriate icon based on stat type and level
    // 0 = hunger, 1 = happy, 2 = health, 3 = energy
    switch (stat_type) {
        case 0: return icon_hunger_full;
        case 1: return icon_happy_full;
        case 2: return icon_health_full;
        case 3: return icon_energy_full;
        default: return icon_attention;
    }
}

const uint16_t *sprites_get_menu_icon(int menu_item)
{
    switch (menu_item) {
        case 0: return icon_menu_feed;
        case 1: return icon_menu_play;
        case 2: return icon_menu_sleep;
        case 3: return icon_menu_clean;
        case 4: return icon_menu_medicine;
        case 5: return icon_menu_stats;
        case 6: return icon_menu_settings;
        default: return icon_attention;
    }
}
