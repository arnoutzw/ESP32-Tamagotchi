# Plan: REQ-SW-035 Battery Indicator Implementation

## Overview

Implement battery voltage monitoring and State of Charge (SoC) estimation for LiPo batteries on TTGO T-Display hardware.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: Users need to know battery level to avoid unexpected shutdowns. Critical for portable use.
- **Dependencies**: Display component (for showing indicator)

## Implementation Details

### Hardware

The TTGO T-Display has a voltage divider circuit on GPIO 34:
- 100K + 100K divider = 2:1 ratio
- Battery voltage range: 3.0V (empty) to 4.2V (full)
- ADC input range: 1.5V to 2.1V (with divider)

### LiPo Discharge Curve

Used piecewise linear approximation:
```
Voltage    SoC
4.2V       100%
3.9V       75%
3.7V       50%
3.5V       25%
3.3V       10%
3.0V       0%
```

### Battery Levels

| Level    | SoC Range | Icon |
|----------|-----------|------|
| FULL     | 80-100%   | Full battery |
| HIGH     | 60-80%    | 3/4 battery |
| MEDIUM   | 40-60%    | Half battery |
| LOW      | 20-40%    | 1/4 battery (warning) |
| CRITICAL | 0-20%     | Empty battery (flash) |

## Files Created

| File | Description |
|------|-------------|
| `firmware/components/battery/include/battery.h` | Public API |
| `firmware/components/battery/battery.c` | Implementation |
| `firmware/components/battery/CMakeLists.txt` | Component registration |

## API

```c
esp_err_t battery_init(void);
float battery_read_voltage(void);
uint8_t battery_get_soc(void);
battery_level_t battery_get_level(void);
void battery_get_status(battery_status_t *status);
bool battery_is_low(void);
```

## Integration

- Initialized in `app_main()` after OTA manager
- Logs battery status on startup
- Available for display rendering (future: battery icon on main screen)

## Verification Tests

| Test ID | Description | Pass Criteria |
|---------|-------------|---------------|
| VT-017 | Battery init | `battery_init()` returns ESP_OK |
| VT-018 | Voltage reading | Voltage in valid range (2.5V - 4.5V) |
| VT-019 | SoC calculation | SoC matches voltage (within 5%) |

## Status

- [x] Phase 1: Battery component created (completed 2026-01-26)
- [x] Phase 2: ADC initialization with calibration
- [x] Phase 3: Voltage to SoC conversion
- [x] Phase 4: Integration in main.c
- [ ] Phase 5: Display integration (battery icon on screen)
- [ ] Phase 6: Low battery warning

## Future Enhancements

- Add battery icon to main game screen
- Flash icon when battery critical
- USB power detection (charging indicator)
- Power saving mode when battery low
