# Root Cause Analysis: Flickering Screen Elements

## Summary

The main menu flickers (appears and disappears) without user interaction, significantly degrading the user experience.

## Symptoms

- Menu panel visibly flickers when displayed
- Background redraws are visible to the user
- No user input triggers the flickering - it happens automatically during normal rendering

## Environment

- Hardware: TTGO T-Display ESP32 (240x135 ST7789 TFT)
- Game loop: 30 FPS (~33ms per frame)
- Display driver: Direct SPI writes to LCD

## Investigation

1. **Analyzed game.c rendering flow**: Found that `render_menu()` calls `render_main()` at line 216, which redraws the entire background before drawing the menu panel on top.

2. **Analyzed display.c driver**: Confirmed no double buffering is implemented. `display_start_frame()` and `display_end_frame()` are empty placeholder functions (lines 472-480).

3. **Analyzed sprite drawing**: `display_draw_sprite()` draws pixel-by-pixel via individual SPI transactions, which is slow and contributes to visible tearing.

4. **Analyzed main.c game loop**: Game renders at 30 FPS, calling `game_render()` every 33ms which triggers full screen redraws.

## Root Cause

**Primary cause**: Lack of double buffering in the display driver.

The display is updated directly via SPI. When `render_menu()` is called:
1. First, `render_main()` clears and redraws the entire background
2. Then the menu panel is drawn on top
3. The user sees the background appear briefly before the menu is drawn over it

This creates visible flickering because the old content is erased before new content is drawn.

**Secondary cause**: Slow pixel-by-pixel sprite drawing amplifies the problem by increasing the time window during which partial updates are visible.

## Contributing Factors

1. **Full screen redraw in menu state**: `render_menu()` unnecessarily calls `render_main()` every frame
2. **No dirty rectangle tracking**: The entire screen is redrawn even when only small portions change
3. **Animation timer**: Periodic animation updates trigger redraws even without user input

## Impact Assessment

- **Severity**: Medium
- **Affected functionality**: Menu display, overall visual quality
- **User impact**: Annoying visual artifact that degrades the experience
- **Risk of recurrence**: Low once framebuffer is implemented

## Resolution

See `Fix_Plan.md` for implementation details. Recommended approach:
1. Quick fix: Stop calling `render_main()` in menu state (immediate improvement)
2. Long-term: Implement framebuffer with dirty region tracking
