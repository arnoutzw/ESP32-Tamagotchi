/**
 * @file config.h
 * @brief ESP32 Tamagotchi - Hardware and game configuration
 *
 * Central configuration for TTGO T-Display hardware pins and game parameters.
 */

#ifndef CONFIG_H
#define CONFIG_H

//=============================================================================
// Hardware Configuration - TTGO T-Display
//=============================================================================

// Display - ST7789 1.14" TFT (240x135)
#define LCD_WIDTH           240
#define LCD_HEIGHT          135
#define LCD_SPI_HOST        SPI2_HOST
#define LCD_PIN_MOSI        19
#define LCD_PIN_SCLK        18
#define LCD_PIN_CS          5
#define LCD_PIN_DC          16
#define LCD_PIN_RST         23
#define LCD_PIN_BL          4
#define LCD_SPI_CLOCK_HZ    (40 * 1000 * 1000)

// ST7789 display offset (240x320 panel showing 135x240 window)
#define LCD_COL_OFFSET      52
#define LCD_ROW_OFFSET      40

// Buttons
#define BUTTON_LEFT_GPIO    0   // Boot button
#define BUTTON_RIGHT_GPIO   35  // User button

// Battery ADC (TTGO T-Display)
#define BATTERY_ADC_GPIO    34  // Battery voltage with divider

// Button timing
#define BUTTON_DEBOUNCE_MS      50
#define BUTTON_LONG_PRESS_MS    2000

// Backlight PWM
#define LCD_BL_PWM_CHANNEL      0
#define LCD_BL_PWM_FREQ_HZ      5000
#define LCD_BL_PWM_RESOLUTION   8
#define LCD_BL_DUTY_MAX         255

//=============================================================================
// Game Configuration
//=============================================================================

// Pet stats range
#define STAT_MIN                0
#define STAT_MAX                100
#define STAT_CRITICAL           20  // Below this, icon flashes

// Stat decay rates (points per minute)
#define HUNGER_DECAY_RATE       2   // ~50 points over ~25 min
#define HAPPINESS_DECAY_RATE    1   // ~50 points over ~50 min
#define ENERGY_DECAY_RATE       1   // Slower decay when active

// Health calculation weights
#define HEALTH_HUNGER_WEIGHT    0.4f
#define HEALTH_HAPPINESS_WEIGHT 0.3f
#define HEALTH_CLEANLINESS_WEIGHT 0.3f

// Feeding effects
#define FISH_HUNGER_RESTORE     20
#define FISH_WEIGHT_GAIN        5
#define SHRIMP_HUNGER_RESTORE   5
#define SHRIMP_HAPPINESS_GAIN   10
#define SHRIMP_WEIGHT_GAIN      2
#define OVERFEED_HEALTH_PENALTY 5
#define OVERFEED_THRESHOLD      90

// Play effects
#define PLAY_SUCCESS_HAPPINESS  15
#define PLAY_SUCCESS_ENERGY     10
#define PLAY_FAIL_HAPPINESS     5
#define PLAY_FAIL_ENERGY        5
#define PLAY_MIN_ENERGY         20

// Sleep effects
#define SLEEP_ENERGY_RESTORE_PER_MIN    5   // +30 per 6 min
#define WAKE_EARLY_HAPPINESS_PENALTY    10

// Poop timing
#define POOP_INTERVAL_MIN_MS    (2 * 60 * 60 * 1000)  // 2 hours
#define POOP_INTERVAL_MAX_MS    (4 * 60 * 60 * 1000)  // 4 hours
#define POOP_HEALTH_PENALTY     5   // Per hour uncleaned

// Sickness
#define SICK_THRESHOLD          30
#define MEDICINE_HEALTH_RESTORE 40

// Life stages (in days)
#define STAGE_BABY_MAX_DAYS     2
#define STAGE_CHILD_MAX_DAYS    6
#define STAGE_TEEN_MAX_DAYS     13
// Adult: 14+ days

// Time tracking
#define SAVE_INTERVAL_MS        (5 * 60 * 1000)  // 5 minutes
#define MAX_OFFLINE_HOURS       48
#define GAME_TICK_MS            1000  // 1 second game tick

// Animation
#define ANIMATION_FPS_IDLE      10
#define ANIMATION_FPS_ACTIVE    30
#define FRAME_TIME_IDLE_MS      (1000 / ANIMATION_FPS_IDLE)
#define FRAME_TIME_ACTIVE_MS    (1000 / ANIMATION_FPS_ACTIVE)

// Display brightness
#define BRIGHTNESS_NORMAL       200
#define BRIGHTNESS_DIM          50
#define DIM_TIMEOUT_MS          30000  // 30 seconds

//=============================================================================
// Color Definitions (RGB565)
//=============================================================================

#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_CYAN          0x07FF
#define COLOR_MAGENTA       0xF81F
#define COLOR_YELLOW        0xFFE0
#define COLOR_ORANGE        0xFD20
#define COLOR_PINK          0xFE19

// Game-specific colors
#define COLOR_OCEAN_LIGHT   0x5D9F  // Light blue ocean
#define COLOR_OCEAN_DARK    0x2B4D  // Dark blue ocean
#define COLOR_DOLPHIN_BODY  0x7BEF  // Gray-blue dolphin
#define COLOR_DOLPHIN_BELLY 0xC638  // Light gray belly
#define COLOR_SAND          0xE6D4  // Sandy beach
#define COLOR_CORAL         0xFB2C  // Coral pink

#endif // CONFIG_H
