# Plan: REQ-SW-034 OTA Updates Implementation

## Overview

Implement Over-The-Air (OTA) firmware update capability with automatic rollback on boot failure. This enables remote firmware updates without physical access to the device.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: OTA is essential for field updates and bug fixes. Without OTA, every firmware update requires physical USB connection, which is impractical for deployed devices.
- **Dependencies**:
  - WiFi connectivity (REQ-SW-041 or basic AP mode)
  - Two-OTA partition table

## Implementation Steps

### Phase 1: Partition Table Configuration

1. **Update partition table** to support two OTA slots
   - Create custom partition table CSV with:
     - `nvs` - NVS storage (existing)
     - `otadata` - OTA data partition
     - `ota_0` - First OTA app slot
     - `ota_1` - Second OTA app slot
   - Update `sdkconfig.defaults` to use custom partition table

2. **Enable OTA rollback** in sdkconfig:
   ```
   CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE=y
   CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK=n
   ```

### Phase 2: WiFi Component

3. **Create WiFi component** (`firmware/components/wifi/`)
   - Implement AP mode for initial setup (fallback)
   - Implement STA mode for normal operation
   - Store WiFi credentials in NVS
   - Auto-reconnect on disconnect

### Phase 3: OTA Component

4. **Create OTA component** (`firmware/components/ota/`)
   - `ota.h` - Public API
   - `ota.c` - Implementation

   Functions to implement:
   ```c
   esp_err_t ota_init(void);
   esp_err_t ota_start_update(const char *url);
   esp_err_t ota_mark_valid(void);  // Call after successful boot
   ota_state_t ota_get_state(void);
   ```

5. **Implement HTTP OTA handler**
   - Listen on `/ota` endpoint
   - Require password authentication (Basic Auth or custom header)
   - Accept firmware binary via POST
   - Show progress on LCD during update
   - Reboot after successful write

6. **Implement rollback logic**
   - On boot, check if this is first boot after OTA
   - Run validation checks (display works, buttons respond)
   - If valid: call `esp_ota_mark_app_valid_cancel_rollback()`
   - If invalid: reboot to trigger automatic rollback

### Phase 4: Integration

7. **Update main.c**
   - Initialize WiFi on startup
   - Initialize OTA component
   - Call `ota_mark_valid()` after successful initialization
   - Add OTA status to display (optional)

8. **Add Settings menu option**
   - Show WiFi status
   - Show current firmware version
   - Manual "Check for updates" option (stretch)

### Phase 5: Testing

9. **Create test scenarios**
   - VT-009: Upload valid firmware → verify update succeeds
   - VT-010: Upload firmware that fails boot → verify rollback occurs

## Files to Modify

| File | Changes |
|------|---------|
| `firmware/partitions.csv` | New file: custom partition table with OTA slots |
| `firmware/sdkconfig.defaults` | Enable OTA partition table and rollback |
| `firmware/CMakeLists.txt` | Reference custom partition table |
| `firmware/components/wifi/wifi.c` | New file: WiFi AP/STA management |
| `firmware/components/wifi/include/wifi.h` | New file: WiFi public API |
| `firmware/components/wifi/CMakeLists.txt` | New file: component registration |
| `firmware/components/ota/ota.c` | New file: OTA update logic |
| `firmware/components/ota/include/ota.h` | New file: OTA public API |
| `firmware/components/ota/CMakeLists.txt` | New file: component registration |
| `firmware/main/main.c` | Initialize WiFi and OTA, mark boot valid |
| `firmware/main/config.h` | Add OTA and WiFi configuration constants |

## Memory Impact

| Component | Flash | RAM |
|-----------|-------|-----|
| OTA partition overhead | ~1.5MB (second app slot) | 0 |
| WiFi stack | ~100KB | ~40KB |
| OTA component | ~10KB | ~2KB |
| HTTP server | ~30KB | ~8KB |

**Note**: The 4MB flash can accommodate two ~1.5MB app images plus NVS and OTA data.

## Partition Table Layout

```csv
# Name,   Type, SubType, Offset,   Size,    Flags
nvs,      data, nvs,     0x9000,   0x6000,
otadata,  data, ota,     0xf000,   0x2000,
ota_0,    app,  ota_0,   0x20000,  0x1C0000,
ota_1,    app,  ota_1,   0x1E0000, 0x1C0000,
```

This gives each OTA slot ~1.8MB, which is sufficient for the Tamagotchi firmware.

## Security Considerations

1. **Password protection**: OTA endpoint requires authentication
2. **HTTPS optional**: Can use HTTP for local network, HTTPS for internet updates
3. **Firmware validation**: ESP-IDF validates firmware signature before boot
4. **Rollback protection**: Prevents bricking from bad firmware

## Verification

1. **Build verification**:
   ```bash
   cd firmware && idf.py build
   # Verify partition table in build output
   # Verify OTA-related symbols in map file
   ```

2. **Flash and test**:
   ```bash
   idf.py -p /dev/cu.usbserial-* flash monitor
   # Connect to device AP or configure STA
   # Upload test firmware via HTTP
   # Verify update and rollback scenarios
   ```

3. **Rollback test**:
   - Build firmware with intentional early crash
   - OTA update with bad firmware
   - Verify device reboots and rolls back to previous version

## Estimated Complexity

- **Partition setup**: Low (configuration only)
- **WiFi component**: Medium (reusable from ESP-IDF examples)
- **OTA component**: Medium (ESP-IDF provides OTA APIs)
- **Integration**: Low (initialization calls)
- **Testing**: Medium (requires multiple firmware versions)

## Status

- [x] Phase 1: Partition table configuration (completed 2026-01-26)
- [x] Phase 2: WiFi component (completed 2026-01-26)
- [x] Phase 3: OTA component (completed 2026-01-26)
- [x] Phase 4: Integration (completed 2026-01-26)
- [ ] Phase 5: Testing (pending hardware verification)

## Implementation Notes

**Build output** (2026-01-26):
- Binary size: 0xdd9c0 bytes (~886KB)
- OTA partition size: 0x1C0000 bytes (~1.8MB)
- Free space in partition: 51% (0xe2640 bytes)
- Bootloader size: 0x61d0 bytes (13% free)

**Files created:**
- `firmware/partitions.csv` - Custom OTA partition table
- `firmware/components/wifi/wifi_manager.c` - WiFi AP/STA management
- `firmware/components/wifi/include/wifi_manager.h` - WiFi API
- `firmware/components/ota/ota_manager.c` - OTA update handler
- `firmware/components/ota/include/ota_manager.h` - OTA API

**Configuration:**
- WiFi AP SSID: "Tamagotchi"
- WiFi AP Password: "dolphin123"
- OTA endpoint: POST /ota (with X-OTA-Password header)
- OTA status: GET /ota/status
- Default OTA password: "tamagotchi"

**To perform OTA update:**
```bash
curl -X POST -H "X-OTA-Password: tamagotchi" \
  --data-binary @build/esp32-tamagotchi.bin \
  http://192.168.4.1/ota
```
