# Plan: REQ-SW-039 GUI Location Implementation

## Overview

Implement a bottom-positioned scrollable menu system taking up approximately 20% of the screen height in portrait mode.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: Core UI element that affects all user interaction. Must be designed after screen orientation change.
- **Dependencies**: REQ-SW-038 (Screen Orientation) - must be in portrait mode first
- **Blocks**: REQ-SW-043 (Button Functions)

## Current State

- No dedicated menu area defined
- UI scattered across screen
- Portrait mode: 135 wide x 240 tall (after REQ-SW-038)

## Target State

- Bottom 20% of screen (~48 pixels) dedicated to menu
- Scrollable menu with icon-based actions
- Clear visual separation between pet area and menu
- Menu icons: Feed, Play, Clean, Sleep, Stats, Medicine

## Screen Layout (Portrait 135x240)

```
┌─────────────────┐  0
│                 │
│   Status Icons  │  20-30
│   (top bar)     │
│                 │
│─────────────────│  40
│                 │
│                 │
│   Pet Display   │
│   Area          │
│   (main game)   │
│                 │
│                 │
│─────────────────│  192 (240 - 48)
│   Menu Bar      │
│   (scrollable)  │
└─────────────────┘  240
```

## Implementation Steps

### Phase 1: Menu Data Structures

1. **Create menu component** (`firmware/components/menu/`)
   - `menu.h` - Public API
   - `menu.c` - Implementation
   - `menu_icons.h` - Icon sprite data

2. **Menu Item Structure**
   ```c
   typedef enum {
       MENU_ITEM_FEED,
       MENU_ITEM_PLAY,
       MENU_ITEM_CLEAN,
       MENU_ITEM_SLEEP,
       MENU_ITEM_STATS,
       MENU_ITEM_MEDICINE,
       MENU_ITEM_COUNT
   } menu_item_t;

   typedef struct {
       menu_item_t current_item;
       uint8_t scroll_offset;      // For horizontal scrolling
       uint8_t visible_count;      // Icons visible at once
       bool is_active;             // Menu currently selected
   } menu_state_t;
   ```

3. **Menu API**
   ```c
   esp_err_t menu_init(void);
   void menu_render(void);
   void menu_scroll_left(void);
   void menu_scroll_right(void);
   menu_item_t menu_get_selected(void);
   void menu_select_current(void);
   void menu_activate(void);
   void menu_deactivate(void);
   bool menu_is_active(void);
   ```

### Phase 2: Menu Rendering

4. **Menu bar background**
   - Draw separator line at y=192
   - Fill menu area with distinct background color
   - Consider gradient or texture for visual appeal

5. **Icon rendering**
   - Each icon: 32x32 pixels (fits 4 icons with spacing)
   - Visible icons: 3-4 at a time with scroll indicators
   - Selected icon: highlighted border or glow effect
   - Icon spacing: ~8 pixels between icons

6. **Scroll indicators**
   - Left arrow when more items to left
   - Right arrow when more items to right
   - Arrows at y=208 (centered in menu area)

### Phase 3: Menu Icons (32x32 sprites)

7. **Create menu icons**
   | Icon | Visual | Description |
   |------|--------|-------------|
   | Feed | Fish/Food | Open feeding submenu |
   | Play | Ball/Game | Start play minigame |
   | Clean | Broom/Water | Clean up poop |
   | Sleep | Moon/Zzz | Put pet to sleep |
   | Stats | Chart/Heart | Show pet statistics |
   | Medicine | Pill/Cross | Give medicine if sick |

8. **Icon states**
   - Normal: standard colors
   - Selected: highlighted border
   - Disabled: grayed out (e.g., medicine when not sick)
   - Active: pressed animation

### Phase 4: Integration

9. **Update game.c**
   - Define screen regions:
     ```c
     #define REGION_STATUS_Y     0
     #define REGION_STATUS_H     40
     #define REGION_GAME_Y       40
     #define REGION_GAME_H       152  // 192 - 40
     #define REGION_MENU_Y       192
     #define REGION_MENU_H       48
     ```
   - Call `menu_render()` after pet rendering
   - Handle menu activation/deactivation

10. **Update main game loop**
    - Integrate menu state with button handling
    - Connect menu selections to game actions

## Files to Create/Modify

| File | Action | Description |
|------|--------|-------------|
| `firmware/components/menu/menu.h` | Create | Menu public API |
| `firmware/components/menu/menu.c` | Create | Menu implementation |
| `firmware/components/menu/menu_icons.h` | Create | Icon sprite data |
| `firmware/components/menu/CMakeLists.txt` | Create | Component registration |
| `firmware/main/config.h` | Modify | Add screen region defines |
| `firmware/components/game/game.c` | Modify | Integrate menu rendering |
| `firmware/main/main.c` | Modify | Initialize menu, handle input |

## Menu Navigation Flow

```
Normal Mode (pet visible)
    │
    ├─ [Right Button Short] → Activate Menu
    │
    └─ Menu Active Mode
         │
         ├─ [Left Button Short] → Scroll Left
         ├─ [Right Button Short] → Scroll Right
         ├─ [Right Button Long] → Select Item
         └─ [Left Button Long] → Deactivate Menu (back)
```

## Memory Impact

| Component | Flash | RAM |
|-----------|-------|-----|
| Menu logic | ~3KB | ~256B |
| 6 icons (32x32 RGB565) | ~12KB | 0 (const) |
| Menu state | 0 | ~16B |

## Visual Design Notes

1. **Color scheme** - Match ocean theme
   - Menu background: darker blue (#2B4D or similar)
   - Selected item: bright cyan border
   - Icons: colorful, easy to distinguish

2. **Animation considerations**
   - Smooth scroll animation (optional, may skip for simplicity)
   - Selection pulse/glow effect
   - Item press feedback

## Verification Tests

| Test ID | Description | Pass Criteria |
|---------|-------------|---------------|
| VT-030 | Menu location | Menu renders in bottom 20% |
| VT-031 | Menu scrolling | Can scroll through all 6 items |
| Manual | Icon visibility | All icons clearly visible |
| Manual | Selection indication | Selected item clearly highlighted |

## Status

- [x] Phase 1: Menu data structures (completed 2026-01-26)
- [x] Phase 2: Menu rendering (completed 2026-01-26)
- [x] Phase 3: Menu icons (using text labels, completed 2026-01-26)
- [x] Phase 4: Integration (completed 2026-01-26)

## Implementation Notes

**Completed 2026-01-26:**
- Implemented in `game.c` rather than separate menu component for simplicity
- Menu bar height: 48 pixels (20% of 240)
- Menu bar position: Y=192 to Y=240 (bottom of screen)
- Shows 4 menu items at a time with scroll indicators
- Text-based labels instead of icons for clarity on small screen
- Scroll offset auto-adjusts to keep selection visible
- Green highlight for selected item when menu is active
- Button hint shows "R:Menu" when menu not active

## Dependencies on REQ-SW-038

This implementation assumes portrait mode (135x240) is complete:
- Menu height calculation based on 240 pixel height
- Icon sizing for 135 pixel width
- Y coordinates relative to portrait layout

If portrait mode is not yet implemented, all Y coordinates and height calculations must be adjusted.
