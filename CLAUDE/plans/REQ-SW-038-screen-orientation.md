# Plan: REQ-SW-038 Screen Orientation Implementation

## Overview

Change display orientation from landscape (240x135) to portrait (135x240) mode for a more traditional Tamagotchi-style vertical layout.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: Screen orientation affects ALL display code - sprites, UI, menus, coordinates. This is a foundational change that must be completed before other UI work.
- **Dependencies**: None (foundational)
- **Blocks**: REQ-SW-039 (GUI Location), REQ-SW-043 (Button Functions)

## Current State

- Display currently in landscape mode: 240 wide x 135 tall
- ST7789 panel is actually 240x320, with a 135x240 window
- Current offsets: LCD_COL_OFFSET=52, LCD_ROW_OFFSET=40

## Target State

- Display in portrait mode: 135 wide x 240 tall
- Coordinate (0,0) at top-left corner
- All sprites and UI designed for vertical layout

## Implementation Steps

### Phase 1: Display Driver Changes

1. **Update display initialization** in `display.c`
   - Change MADCTL (Memory Access Control) register for portrait orientation
   - Swap width/height in initialization
   - Update column/row offsets for portrait window

   ```c
   // Portrait mode MADCTL value
   // MY=0, MX=1, MV=0, ML=0, RGB=0, MH=0
   #define MADCTL_PORTRAIT  0x40  // Or appropriate value for your panel
   ```

2. **Update config.h dimensions**
   ```c
   #define LCD_WIDTH           135  // Was 240
   #define LCD_HEIGHT          240  // Was 135
   #define LCD_COL_OFFSET      40   // Recalculate for portrait
   #define LCD_ROW_OFFSET      52   // Recalculate for portrait
   ```

3. **Update display API** (if needed)
   - Ensure `display_fill()`, `display_draw_rect()`, etc. work with new dimensions
   - Verify coordinate system is (0,0) at top-left

### Phase 2: Sprite Adjustments

4. **Audit all sprites** in `sprites.c/sprites.h`
   - Identify sprites that assume landscape layout
   - Determine if sprites need rotation or redesign
   - Update sprite positioning for centered vertical display

5. **Update sprite rendering positions**
   - Pet sprite: center at approximately (67, 96) instead of (120, 67)
   - Status icons: reposition from top-horizontal to top-vertical or side
   - Menu icons: reposition for bottom 20% in portrait

### Phase 3: Game/UI Coordinate Updates

6. **Update game.c rendering**
   - Modify all draw calls to use new coordinate system
   - Update any hardcoded positions

7. **Update menu.c positioning**
   - Menu now at bottom 48 pixels (was designed for landscape)
   - Adjust menu item layout for narrower width

### Phase 4: Testing

8. **Visual verification**
   - All sprites render correctly
   - No clipping at edges
   - Text readable and properly positioned
   - Animations play without artifacts

## Files to Modify

| File | Changes |
|------|---------|
| `firmware/main/config.h` | LCD_WIDTH, LCD_HEIGHT, offsets |
| `firmware/components/display/display.c` | MADCTL register, init sequence |
| `firmware/components/display/include/display.h` | Any dimension macros |
| `firmware/components/game/game.c` | Rendering coordinates |
| `firmware/components/sprites/sprites.c` | Sprite positions |
| `firmware/components/game/menu.c` | Menu layout |

## Technical Details

### ST7789 MADCTL Register (0x36)

| Bit | Name | Function |
|-----|------|----------|
| D7 | MY | Row address order |
| D6 | MX | Column address order |
| D5 | MV | Row/Column exchange |
| D4 | ML | Vertical refresh order |
| D3 | RGB | RGB/BGR order |
| D2 | MH | Horizontal refresh order |

For portrait mode on TTGO T-Display, typically:
- `MV=0` (no row/column swap)
- May need `MX=1` or `MY=1` depending on desired orientation

### Coordinate Mapping

| Orientation | (0,0) | Width | Height |
|-------------|-------|-------|--------|
| Landscape (current) | Top-left | 240 | 135 |
| Portrait (target) | Top-left | 135 | 240 |

## Memory Impact

No additional memory required - just coordinate transformations.

## Verification Tests

| Test ID | Description | Pass Criteria |
|---------|-------------|---------------|
| VT-029 | Portrait orientation | Display shows 135x240 layout |
| Manual | Sprite centering | Pet centered horizontally |
| Manual | Edge clipping | No sprites cut off at edges |
| Manual | Text readability | All text properly oriented |

## Status

- [ ] Phase 1: Display driver changes
- [ ] Phase 2: Sprite adjustments
- [ ] Phase 3: Game/UI coordinate updates
- [ ] Phase 4: Testing

## Implementation Notes

*To be updated during implementation*

## Risks

1. **Panel variation**: Different TTGO T-Display batches may have different panel orientations
2. **Sprite redesign**: Some sprites may look wrong when simply repositioned
3. **Touch coordinates**: If touch is ever added, coordinates must also be rotated
