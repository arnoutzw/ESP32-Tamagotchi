# Plan: REQ-SW-042 Personality Traits Implementation

## Overview

Implement a personality system where the dolphin develops distinct traits based on care patterns. The pet's personality affects its appearance, animations, and behavior.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: Personality traits add replayability and emotional engagement. Different care patterns lead to different outcomes, encouraging players to try different approaches. This is a core differentiator from the original Tamagotchi.
- **Dependencies**:
  - REQ-SW-001 (Pet State System) - For tracking care patterns
  - REQ-SW-013 (Animations) - For personality-specific animations
  - REQ-SW-020 (Save State) - For persisting personality

## Personality Types

### 1. Chubby Variant
**Trigger**: Consistently overfed (feeding when hunger > 70 frequently)
- **Appearance**: Rounder sprite, bigger belly
- **Behavior**: Slower movement, eats faster
- **Stat modifiers**: +10% weight gain, -10% energy decay

### 2. Playful Variant
**Trigger**: High happiness maintained (happiness > 80 for extended periods)
- **Appearance**: Brighter colors, more expressive eyes
- **Behavior**: More active idle animations, jumps more
- **Stat modifiers**: +10% happiness from play, -10% energy cost

### 3. Shy Variant
**Trigger**: Neglected (stats frequently below 30, long periods without interaction)
- **Appearance**: Duller colors, smaller-looking
- **Behavior**: Slower animations, hides more
- **Stat modifiers**: -10% happiness gain, slower to warm up

### 4. Athletic Variant
**Trigger**: Frequent play sessions, good balance of food and exercise
- **Appearance**: Sleeker sprite, more muscular
- **Behavior**: Faster animations, eager to play
- **Stat modifiers**: -10% energy decay, better mini-game performance

### 5. Balanced/Normal Variant
**Trigger**: Default, average care without strong patterns
- **Appearance**: Standard dolphin sprite
- **Behavior**: Normal animations
- **Stat modifiers**: None

## Implementation Steps

### Phase 1: Personality Tracking System

1. **Add personality tracking to pet state**
   ```c
   typedef enum {
       PERSONALITY_NORMAL,
       PERSONALITY_CHUBBY,
       PERSONALITY_PLAYFUL,
       PERSONALITY_SHY,
       PERSONALITY_ATHLETIC
   } personality_t;

   typedef struct {
       uint16_t overfeed_count;      // Times fed when hunger > 70
       uint16_t high_happiness_mins; // Minutes with happiness > 80
       uint16_t neglect_count;       // Times stats dropped below 30
       uint16_t play_sessions;       // Total play sessions
       uint16_t balanced_days;       // Days with all stats > 50
   } personality_tracking_t;
   ```

2. **Update pet_state.h/c**
   - Add `personality_t current_personality`
   - Add `personality_tracking_t tracking`
   - Add `pet_update_personality()` function
   - Call personality check every game day

### Phase 2: Personality Determination Logic

3. **Implement personality calculation**
   ```c
   personality_t calculate_personality(const personality_tracking_t *tracking, uint16_t age_days) {
       // Only develop personality after 3 days (child stage)
       if (age_days < 3) return PERSONALITY_NORMAL;

       // Check thresholds (example values, tune during testing)
       if (tracking->overfeed_count > age_days * 2) return PERSONALITY_CHUBBY;
       if (tracking->high_happiness_mins > age_days * 60) return PERSONALITY_PLAYFUL;
       if (tracking->neglect_count > age_days * 3) return PERSONALITY_SHY;
       if (tracking->play_sessions > age_days * 4 &&
           tracking->balanced_days > age_days / 2) return PERSONALITY_ATHLETIC;

       return PERSONALITY_NORMAL;
   }
   ```

### Phase 3: Sprite Variants

4. **Create personality-specific sprites**
   - Modify existing sprites or create overlays
   - Color palette shifts for each personality
   - Size adjustments (chubby = larger, shy = smaller)

5. **Update sprite selection in game rendering**
   - `get_pet_sprite()` considers personality
   - Personality affects idle animation selection

### Phase 4: Behavior Modifiers

6. **Apply stat modifiers based on personality**
   ```c
   float get_happiness_multiplier(personality_t p) {
       switch (p) {
           case PERSONALITY_PLAYFUL: return 1.1f;
           case PERSONALITY_SHY: return 0.9f;
           default: return 1.0f;
       }
   }
   ```

7. **Animation speed/style modifiers**
   - Playful: faster idle animation
   - Shy: slower, more hesitant
   - Chubby: slower movement
   - Athletic: snappier reactions

### Phase 5: Persistence

8. **Update save manager**
   - Add personality and tracking data to save structure
   - Ensure backward compatibility with old saves

### Phase 6: UI Integration

9. **Show personality in stats screen**
   - Display current personality name
   - Optional: show personality progress bars

## Files to Create/Modify

| File | Action | Description |
|------|--------|-------------|
| `firmware/components/pet/include/pet.h` | Modify | Add personality types and tracking |
| `firmware/components/pet/pet_state.c` | Modify | Implement personality tracking and calculation |
| `firmware/components/sprites/sprites.h` | Modify | Add personality sprite variants |
| `firmware/components/sprites/sprites.c` | Modify | Implement sprite selection by personality |
| `firmware/components/game/game.c` | Modify | Apply stat modifiers |
| `firmware/components/save_manager/save_manager.c` | Modify | Save/load personality data |

## Memory Impact

| Component | Flash | RAM |
|-----------|-------|-----|
| Personality tracking | ~500B | ~32B |
| Sprite variants | ~5KB per variant | 0 (in flash) |
| Stat modifiers | ~200B | 0 |

## Sprite Design Notes

For initial implementation, use color palette swaps rather than entirely new sprites:
- **Chubby**: Warmer colors (more pink/orange tint)
- **Playful**: Brighter, more saturated colors
- **Shy**: Desaturated, cooler colors
- **Athletic**: Cooler blue tones, sharper contrast

## Verification Tests

| Test ID | Description | Pass Criteria |
|---------|-------------|---------------|
| VT-024 | Chubby development | Overfeeding 10+ times triggers chubby personality |
| VT-025 | Playful development | High happiness for 2+ days triggers playful |
| VT-026 | Shy development | Neglecting pet triggers shy personality |
| VT-027 | Personality persistence | Personality saved and restored correctly |
| VT-028 | Stat modifiers | Personality affects stats as specified |

## Status

- [ ] Phase 1: Personality Tracking System
- [ ] Phase 2: Personality Determination Logic
- [ ] Phase 3: Sprite Variants
- [ ] Phase 4: Behavior Modifiers
- [ ] Phase 5: Persistence
- [ ] Phase 6: UI Integration

## Implementation Notes

*To be updated during implementation*

## Future Enhancements

- Personality can slowly change if care patterns change drastically
- Rare "secret" personalities for very specific care patterns
- Personality affects mini-game difficulty/rewards
- Personality-specific dialogue/thought bubbles
