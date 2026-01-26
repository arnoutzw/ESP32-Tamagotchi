# ESP32 Tamagotchi - Software Requirements Specification

## Overview

Recreate the classic Tamagotchi virtual pet experience on ESP32 TTGO T-Display hardware, featuring a colorful baby dolphin as the pet character.

## Target Hardware

- **Platform**: TTGO T-Display ESP32
- **Display**: 1.14" ST7789 TFT IPS (240x135 pixels, 16-bit color)
- **Input**: 2x push buttons (GPIO 0 - Left, GPIO 35 - Right)
- **Storage**: 4MB Flash (NVS for save data)
- **Memory**: 520KB SRAM
- **Power**: USB or Li-Po battery with TP4054 charging

---

## Functional Requirements

### REQ-SW-001: Pet State System
**Priority**: Critical
**Description**: The dolphin pet shall have the following core attributes that change over time:
- **Hunger** (0-100): Decreases over time, replenished by feeding
- **Happiness** (0-100): Decreases over time, increased by playing
- **Health** (0-100): Affected by hunger and happiness levels
- **Energy** (0-100): Decreases with activity, restored by sleep
- **Age** (days): Increments based on real time elapsed
- **Weight** (1-99): Affected by feeding frequency

**Acceptance Criteria**:
- All stats visible via status screen
- Stats decay at configurable rates
- Pet can "die" if health reaches 0

### REQ-SW-002: Pet Life Stages
**Priority**: High
**Description**: The dolphin shall progress through life stages based on age:
- **Baby** (0-2 days): Small sprite, high care needs
- **Child** (3-6 days): Medium sprite, moderate needs
- **Teen** (7-13 days): Larger sprite, developing personality
- **Adult** (14+ days): Full-size sprite, stable needs

**Acceptance Criteria**:
- Different sprite for each life stage
- Smooth transition animations between stages

### REQ-SW-003: Feeding Mechanic
**Priority**: Critical
**Description**: User shall be able to feed the dolphin to restore hunger.
- Two food types: **Fish** (main meal) and **Shrimp** (snack/treat)
- Fish: +20 hunger, +5 weight
- Shrimp: +5 hunger, +10 happiness, +2 weight
- Overfeeding (hunger > 90 when feeding) causes -5 health

**Acceptance Criteria**:
- Eating animation plays when fed
- Visual feedback for overfeed warning
- Food selection menu

### REQ-SW-004: Play Mechanic
**Priority**: Critical
**Description**: User shall be able to play with the dolphin to increase happiness.
- Mini-game: "Jump the Wave" - time button press to make dolphin jump
- Success: +15 happiness, -10 energy
- Failure: +5 happiness, -5 energy
- Cannot play if energy < 20

**Acceptance Criteria**:
- Simple reaction-based mini-game
- Score displayed after game
- Energy check before playing

### REQ-SW-005: Sleep/Rest Mechanic
**Priority**: High
**Description**: The dolphin shall have a sleep cycle.
- Manual: User can put pet to sleep (button action)
- Auto: Pet sleeps at night (configurable schedule)
- Sleep restores +30 energy per hour
- Waking during sleep causes -10 happiness

**Acceptance Criteria**:
- Sleep animation (Z's floating)
- Day/night cycle on display
- Sleep/wake button action

### REQ-SW-006: Cleaning/Hygiene
**Priority**: Medium
**Description**: The dolphin produces waste that must be cleaned.
- Poop appears every 2-4 hours (random)
- Uncleaned poop: -5 health per hour
- Clean action removes poop with animation

**Acceptance Criteria**:
- Visual poop indicator
- Clean animation
- Health penalty for neglect

### REQ-SW-007: Sickness and Medicine
**Priority**: High
**Description**: The dolphin can get sick and require medicine.
- Sick when health < 30
- Sick sprite/animation displayed
- Medicine: +40 health, costs in-game currency (optional)
- Untreated sickness: health continues to decay faster

**Acceptance Criteria**:
- Sick visual indicator (sad face, different color)
- Medicine action available
- Recovery animation

### REQ-SW-008: Death and Reset
**Priority**: High
**Description**: If health reaches 0, the pet dies.
- Death screen displayed with final age
- Grave/memorial animation
- Option to start new pet

**Acceptance Criteria**:
- Death animation
- Final stats display
- New game option

---

## User Interface Requirements

### REQ-SW-010: Main Display
**Priority**: Critical
**Description**: Default screen showing the dolphin and key status indicators.
- Dolphin sprite centered on screen
- Status icons along top: hunger, happiness, health, energy
- Age display in corner
- Time of day indicator (sun/moon)

**Acceptance Criteria**:
- 240x135 pixel layout
- 60 FPS animation capability
- Icons visible at glance

### REQ-SW-011: Menu System
**Priority**: Critical
**Description**: Navigation menu accessible via button press.
- **Left Button**: Cycle through menu options
- **Right Button**: Select current option

Menu options:
1. Feed (Fish/Shrimp submenu)
2. Play (mini-game)
3. Sleep (put to bed / wake up)
4. Clean (if poop present)
5. Medicine (if sick)
6. Stats (detailed status screen)
7. Settings (brightness, sound)

**Acceptance Criteria**:
- Icon-based menu (like original Tamagotchi)
- Visual highlight on selected item
- Quick access from any screen

### REQ-SW-012: Button Input
**Priority**: Critical
**Description**: Two-button control scheme.
- **Left (GPIO 0)**: Navigate/Scroll/Previous
- **Right (GPIO 35)**: Select/Confirm/Action
- Long press (2s): Special actions (reset menu, attention)
- Button debouncing: 50ms

**Acceptance Criteria**:
- Responsive input (< 100ms latency)
- No false triggers from noise
- Both short and long press detection

### REQ-SW-013: Animations
**Priority**: High
**Description**: Smooth sprite animations for pet actions.
- Idle: Gentle swimming motion (4 frames)
- Eating: Chomping animation (3 frames)
- Playing: Jumping animation (5 frames)
- Sleeping: Floating with Z's (3 frames)
- Happy: Spinning/flipping (4 frames)
- Sad: Droopy/slow movement (3 frames)
- Sick: Pale color, slow movement
- Death: Fade to angel sprite

**Acceptance Criteria**:
- Minimum 10 FPS for animations
- Smooth frame transitions
- Context-appropriate animations

### REQ-SW-014: Status Icons
**Priority**: High
**Description**: Visual indicators for pet needs.
- **Hunger**: Fish bone icon (fills as hunger decreases)
- **Happiness**: Heart icon (fills based on happiness)
- **Health**: Cross/plus icon (fills based on health)
- **Energy**: Battery/lightning icon (fills based on energy)
- **Attention**: Exclamation mark when needs are critical

**Acceptance Criteria**:
- Icons change fill level (0%, 25%, 50%, 75%, 100%)
- Critical state (< 20%) causes icon to flash

---

## Data Persistence Requirements

### REQ-SW-020: Save State
**Priority**: Critical
**Description**: Pet state persists across power cycles using NVS.
- Auto-save every 5 minutes
- Save on power-off detection (if supported)
- Save all pet stats, age, and game state

**Acceptance Criteria**:
- State restored on boot
- No data loss on normal power cycle
- Graceful handling of corrupt save data

### REQ-SW-021: Time Tracking
**Priority**: High
**Description**: Track elapsed time for stat decay even when powered off.
- Store last save timestamp
- On boot, calculate time elapsed
- Apply stat changes based on time away (max 48 hours)

**Acceptance Criteria**:
- Stats decay realistically when away
- "Death" possible if away too long without care
- RTC or millis-based time tracking

---

## Technical Requirements

### REQ-SW-030: Display Driver
**Priority**: Critical
**Description**: ST7789 LCD driver optimized for TTGO T-Display.
- SPI interface at 40MHz
- Double-buffered rendering to prevent tearing
- Partial screen updates for efficiency

**Acceptance Criteria**:
- No visible tearing
- 30+ FPS achievable
- Low CPU overhead

### REQ-SW-031: Memory Management
**Priority**: Critical
**Description**: Efficient memory usage within 520KB SRAM limit.
- Sprite data in Flash (PROGMEM equivalent)
- Minimal heap allocation during runtime
- Target: < 100KB heap usage

**Acceptance Criteria**:
- No memory leaks
- Stable operation over 24+ hours
- Heap high-water mark logged

### REQ-SW-032: Power Management
**Priority**: Medium
**Description**: Optimize for battery operation.
- Reduce display brightness after 30s idle
- Option for deep sleep mode (pet sleeps too)
- Battery voltage monitoring (if ADC available)

**Acceptance Criteria**:
- Extended battery life vs full brightness
- Wake from sleep via button press
- Low battery warning

### REQ-SW-033: Frame Rate Control
**Priority**: High
**Description**: Maintain consistent animation frame rate.
- Target: 30 FPS for gameplay, 10 FPS for idle
- Frame timing via FreeRTOS tick
- Drop frames gracefully if overloaded

**Acceptance Criteria**:
- Smooth animations
- No stuttering during menu navigation
- Consistent timing for mini-games

### REQ-SW-XXX: Screen Orientation
**Priority**: Critical
**Description**:  Graphics shall be drawn in portrait mode

### REQ-SW-XXX: GUI Location
**Priority**: Critical
**Description**:  GUI shall only cover bottom 20% of the screen and shall be scrollable

### REQ-SW-XXX:  Button function
**Priority**: Critical
**Description**:  Left button shall navigate down in 

### REQ-SW-034: OTA Updates
**Priority**: Critical
**Description**: Allow for Over-The-Air (OTA) firmware updates with rollback capability.
- Enable OTA partition scheme (two OTA slots)
- Implement HTTP/HTTPS OTA update endpoint
- Automatic rollback to previous firmware if new firmware fails to boot
- Password-protected OTA endpoint

**Acceptance Criteria**:
- OTA update succeeds when valid firmware is uploaded
- Device automatically rolls back if new firmware fails boot verification
- OTA endpoint requires authentication
- Update progress feedback (if display available during update)

### REQ-SW-035: Battery Indicator
**Priority**: Critical
**Description**: Show a battery indicator to the user that predicts estimated SoC based on measured battery voltage from the hardware

**Acceptance Criteria**:
- Accurate indication of remaining runtime

### REQ-SW-036: WiFi Connectivity with AP Fallback
**Priority**: High
**Description**: Provide WiFi connectivity for OTA updates and optional network features.
- **Station (STA) mode**: Connect to configured home WiFi network
- **Access Point (AP) fallback**: If STA connection fails or no credentials configured, start AP mode
- **Credential storage**: Save WiFi SSID/password in NVS for persistence
- **Auto-reconnect**: Automatically reconnect if STA connection is lost

**WiFi Modes:**
1. **AP Mode** (default/fallback):
   - SSID: "Tamagotchi"
   - Password: configurable (default: "dolphin123")
   - IP: 192.168.4.1
   - Used for initial setup and OTA when no STA configured

2. **STA Mode** (preferred when configured):
   - Connects to user's home network
   - Enables NTP time sync
   - Enables remote OTA updates
   - Falls back to AP mode after N failed connection attempts

**Acceptance Criteria**:
- Device starts in AP mode if no WiFi credentials stored
- Device attempts STA connection if credentials exist
- Device falls back to AP mode after 5 failed STA connection attempts
- WiFi credentials persist across reboots
- OTA updates work in both AP and STA modes


### REQ-SW-037: Build System
**Priority**: Critical
**Description**: Standardized build system with embedded ESP-IDF and YAML-based configuration.
- ESP-IDF shall be embedded as a git submodule in the repo
- Build system shall follow ESP-IDF industry standards (CMake-based)
- Use YAML file (`config/secrets.yaml`) for compile-time configuration
- Python script generates C header from YAML configuration
- Generated headers shall not be committed to version control

**Acceptance Criteria**:
- ESP-IDF is available as git submodule at `esp-idf/` directory
- Build works with `idf.py build` after submodule init
- `config/secrets.yaml` contains all configurable secrets (WiFi, OTA passwords)
- `scripts/generate_config.py` generates `config_secrets.h` from YAML
- `.gitignore` excludes generated `config_secrets.h`
- Build instructions documented in README

---

## Stretch Goals (If Resources Permit)

### REQ-SW-040: Sound Effects
**Priority**: Descoped
**Description**: Audio feedback using PWM buzzer (if hardware added).
- Beep on button press
- Melody for happy events
- Alarm for attention needed

### REQ-SW-041: WiFi Features
**Priority**: Critical
**Description**: Optional network features.
- Sync time via NTP for accurate aging
- Share pet stats to web dashboard
- Visit other pets (multiplayer concept)

### REQ-SW-042: Personality Traits
**Priority**: Critical
**Description**: Dolphin develops personality based on care.
- Well-fed pets become "chubby" variant
- Happy pets become "playful" variant
- Neglected pets become "shy" variant

---

## Validation Tests

| Test ID | Requirement | Test Description |
|---------|-------------|------------------|
| VT-001 | REQ-SW-001 | Verify all stats display and decay correctly |
| VT-002 | REQ-SW-002 | Verify life stage transitions at correct ages |
| VT-003 | REQ-SW-003 | Verify feeding increases hunger, plays animation |
| VT-004 | REQ-SW-004 | Verify mini-game playable and affects stats |
| VT-005 | REQ-SW-005 | Verify sleep restores energy |
| VT-006 | REQ-SW-011 | Verify menu navigation with both buttons |
| VT-007 | REQ-SW-020 | Verify save/load across power cycle |
| VT-008 | REQ-SW-021 | Verify time-based stat decay after power off |
| VT-009 | REQ-SW-034 | Verify OTA update succeeds with valid firmware |
| VT-010 | REQ-SW-034 | Verify OTA rollback on boot failure |
| VT-011 | REQ-SW-036 | Verify AP mode starts when no credentials stored |
| VT-012 | REQ-SW-036 | Verify STA mode connects with valid credentials |
| VT-013 | REQ-SW-036 | Verify fallback to AP after STA connection failure |
| VT-014 | REQ-SW-037 | Verify ESP-IDF submodule initializes correctly |
| VT-015 | REQ-SW-037 | Verify config generation from YAML produces valid header |
| VT-016 | REQ-SW-037 | Verify build succeeds with generated config |
| VT-017 | REQ-SW-035 | Verify battery monitor initializes correctly |
| VT-018 | REQ-SW-035 | Verify battery voltage reading in valid range |
| VT-019 | REQ-SW-035 | Verify SoC percentage matches voltage |
| VT-020 | REQ-SW-041 | Verify NTP time sync within 5 seconds of WiFi connect |
| VT-021 | REQ-SW-041 | Verify time maintained across short power cycles |
| VT-022 | REQ-SW-041 | Verify web dashboard loads at device IP |
| VT-023 | REQ-SW-041 | Verify GET /api/status returns valid JSON |
| VT-024 | REQ-SW-042 | Verify chubby personality develops from overfeeding |
| VT-025 | REQ-SW-042 | Verify playful personality develops from high happiness |
| VT-026 | REQ-SW-042 | Verify shy personality develops from neglect |
| VT-027 | REQ-SW-042 | Verify personality saved and restored correctly |
| VT-028 | REQ-SW-042 | Verify personality affects stats as specified |

---

## Traceability Matrix

| Requirement | Implementation File | Test |
|-------------|---------------------|------|
| REQ-SW-001 | pet_state.c | VT-001 |
| REQ-SW-002 | pet_state.c, sprites.h | VT-002 |
| REQ-SW-003 | game_actions.c | VT-003 |
| REQ-SW-004 | minigame.c | VT-004 |
| REQ-SW-005 | pet_state.c, game_actions.c | VT-005 |
| REQ-SW-010 | display.c | - |
| REQ-SW-011 | menu.c | VT-006 |
| REQ-SW-012 | input.c | VT-006 |
| REQ-SW-020 | save_manager.c | VT-007 |
| REQ-SW-021 | time_manager.c | VT-008 |
| REQ-SW-034 | ota_manager.c, wifi_manager.c | VT-009, VT-010 |
| REQ-SW-036 | wifi_manager.c | VT-011, VT-012, VT-013 |
| REQ-SW-035 | battery.c | VT-017, VT-018, VT-019 |
| REQ-SW-037 | scripts/generate_config.py, config/secrets.yaml | VT-014, VT-015, VT-016 |
| REQ-SW-041 | ntp_manager.c, web_dashboard.c | VT-020, VT-021, VT-022, VT-023 |
| REQ-SW-042 | pet_state.c, sprites.c | VT-024, VT-025, VT-026, VT-027, VT-028 |
