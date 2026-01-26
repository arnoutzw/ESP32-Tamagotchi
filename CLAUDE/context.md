# ESP32 Tamagotchi - Project Context

This document provides comprehensive context for AI assistants working on this project.

## Project Overview

A virtual pet game inspired by the classic Tamagotchi, running on TTGO T-Display ESP32. Features a colorful baby dolphin that users care for through feeding, playing, and other activities.

## Hardware

**TTGO T-Display ESP32**
- ESP32 dual-core 240MHz
- 520KB SRAM, 4MB Flash
- 1.14" ST7789 TFT IPS (240x135, 16-bit color)
- 2 push buttons (GPIO 0 = Left, GPIO 35 = Right)
- USB-C power, optional Li-Po battery

### Pin Map

| GPIO | Function |
|------|----------|
| 0 | Left button (boot) |
| 35 | Right button |
| 4 | LCD backlight |
| 5 | LCD CS |
| 16 | LCD DC |
| 18 | LCD SCLK |
| 19 | LCD MOSI |
| 23 | LCD RST |

## Architecture

### Components

| Component | Purpose |
|-----------|---------|
| `display` | ST7789 SPI driver, drawing primitives |
| `input` | Button debouncing, event callbacks |
| `pet` | Pet state machine, stats, life stages |
| `game` | Screen states, menu, rendering |
| `sprites` | Pixel art data in Flash |
| `save_manager` | NVS persistence |

### Game States

```
SPLASH → MAIN ↔ MENU → (FEED/PLAY/SLEEP/STATS/SETTINGS)
                          ↓
                       DEATH → NEW_GAME
```

### Task Structure

- Single game task at 30 FPS
- Input polling at 20ms intervals
- Auto-save every 5 minutes

## Key Data Structures

### Pet State

```c
typedef struct {
    uint8_t hunger;         // 0-100
    uint8_t happiness;      // 0-100
    uint8_t health;         // 0-100, calculated
    uint8_t energy;         // 0-100
    uint8_t weight;         // 1-99
    pet_stage_t stage;      // EGG/BABY/CHILD/TEEN/ADULT/DEAD
    uint32_t age_minutes;   // Total age
    bool is_sick;
    bool has_poop;
    bool is_sleeping;
} pet_state_t;
```

### Colors (RGB565)

| Name | Value | Use |
|------|-------|-----|
| `COLOR_BG` | 0x2B4D | Dark ocean |
| `COLOR_BG_LIGHT` | 0x5D9F | Light ocean |
| `SPRITE_TRANSPARENT` | 0xF81F | Magenta transparency |
| Dolphin body | 0x7BEF | Gray-blue |
| Dolphin belly | 0xC618 | Light gray |

## Common Commands

```bash
# Build
cd firmware && idf.py build

# Flash
idf.py -p /dev/cu.usbserial-XXXX flash monitor

# Clean build
idf.py fullclean && idf.py build
```

## Requirements Reference

Key requirements from `docs/requirements/software_requirements.md`:

- **REQ-SW-001**: Pet stat system (hunger, happiness, health, energy)
- **REQ-SW-002**: Life stages (baby → child → teen → adult)
- **REQ-SW-003**: Feeding mechanic (fish, shrimp)
- **REQ-SW-004**: Play mechanic (mini-game)
- **REQ-SW-011**: Menu system (7 items)
- **REQ-SW-020**: NVS save persistence

## Known Constraints

1. **No PSRAM**: TTGO has no external RAM, careful with allocations
2. **Limited IRAM**: Some functions moved to Flash
3. **No RTC**: Time tracking resets on power cycle
4. **Two buttons only**: All UI must work with navigate + select

## Sprite Organization

All sprites stored in Flash as RGB565 arrays in `sprites.c`:
- Egg sprites (24x28, 3 frames)
- Baby dolphin (32x24, 4 idle frames)
- Status icons (16x16)
- Menu icons (16x16)

Scaling: `display_draw_sprite_scaled()` does 2x or higher for visibility.

## Testing

Manual testing required on hardware. Key test scenarios:
1. Boot with no save → show splash → new game
2. Boot with save → load and resume
3. Let pet die → death screen → new game option
4. All menu actions work
5. Mini-game plays through all 3 rounds
