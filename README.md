# ESP32 Tamagotchi - Dolphin Pet

A virtual pet game inspired by the classic Tamagotchi, featuring a colorful baby dolphin on the TTGO T-Display ESP32.

## Features

- **Virtual Dolphin Pet**: Cute baby dolphin that grows through life stages (egg → baby → child → teen → adult)
- **Care Activities**: Feed, play, clean, and give medicine to your pet
- **Stat System**: Hunger, happiness, health, and energy - all decay over time
- **Mini-Game**: "Jump the Wave" reaction game to increase happiness
- **Full Color Display**: 240x135 TFT with custom pixel art sprites
- **Persistent Save**: Game state saved to NVS flash, survives power cycles
- **Two-Button Control**: Simple navigation like the original Tamagotchi

## Hardware Requirements

- **TTGO T-Display ESP32**
  - ESP32 dual-core 240MHz
  - 1.14" ST7789 TFT IPS display (240x135)
  - 2 push buttons (GPIO 0, GPIO 35)
  - USB-C power/programming
  - Optional: Li-Po battery

## Quick Start

### 1. Setup ESP-IDF

```bash
# Install ESP-IDF v5.2+ if not already installed
# See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
```

### 2. Build and Flash

```bash
cd firmware

# Configure for ESP32
idf.py set-target esp32

# Build
idf.py build

# Flash (adjust port as needed)
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

## Controls

| Button | Action |
|--------|--------|
| **Left (GPIO 0)** | Navigate / Scroll |
| **Right (GPIO 35)** | Select / Confirm |
| Both long press | (Reserved for future use) |

## Menu Options

1. **Feed**: Choose Fish (hunger+20) or Shrimp (hunger+5, happiness+10)
2. **Play**: Jump the Wave mini-game (happiness++, energy--)
3. **Sleep**: Put pet to bed (energy restores while sleeping)
4. **Clean**: Remove poop (prevents health penalty)
5. **Medicine**: Cure sickness (when health < 30%)
6. **Stats**: View detailed pet statistics
7. **Settings**: (Placeholder for brightness, etc.)

## Pet Care Guide

### Stats

| Stat | Decay Rate | Effect |
|------|------------|--------|
| Hunger | 2/min | Pet gets hungry, health decreases |
| Happiness | 1/min | Sad pet, affects overall mood |
| Health | Calculated | Based on hunger, happiness, cleanliness |
| Energy | 1/min awake | Too tired = can't play |

### Life Stages

| Stage | Age | Sprite Size |
|-------|-----|-------------|
| Egg | 0-2 min | Small |
| Baby | 0-2 days | 32x24 |
| Child | 3-6 days | 40x32 |
| Teen | 7-13 days | 48x36 |
| Adult | 14+ days | 56x40 |

### Tips

- Feed fish for hunger, shrimp for happiness
- Don't overfeed (hunger > 90% causes health penalty)
- Clean poop promptly to prevent sickness
- Let pet sleep to restore energy
- Play mini-games to boost happiness

## Project Structure

```
ESP32_tamagotchi/
├── firmware/
│   ├── main/
│   │   ├── main.c              # Entry point
│   │   └── config.h            # Hardware configuration
│   ├── components/
│   │   ├── display/            # ST7789 LCD driver
│   │   ├── input/              # Button handling
│   │   ├── pet/                # Pet state management
│   │   ├── game/               # Game logic & mini-games
│   │   ├── sprites/            # Pixel art graphics
│   │   └── save_manager/       # NVS persistence
│   ├── CMakeLists.txt
│   └── sdkconfig.defaults
├── docs/
│   └── requirements/
│       ├── software_requirements.md
│       └── electrical_requirements.md
├── CLAUDE/
│   └── rules.md                # AI assistant guidelines
└── README.md
```

## Memory Usage

- **Flash**: ~1MB (sprites + code)
- **RAM**: ~60KB heap usage
- **NVS**: ~1KB save data

## Future Enhancements

- [ ] More sprite animations
- [ ] Sound effects (PWM buzzer)
- [ ] WiFi time sync for accurate aging
- [ ] Multiple pet personalities
- [ ] Additional mini-games
- [ ] Battery voltage display

## License

MIT License

## Acknowledgments

- Original Tamagotchi by Bandai (1996)
- ESP-IDF by Espressif
- TTGO T-Display hardware
