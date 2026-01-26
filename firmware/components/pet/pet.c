/**
 * @file pet.c
 * @brief Pet state management implementation
 *
 * REQ-SW-001: Pet State System
 * REQ-SW-002: Pet Life Stages
 */

#include "pet.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_random.h"
#include <string.h>

static const char *TAG = "pet";

//=============================================================================
// Configuration Constants
//=============================================================================

// Stat decay rates (per minute)
#define HUNGER_DECAY_PER_MIN        2
#define HAPPINESS_DECAY_PER_MIN     1
#define ENERGY_DECAY_PER_MIN        1   // Only when awake
#define ENERGY_RESTORE_PER_MIN      5   // When sleeping

// Feeding effects
#define FISH_HUNGER_GAIN            20
#define FISH_WEIGHT_GAIN            3
#define SHRIMP_HUNGER_GAIN          8
#define SHRIMP_HAPPINESS_GAIN       10
#define SHRIMP_WEIGHT_GAIN          1
#define OVERFEED_PENALTY            5

// Play effects
#define PLAY_WIN_HAPPINESS          15
#define PLAY_WIN_ENERGY_COST        10
#define PLAY_LOSE_HAPPINESS         5
#define PLAY_LOSE_ENERGY_COST       5
#define PLAY_MIN_ENERGY             20

// Medicine effects
#define MEDICINE_HEALTH_RESTORE     40

// Poop timing (in minutes)
#define POOP_INTERVAL_MIN           30      // 30 minutes minimum
#define POOP_INTERVAL_MAX           90      // 90 minutes maximum
#define POOP_HEALTH_PENALTY_PER_MIN 1

// Life stages (in minutes)
#define EGG_DURATION_MIN            2       // 2 minutes to hatch
#define BABY_DURATION_MIN           (2 * 24 * 60)   // 2 days
#define CHILD_DURATION_MIN          (4 * 24 * 60)   // 4 more days (day 3-6)
#define TEEN_DURATION_MIN           (7 * 24 * 60)   // 7 more days (day 7-13)
// Adult: 14+ days

// Sickness threshold
#define SICK_THRESHOLD              30
#define SICK_DECAY_MULTIPLIER       2

//=============================================================================
// Static State
//=============================================================================

static pet_state_t s_pet = {0};

//=============================================================================
// Helper Functions
//=============================================================================

static inline uint32_t get_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static inline uint8_t clamp_stat(int32_t value)
{
    if (value < PET_STAT_MIN) return PET_STAT_MIN;
    if (value > PET_STAT_MAX) return PET_STAT_MAX;
    return (uint8_t)value;
}

static inline uint8_t clamp_weight(int32_t value)
{
    if (value < 1) return 1;
    if (value > 99) return 99;
    return (uint8_t)value;
}

static uint32_t random_range(uint32_t min, uint32_t max)
{
    return min + (esp_random() % (max - min + 1));
}

/**
 * @brief Update pet's mood based on current stats
 */
static void update_mood(void)
{
    if (s_pet.is_sleeping) {
        s_pet.mood = PET_MOOD_SLEEPING;
        return;
    }

    if (s_pet.is_sick) {
        s_pet.mood = PET_MOOD_SICK;
        return;
    }

    if (s_pet.hunger < PET_CRITICAL) {
        s_pet.mood = PET_MOOD_HUNGRY;
        return;
    }

    if (s_pet.energy < PET_CRITICAL) {
        s_pet.mood = PET_MOOD_SLEEPY;
        return;
    }

    if (s_pet.happiness < PET_CRITICAL) {
        s_pet.mood = PET_MOOD_SAD;
        return;
    }

    if (s_pet.happiness >= 80 && s_pet.hunger >= 60 && s_pet.health >= 70) {
        s_pet.mood = PET_MOOD_HAPPY;
        return;
    }

    s_pet.mood = PET_MOOD_NORMAL;
}

/**
 * @brief Update life stage based on age
 */
static void update_life_stage(void)
{
    if (s_pet.stage == PET_STAGE_DEAD) return;

    uint32_t age_min = s_pet.age_minutes;

    if (s_pet.stage == PET_STAGE_EGG) {
        if (age_min >= EGG_DURATION_MIN) {
            s_pet.stage = PET_STAGE_BABY;
            ESP_LOGI(TAG, "Pet hatched! Now a baby dolphin.");
        }
    } else if (age_min < BABY_DURATION_MIN) {
        s_pet.stage = PET_STAGE_BABY;
    } else if (age_min < BABY_DURATION_MIN + CHILD_DURATION_MIN) {
        if (s_pet.stage == PET_STAGE_BABY) {
            s_pet.stage = PET_STAGE_CHILD;
            ESP_LOGI(TAG, "Pet grew! Now a child dolphin.");
        }
    } else if (age_min < BABY_DURATION_MIN + CHILD_DURATION_MIN + TEEN_DURATION_MIN) {
        if (s_pet.stage == PET_STAGE_CHILD) {
            s_pet.stage = PET_STAGE_TEEN;
            ESP_LOGI(TAG, "Pet grew! Now a teen dolphin.");
        }
    } else {
        if (s_pet.stage == PET_STAGE_TEEN) {
            s_pet.stage = PET_STAGE_ADULT;
            ESP_LOGI(TAG, "Pet is fully grown! Now an adult dolphin.");
        }
    }
}

/**
 * @brief Calculate health based on other stats
 */
static void calculate_health(void)
{
    // Health is influenced by hunger, happiness, and cleanliness
    int32_t health_target = 100;

    // Hunger penalty
    if (s_pet.hunger < 50) {
        health_target -= (50 - s_pet.hunger) / 2;
    }

    // Happiness penalty
    if (s_pet.happiness < 40) {
        health_target -= (40 - s_pet.happiness) / 3;
    }

    // Poop penalty
    health_target -= s_pet.poop_count * 10;

    // Sickness accelerates health loss
    if (s_pet.is_sick) {
        health_target -= 20;
    }

    // Gradually move health towards target
    if (s_pet.health > health_target) {
        s_pet.health = clamp_stat(s_pet.health - 1);
    } else if (s_pet.health < health_target && !s_pet.is_sick) {
        s_pet.health = clamp_stat(s_pet.health + 1);
    }
}

//=============================================================================
// Initialization
//=============================================================================

esp_err_t pet_init(void)
{
    ESP_LOGI(TAG, "Initializing pet system");
    memset(&s_pet, 0, sizeof(s_pet));
    return ESP_OK;
}

void pet_new(void)
{
    ESP_LOGI(TAG, "Creating new pet (egg)");

    memset(&s_pet, 0, sizeof(s_pet));

    // Starting stats
    s_pet.hunger = 50;
    s_pet.happiness = 50;
    s_pet.health = 100;
    s_pet.energy = 100;
    s_pet.weight = 20;
    s_pet.discipline = 0;

    // Life state
    s_pet.stage = PET_STAGE_EGG;
    s_pet.age_minutes = 0;
    s_pet.birth_time = get_ms() / 1000;  // Approximate unix time

    // Activity state
    s_pet.mood = PET_MOOD_NORMAL;
    s_pet.activity = PET_ACTIVITY_HATCHING;
    s_pet.is_sick = false;
    s_pet.has_poop = false;
    s_pet.poop_count = 0;
    s_pet.is_sleeping = false;
    s_pet.attention_needed = false;

    // Timing
    uint32_t now = get_ms();
    s_pet.last_update_ms = now;
    s_pet.last_fed_ms = now;
    s_pet.last_played_ms = now;
    s_pet.last_poop_ms = now;
    s_pet.sleep_start_ms = 0;

    // Stats tracking
    s_pet.games_won = 0;
    s_pet.games_played = 0;
    s_pet.times_fed = 0;
    s_pet.times_played = 0;
    s_pet.times_cleaned = 0;
    s_pet.times_medicated = 0;
}

const pet_state_t *pet_get_state(void)
{
    return &s_pet;
}

pet_state_t *pet_get_state_mutable(void)
{
    return &s_pet;
}

//=============================================================================
// Core Update
//=============================================================================

void pet_update(uint32_t delta_ms)
{
    if (s_pet.stage == PET_STAGE_DEAD) return;

    uint32_t now = get_ms();
    uint32_t elapsed_min = delta_ms / 60000;  // Minutes elapsed

    // Only apply decay if at least 1 minute has passed
    if (elapsed_min > 0) {
        // Age increases
        s_pet.age_minutes += elapsed_min;

        // Stat decay (only if past egg stage)
        if (s_pet.stage != PET_STAGE_EGG) {
            // Hunger decay
            uint8_t hunger_decay = elapsed_min * HUNGER_DECAY_PER_MIN;
            if (s_pet.is_sick) hunger_decay *= SICK_DECAY_MULTIPLIER;
            s_pet.hunger = clamp_stat(s_pet.hunger - hunger_decay);

            // Happiness decay
            uint8_t happy_decay = elapsed_min * HAPPINESS_DECAY_PER_MIN;
            s_pet.happiness = clamp_stat(s_pet.happiness - happy_decay);

            // Energy management
            if (s_pet.is_sleeping) {
                // Restore energy while sleeping
                s_pet.energy = clamp_stat(s_pet.energy + elapsed_min * ENERGY_RESTORE_PER_MIN);

                // Wake up automatically if fully rested
                if (s_pet.energy >= 100) {
                    pet_wake();
                }
            } else {
                // Drain energy while awake
                s_pet.energy = clamp_stat(s_pet.energy - elapsed_min * ENERGY_DECAY_PER_MIN);
            }

            // Poop generation (random chance based on time since last poop)
            uint32_t since_poop_min = (now - s_pet.last_poop_ms) / 60000;
            if (since_poop_min >= POOP_INTERVAL_MIN && !s_pet.is_sleeping) {
                uint32_t poop_chance = (since_poop_min - POOP_INTERVAL_MIN) * 100 /
                                        (POOP_INTERVAL_MAX - POOP_INTERVAL_MIN);
                if (random_range(0, 100) < poop_chance) {
                    s_pet.has_poop = true;
                    s_pet.poop_count++;
                    s_pet.last_poop_ms = now;
                    ESP_LOGI(TAG, "Pet made poop! Total: %d", s_pet.poop_count);
                }
            }

            // Poop health penalty
            if (s_pet.has_poop) {
                s_pet.health = clamp_stat(s_pet.health - s_pet.poop_count * POOP_HEALTH_PENALTY_PER_MIN);
            }

            // Calculate overall health
            calculate_health();

            // Check for sickness
            if (s_pet.health < SICK_THRESHOLD && !s_pet.is_sick) {
                s_pet.is_sick = true;
                ESP_LOGW(TAG, "Pet got sick! Health: %d", s_pet.health);
            }

            // Check for death
            if (s_pet.health == 0) {
                s_pet.stage = PET_STAGE_DEAD;
                s_pet.activity = PET_ACTIVITY_IDLE;
                ESP_LOGE(TAG, "Pet died! Age: %lu minutes", (unsigned long)s_pet.age_minutes);
            }
        }

        // Update life stage
        update_life_stage();
    }

    // Update mood (always)
    update_mood();

    // Check attention
    s_pet.attention_needed = (s_pet.hunger < PET_CRITICAL ||
                              s_pet.happiness < PET_CRITICAL ||
                              s_pet.health < PET_CRITICAL ||
                              s_pet.energy < PET_CRITICAL ||
                              s_pet.has_poop ||
                              s_pet.is_sick);

    s_pet.last_update_ms = now;
}

void pet_apply_time_away(uint32_t away_minutes)
{
    if (away_minutes == 0) return;

    ESP_LOGI(TAG, "Applying %lu minutes of time away", (unsigned long)away_minutes);

    // Cap at 48 hours
    if (away_minutes > 48 * 60) {
        away_minutes = 48 * 60;
    }

    // Apply accelerated decay
    pet_update(away_minutes * 60000);
}

//=============================================================================
// Actions
//=============================================================================

bool pet_feed(food_type_t food)
{
    if (s_pet.stage == PET_STAGE_DEAD || s_pet.stage == PET_STAGE_EGG) return false;
    if (s_pet.is_sleeping) return false;

    bool overfed = s_pet.hunger >= PET_OVERFEED;

    if (food == FOOD_FISH) {
        s_pet.hunger = clamp_stat(s_pet.hunger + FISH_HUNGER_GAIN);
        s_pet.weight = clamp_weight(s_pet.weight + FISH_WEIGHT_GAIN);
        ESP_LOGI(TAG, "Fed fish. Hunger: %d, Weight: %d", s_pet.hunger, s_pet.weight);
    } else {
        s_pet.hunger = clamp_stat(s_pet.hunger + SHRIMP_HUNGER_GAIN);
        s_pet.happiness = clamp_stat(s_pet.happiness + SHRIMP_HAPPINESS_GAIN);
        s_pet.weight = clamp_weight(s_pet.weight + SHRIMP_WEIGHT_GAIN);
        ESP_LOGI(TAG, "Fed shrimp. Hunger: %d, Happy: %d", s_pet.hunger, s_pet.happiness);
    }

    if (overfed) {
        s_pet.health = clamp_stat(s_pet.health - OVERFEED_PENALTY);
        ESP_LOGW(TAG, "Overfed! Health penalty applied: %d", s_pet.health);
    }

    s_pet.activity = PET_ACTIVITY_EATING;
    s_pet.last_fed_ms = get_ms();
    s_pet.times_fed++;

    return true;
}

bool pet_play_start(void)
{
    if (s_pet.stage == PET_STAGE_DEAD || s_pet.stage == PET_STAGE_EGG) return false;
    if (s_pet.is_sleeping) return false;
    if (s_pet.energy < PLAY_MIN_ENERGY) return false;

    s_pet.activity = PET_ACTIVITY_PLAYING;
    s_pet.last_played_ms = get_ms();
    s_pet.games_played++;

    return true;
}

void pet_play_complete(bool won)
{
    if (won) {
        s_pet.happiness = clamp_stat(s_pet.happiness + PLAY_WIN_HAPPINESS);
        s_pet.energy = clamp_stat(s_pet.energy - PLAY_WIN_ENERGY_COST);
        s_pet.games_won++;
        ESP_LOGI(TAG, "Game won! Happy: %d, Energy: %d", s_pet.happiness, s_pet.energy);
    } else {
        s_pet.happiness = clamp_stat(s_pet.happiness + PLAY_LOSE_HAPPINESS);
        s_pet.energy = clamp_stat(s_pet.energy - PLAY_LOSE_ENERGY_COST);
        ESP_LOGI(TAG, "Game lost. Happy: %d, Energy: %d", s_pet.happiness, s_pet.energy);
    }

    s_pet.times_played++;
    s_pet.activity = PET_ACTIVITY_IDLE;
}

bool pet_sleep(void)
{
    if (s_pet.stage == PET_STAGE_DEAD || s_pet.stage == PET_STAGE_EGG) return false;
    if (s_pet.is_sleeping) return false;

    s_pet.is_sleeping = true;
    s_pet.sleep_start_ms = get_ms();
    s_pet.activity = PET_ACTIVITY_SLEEPING;

    ESP_LOGI(TAG, "Pet went to sleep. Energy: %d", s_pet.energy);
    return true;
}

bool pet_wake(void)
{
    if (!s_pet.is_sleeping) return false;

    // Penalty for waking early (if energy not full)
    if (s_pet.energy < 80) {
        s_pet.happiness = clamp_stat(s_pet.happiness - 10);
        ESP_LOGW(TAG, "Woken early! Happiness penalty.");
    }

    s_pet.is_sleeping = false;
    s_pet.sleep_start_ms = 0;
    s_pet.activity = PET_ACTIVITY_IDLE;

    ESP_LOGI(TAG, "Pet woke up. Energy: %d", s_pet.energy);
    return true;
}

void pet_toggle_sleep(void)
{
    if (s_pet.is_sleeping) {
        pet_wake();
    } else {
        pet_sleep();
    }
}

bool pet_clean(void)
{
    if (!s_pet.has_poop) return false;

    s_pet.has_poop = false;
    s_pet.poop_count = 0;
    s_pet.times_cleaned++;

    ESP_LOGI(TAG, "Cleaned up poop!");
    return true;
}

bool pet_give_medicine(void)
{
    if (!s_pet.is_sick) return false;

    s_pet.health = clamp_stat(s_pet.health + MEDICINE_HEALTH_RESTORE);
    s_pet.is_sick = false;
    s_pet.times_medicated++;

    ESP_LOGI(TAG, "Gave medicine. Health: %d", s_pet.health);
    return true;
}

//=============================================================================
// Queries
//=============================================================================

bool pet_is_alive(void)
{
    return s_pet.stage != PET_STAGE_DEAD;
}

bool pet_needs_attention(void)
{
    return s_pet.attention_needed;
}

bool pet_can_play(void)
{
    return s_pet.energy >= PLAY_MIN_ENERGY &&
           s_pet.stage != PET_STAGE_DEAD &&
           s_pet.stage != PET_STAGE_EGG &&
           !s_pet.is_sleeping;
}

uint32_t pet_get_age_days(void)
{
    return s_pet.age_minutes / (24 * 60);
}

const char *pet_get_stage_name(void)
{
    switch (s_pet.stage) {
        case PET_STAGE_EGG:   return "Egg";
        case PET_STAGE_BABY:  return "Baby";
        case PET_STAGE_CHILD: return "Child";
        case PET_STAGE_TEEN:  return "Teen";
        case PET_STAGE_ADULT: return "Adult";
        case PET_STAGE_DEAD:  return "Dead";
        default:              return "Unknown";
    }
}

const char *pet_get_mood_name(void)
{
    switch (s_pet.mood) {
        case PET_MOOD_HAPPY:    return "Happy";
        case PET_MOOD_NORMAL:   return "Normal";
        case PET_MOOD_SAD:      return "Sad";
        case PET_MOOD_HUNGRY:   return "Hungry";
        case PET_MOOD_SLEEPY:   return "Sleepy";
        case PET_MOOD_SICK:     return "Sick";
        case PET_MOOD_SLEEPING: return "Sleeping";
        default:                return "Unknown";
    }
}

uint8_t pet_get_overall_happiness(void)
{
    // Weighted average of stats
    return (uint8_t)((s_pet.hunger * 25 + s_pet.happiness * 35 +
                      s_pet.health * 25 + s_pet.energy * 15) / 100);
}
