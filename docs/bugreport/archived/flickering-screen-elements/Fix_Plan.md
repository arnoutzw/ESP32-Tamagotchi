# Fix Plan: Flickering Screen Elements

## Root Cause Summary

The flickering is caused by **lack of double buffering** - the display is updated directly, causing visible tearing as the old content is erased before new content is drawn.

## Solution: Implement Double Buffering with Dirty Region Tracking

### Approach

Implement a **partial framebuffer** strategy to balance memory usage with flicker elimination:

1. **Add a screen buffer in RAM** - For 240x135 @ 16-bit color = 64,800 bytes (~63KB)
   - This fits in ESP32's 520KB SRAM but is significant
   - Alternative: Use **dirty rectangle** tracking to only update changed regions

2. **Recommended approach: Dirty Region + Selective Redraw**
   - Track which screen regions changed
   - Only send changed pixels to the display
   - Reduces both flickering and SPI traffic

### Implementation Steps

#### Phase 1: Add Framebuffer (Eliminates Flickering)

**File: `firmware/components/display/display.c`**

1. Add a static framebuffer:
```c
static uint16_t s_framebuffer[LCD_WIDTH * LCD_HEIGHT];
static bool s_buffer_dirty = true;
```

2. Modify all drawing functions to write to framebuffer instead of LCD:
   - `display_fill_rect()` → writes to `s_framebuffer`
   - `display_draw_pixel()` → writes to `s_framebuffer`
   - etc.

3. Implement `display_end_frame()` to flush framebuffer to LCD:
```c
void display_end_frame(void)
{
    if (!s_buffer_dirty) return;

    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    // Send entire buffer in large chunks for efficiency
    gpio_set_level(LCD_PIN_DC, 1);
    // ... send s_framebuffer via DMA

    s_buffer_dirty = false;
}
```

4. Update `game_render()` in main.c to use frame boundaries:
```c
void game_render(void)
{
    display_start_frame();  // Optional: clear dirty flag

    // ... existing render code ...

    display_end_frame();    // Flush to LCD
}
```

#### Phase 2: Optimize Sprite Drawing (Reduces Render Time)

**File: `firmware/components/display/display.c`**

1. Replace pixel-by-pixel sprite drawing with batch drawing:
```c
void display_draw_sprite(int16_t x, int16_t y, int16_t w, int16_t h,
                         const uint16_t *data, uint16_t transparent)
{
    // Write directly to framebuffer, skipping transparent pixels
    for (int16_t j = 0; j < h; j++) {
        int16_t screen_y = y + j;
        if (screen_y < 0 || screen_y >= LCD_HEIGHT) continue;

        for (int16_t i = 0; i < w; i++) {
            int16_t screen_x = x + i;
            if (screen_x < 0 || screen_x >= LCD_WIDTH) continue;

            uint16_t pixel = data[j * w + i];
            if (pixel != transparent) {
                s_framebuffer[screen_y * LCD_WIDTH + screen_x] = pixel;
            }
        }
    }
    s_buffer_dirty = true;
}
```

#### Phase 3: Memory Optimization (If RAM is Tight)

If 63KB is too much, implement **partial buffering**:

1. Buffer only the menu area (220x60 = 26,400 bytes = ~26KB)
2. Use dirty rectangle tracking
3. Only redraw changed regions

### Alternative Quick Fix (Minimal Changes)

If full framebuffer is not desired, reduce flickering with these changes:

1. **Don't redraw background in menu state**:
```c
static void render_menu(void)
{
    // Remove: render_main();  // This causes the flicker!

    // Only redraw menu panel and items
    // Keep background static
}
```

2. **Add a "menu background" that persists**:
   - Draw main screen once when entering menu
   - Only update the menu panel area each frame

### Files to Modify

| File | Changes |
|------|---------|
| `firmware/components/display/display.c` | Add framebuffer, modify drawing functions |
| `firmware/components/display/include/display.h` | Add `display_start_frame()`, `display_end_frame()` declarations |
| `firmware/components/game/game.c` | Wrap render in frame boundaries, optimize menu redraw |
| `firmware/main/main.c` | No changes needed |

### Memory Impact

| Approach | RAM Usage | Complexity |
|----------|-----------|------------|
| Full framebuffer | +63KB | Low |
| Partial buffer (menu only) | +26KB | Medium |
| Dirty rectangles only | +~1KB | High |
| Quick fix (no buffer) | 0 | Low |

### Recommended Implementation Order

1. **Quick fix first** - Stop calling `render_main()` in menu state (5 min fix)
2. **Then implement framebuffer** if flickering persists elsewhere
3. **Optimize sprite drawing** for overall performance improvement

### Testing

1. Build and flash: `idf.py build && idf.py -p /dev/cu.usbserial-* flash`
2. Navigate to menu and observe:
   - Menu should appear without flashing
   - Animation should be smooth
   - No visible tearing during state transitions
