# Plan: REQ-SW-041 WiFi Features Implementation

## Overview

Implement network-based features including NTP time synchronization, web dashboard for pet stats, and foundation for multiplayer pet visits.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: Accurate time tracking is essential for proper pet aging and stat decay. The web dashboard provides user value and debugging capability. These features leverage the existing WiFi infrastructure (REQ-SW-036).
- **Dependencies**:
  - REQ-SW-036 (WiFi Connectivity) - Already implemented
  - REQ-SW-001 (Pet State System) - For stats to display

## Implementation Steps

### Phase 1: NTP Time Synchronization

1. **Create NTP component** (`firmware/components/ntp/`)
   - `ntp_manager.h` - Public API
   - `ntp_manager.c` - Implementation

2. **NTP Manager API**
   ```c
   esp_err_t ntp_manager_init(void);
   esp_err_t ntp_manager_sync(void);
   bool ntp_manager_is_synced(void);
   time_t ntp_manager_get_time(void);
   void ntp_manager_get_time_str(char *buf, size_t len);
   ```

3. **Configuration**
   - NTP server: `pool.ntp.org` (default)
   - Timezone: configurable in secrets.yaml
   - Sync interval: every 1 hour when connected
   - Fallback: use millis-based time if no sync

4. **Integration with pet aging**
   - Update `pet_state.c` to use real time for age calculation
   - Store last known good time in NVS for offline use

### Phase 2: Web Dashboard

5. **Add dashboard HTTP handlers** to existing web server
   - `GET /` - Dashboard HTML page
   - `GET /api/status` - JSON pet stats
   - `GET /api/stats/history` - Historical data (optional)

6. **Dashboard page features**
   - Pet name and age
   - Current stats (hunger, happiness, health, energy)
   - Battery level
   - WiFi status (STA/AP mode, IP address)
   - Firmware version
   - Real-time updates via JavaScript polling

7. **JSON status endpoint**
   ```json
   {
     "pet": {
       "name": "Splash",
       "age_days": 5,
       "life_stage": "child",
       "hunger": 75,
       "happiness": 60,
       "health": 85,
       "energy": 45,
       "weight": 25,
       "is_sleeping": false,
       "is_sick": false
     },
     "system": {
       "battery_voltage": 3.85,
       "battery_soc": 72,
       "wifi_mode": "STA",
       "ip_address": "192.168.1.100",
       "firmware_version": "1.0.0",
       "uptime_seconds": 3600
     }
   }
   ```

### Phase 3: Multiplayer Foundation (Future)

8. **Pet sharing protocol** (design only for now)
   - REST API for pet data exchange
   - Discovery via mDNS
   - Visit animations and interaction effects

## Files to Create/Modify

| File | Action | Description |
|------|--------|-------------|
| `firmware/components/ntp/ntp_manager.h` | Create | NTP public API |
| `firmware/components/ntp/ntp_manager.c` | Create | NTP implementation |
| `firmware/components/ntp/CMakeLists.txt` | Create | Component registration |
| `firmware/components/wifi/wifi_manager.c` | Modify | Add NTP init after STA connect |
| `firmware/main/main.c` | Modify | Initialize NTP, add dashboard handlers |
| `firmware/main/CMakeLists.txt` | Modify | Add ntp dependency |
| `config/secrets.yaml.example` | Modify | Add timezone config |
| `scripts/generate_config.py` | Modify | Generate timezone config |

## Memory Impact

| Component | Flash | RAM |
|-----------|-------|-----|
| NTP client | ~5KB | ~1KB |
| Dashboard HTML | ~3KB | 0 (served from flash) |
| JSON builder | ~2KB | ~512B |

## Security Considerations

1. Dashboard is read-only (no control actions via web)
2. Status API does not expose secrets
3. Rate limiting on API endpoints (future)

## Verification Tests

| Test ID | Description | Pass Criteria |
|---------|-------------|---------------|
| VT-020 | NTP sync | Time syncs within 5 seconds of connecting to WiFi |
| VT-021 | Time persistence | Time maintained across short power cycles |
| VT-022 | Dashboard access | Dashboard loads at http://device-ip/ |
| VT-023 | Status API | GET /api/status returns valid JSON |

## Status

- [ ] Phase 1: NTP Time Synchronization
- [ ] Phase 2: Web Dashboard
- [ ] Phase 3: Multiplayer Foundation (design only)

## Implementation Notes

*To be updated during implementation*
