# Plan: REQ-SW-043 Button Functions Implementation

## Overview

Define and implement button functions for the two-button TTGO T-Display interface with short press and long press actions for intuitive navigation.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: Buttons are the only user input method. Clear, intuitive button mapping is essential for usability.
- **Dependencies**:
  - REQ-SW-038 (Screen Orientation) - affects visual feedback
  - REQ-SW-039 (GUI Location) - menu system to control

## Button Hardware

| Button | GPIO | Physical Location | ESP-IDF Name |
|--------|------|-------------------|--------------|
| Left | GPIO 0 | Left side (Boot) | BUTTON_LEFT_GPIO |
| Right | GPIO 35 | Right side (User) | BUTTON_RIGHT_GPIO |

## Button Function Mapping

### Main Game Mode (Pet Visible)

| Button | Press Type | Action | Visual Feedback |
|--------|------------|--------|-----------------|
| Left | Short | Scroll down/Previous | Highlight movement |
| Left | Long | Back/Cancel | Cancel animation |
| Right | Short | Scroll up/Next | Highlight movement |
| Right | Long | Confirm/Select | Selection animation |

### Menu Mode (Menu Active)

| Button | Press Type | Action | Visual Feedback |
|--------|------------|--------|-----------------|
| Left | Short | Scroll menu left | Menu scrolls |
| Left | Long | Deactivate menu | Menu closes |
| Right | Short | Scroll menu right | Menu scrolls |
| Right | Long | Select menu item | Item activated |

### Submenu/Action Mode

| Button | Press Type | Action | Visual Feedback |
|--------|------------|--------|-----------------|
| Left | Short | Cycle option left | Option changes |
| Left | Long | Cancel/Back | Return to menu |
| Right | Short | Cycle option right | Option changes |
| Right | Long | Confirm action | Action executes |

## Implementation Steps

### Phase 1: Button Event System

1. **Refactor button handling** in `main.c`
   - Current: simple pressed flags
   - Target: event-based system with press types

2. **Button event types**
   ```c
   typedef enum {
       BUTTON_EVENT_NONE,
       BUTTON_EVENT_SHORT_PRESS,
       BUTTON_EVENT_LONG_PRESS,
       BUTTON_EVENT_RELEASE
   } button_event_t;

   typedef enum {
       BUTTON_LEFT,
       BUTTON_RIGHT
   } button_id_t;

   typedef struct {
       button_id_t button;
       button_event_t event;
       uint32_t press_duration_ms;
   } button_state_t;
   ```

3. **Button timing configuration** (in config.h)
   ```c
   #define BUTTON_DEBOUNCE_MS          50    // Already exists
   #define BUTTON_LONG_PRESS_MS        800   // Reduce from 2000ms
   #define BUTTON_REPEAT_DELAY_MS      500   // Initial repeat delay
   #define BUTTON_REPEAT_RATE_MS       150   // Repeat interval
   ```

### Phase 2: Button State Machine

4. **Create button manager** (`firmware/components/input/`)
   - `input.h` - Public API
   - `input.c` - Implementation

5. **Button Manager API**
   ```c
   esp_err_t input_init(void);
   void input_update(void);  // Call from main loop
   bool input_get_event(button_state_t *event);
   bool input_is_pressed(button_id_t button);
   uint32_t input_get_press_duration(button_id_t button);
   ```

6. **Button state machine**
   ```
   IDLE ──[press]──► PRESSED ──[>800ms]──► LONG_TRIGGERED
     ▲                  │                        │
     │                  │                        │
     └──[release]───────┴────────[release]───────┘
   ```

### Phase 3: Context-Aware Input Handling

7. **Game mode context**
   ```c
   typedef enum {
       INPUT_CONTEXT_GAME,      // Main pet view
       INPUT_CONTEXT_MENU,      // Menu active
       INPUT_CONTEXT_SUBMENU,   // In a submenu
       INPUT_CONTEXT_MINIGAME,  // Playing minigame
       INPUT_CONTEXT_SLEEPING,  // Pet sleeping
       INPUT_CONTEXT_STATS      // Viewing stats
   } input_context_t;
   ```

8. **Context-specific handlers**
   ```c
   typedef void (*button_handler_t)(button_id_t button, button_event_t event);

   void input_set_context(input_context_t context);
   void input_register_handler(input_context_t context, button_handler_t handler);
   ```

### Phase 4: Integration with Game

9. **Update main.c**
   - Replace ISR flag checks with event-based handling
   - Register handlers for each context
   - Integrate with menu and game state

10. **Game context handlers**

    ```c
    // Main game view
    static void game_button_handler(button_id_t btn, button_event_t evt) {
        if (btn == BUTTON_RIGHT && evt == BUTTON_EVENT_SHORT_PRESS) {
            menu_activate();
            input_set_context(INPUT_CONTEXT_MENU);
        }
        // Left button could show quick stats
    }

    // Menu active
    static void menu_button_handler(button_id_t btn, button_event_t evt) {
        if (btn == BUTTON_LEFT && evt == BUTTON_EVENT_SHORT_PRESS) {
            menu_scroll_left();
        } else if (btn == BUTTON_RIGHT && evt == BUTTON_EVENT_SHORT_PRESS) {
            menu_scroll_right();
        } else if (btn == BUTTON_RIGHT && evt == BUTTON_EVENT_LONG_PRESS) {
            menu_select_current();
        } else if (btn == BUTTON_LEFT && evt == BUTTON_EVENT_LONG_PRESS) {
            menu_deactivate();
            input_set_context(INPUT_CONTEXT_GAME);
        }
    }
    ```

### Phase 5: Visual Feedback

11. **Button press indicators**
    - Brief flash/highlight on button press
    - Progress indicator for long press buildup
    - Haptic-like visual feedback (screen flash)

12. **Long press progress**
    ```c
    // Show progress bar as user holds button
    void render_long_press_progress(uint32_t current_ms, uint32_t target_ms) {
        float progress = (float)current_ms / target_ms;
        // Draw small progress arc or bar near menu
    }
    ```

## Files to Create/Modify

| File | Action | Description |
|------|--------|-------------|
| `firmware/components/input/input.h` | Create | Input public API |
| `firmware/components/input/input.c` | Create | Button state machine |
| `firmware/components/input/CMakeLists.txt` | Create | Component registration |
| `firmware/main/config.h` | Modify | Button timing constants |
| `firmware/main/main.c` | Modify | Replace ISR flags with input system |
| `firmware/components/game/game.c` | Modify | Add button handlers per context |
| `firmware/components/menu/menu.c` | Modify | Add menu button handling |

## Button Timing Rationale

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Debounce | 50ms | Standard debounce, prevents double triggers |
| Long press | 800ms | Fast enough for quick actions, long enough to avoid accidents |
| Repeat delay | 500ms | Time before auto-repeat starts |
| Repeat rate | 150ms | ~6 repeats/second for scrolling |

## State Transition Diagram

```
                    ┌────────────────────────┐
                    │                        │
                    ▼                        │
              ┌──────────┐                   │
              │   GAME   │                   │
              │  (pet)   │                   │
              └────┬─────┘                   │
                   │                         │
        Right Short│                         │Left Long
                   ▼                         │
              ┌──────────┐                   │
              │   MENU   │◄──────────────────┤
              │ (bottom) │                   │
              └────┬─────┘                   │
                   │                         │
       Right Long  │                         │
                   ▼                         │
              ┌──────────┐                   │
              │ SUBMENU  │───────────────────┘
              │ (action) │     Left Long (back)
              └──────────┘
```

## Memory Impact

| Component | Flash | RAM |
|-----------|-------|-----|
| Input manager | ~2KB | ~64B |
| Handler table | ~512B | ~128B |
| Event queue | 0 | ~32B |

## Verification Tests

| Test ID | Description | Pass Criteria |
|---------|-------------|---------------|
| VT-032 | Short press detection | Press <2000ms triggers short event |
| VT-033 | Long press detection | Press >2000ms triggers long event |
| VT-034 | Left button down/back | Left navigates down, long press = back |
| VT-035 | Right button up/confirm | Right navigates up, long press = confirm |
| Manual | Menu navigation | Can navigate full menu using buttons |
| Manual | Debounce | No double-triggers on button press |

## Status

- [x] Phase 1: Button event system (existing input.c already supports this)
- [x] Phase 2: Button state machine (existing input.c has debounce, long press)
- [x] Phase 3: Context-aware handling (completed in game.c 2026-01-26)
- [x] Phase 4: Game integration (completed 2026-01-26)
- [ ] Phase 5: Visual feedback (deferred - not critical)

## Implementation Notes

**Completed 2026-01-26:**
- Leveraged existing `input.c` which already has:
  - 50ms debounce
  - 2000ms long press threshold (matches REQ-SW-043 spec)
  - BUTTON_EVENT_CLICK for short press
  - BUTTON_EVENT_LONG_PRESS for 2s hold
- Updated `game.c` with new button handling:
  - MAIN state: Right short/long → enter menu
  - MENU state: Left short=down, Right short=up
  - MENU state: Right long=confirm, Left long=back
  - FEED state: Same pattern as menu
  - All submenus support Left long=back
- Long press timing at 2000ms as specified in requirements

## Design Decisions

1. **800ms long press** - Reduced from 2000ms for better responsiveness
2. **Event-based** - Cleaner than polling, enables proper state management
3. **Context system** - Allows different button behaviors per game state
4. **No auto-repeat in menus** - Explicit presses for menu navigation to avoid overshooting

## Risks

1. **Debounce timing** - May need adjustment based on actual hardware
2. **Long press feel** - 800ms might feel too long/short to users
3. **ISR interaction** - Must ensure thread-safe event passing from ISR to main loop
