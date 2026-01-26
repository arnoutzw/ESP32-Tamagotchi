# Plan: REQ-SW-037 Build System Implementation

## Overview

Standardize the build system with ESP-IDF as a git submodule and YAML-based configuration for compile-time secrets.

## Priority & Rationale

- **Priority**: Critical
- **Why this priority**: A reproducible build system is essential for consistent development and deployment. Without embedded ESP-IDF, builds depend on external environment configuration.
- **Dependencies**: None (foundational)
- **Partial Implementation**: YAML config generation already exists (`scripts/generate_config.py`)

## Current State

Already implemented:
- ✅ `config/secrets.yaml` - YAML configuration file for secrets
- ✅ `scripts/generate_config.py` - Python script to generate C header from YAML
- ✅ `firmware/main/config_secrets.h` - Generated header (should be gitignored)
- ✅ WiFi and OTA components use generated config

Not yet implemented:
- ❌ ESP-IDF as git submodule
- ❌ `.gitignore` update for generated files
- ❌ Build documentation in README

## Implementation Steps

### Phase 1: Git Submodule Setup

1. **Add ESP-IDF as git submodule**
   ```bash
   cd ESP32_tamagotchi
   git submodule add -b v5.2 https://github.com/espressif/esp-idf.git esp-idf
   git submodule update --init --recursive
   ```

2. **Create setup script** (`scripts/setup.sh`)
   ```bash
   #!/bin/bash
   # Initialize submodules
   git submodule update --init --recursive

   # Install ESP-IDF tools
   cd esp-idf
   ./install.sh esp32

   # Generate config
   cd ..
   python scripts/generate_config.py
   ```

### Phase 2: Configuration Updates

3. **Update `.gitignore`**
   ```
   # Generated config
   firmware/main/config_secrets.h

   # ESP-IDF build artifacts
   firmware/build/
   firmware/sdkconfig
   firmware/sdkconfig.old

   # Python
   __pycache__/
   *.pyc
   ```

4. **Create `config/secrets.yaml.example`**
   - Template file for users to copy
   - Contains placeholder values
   - Committed to repo (unlike actual secrets.yaml)

### Phase 3: Build Script

5. **Create build wrapper script** (`scripts/build.sh`)
   ```bash
   #!/bin/bash
   # Source ESP-IDF environment
   source esp-idf/export.sh

   # Generate config from YAML
   python scripts/generate_config.py

   # Build firmware
   cd firmware
   idf.py build
   ```

### Phase 4: Documentation

6. **Update README.md** with build instructions
   - Prerequisites
   - One-time setup (submodule init, ESP-IDF tools)
   - Build commands
   - Configuration via secrets.yaml

### Phase 5: Verification

7. **Test clean build**
   ```bash
   # Clone fresh
   git clone --recursive <repo>
   cd ESP32_tamagotchi

   # Setup
   ./scripts/setup.sh

   # Configure secrets
   cp config/secrets.yaml.example config/secrets.yaml
   # Edit secrets.yaml with actual values

   # Build
   ./scripts/build.sh
   ```

## Files to Create/Modify

| File | Action | Description |
|------|--------|-------------|
| `esp-idf/` | Create (submodule) | ESP-IDF v5.2 as git submodule |
| `.gitmodules` | Create | Git submodule configuration |
| `.gitignore` | Modify | Add generated files and build artifacts |
| `config/secrets.yaml.example` | Create | Template configuration file |
| `scripts/setup.sh` | Create | One-time setup script |
| `scripts/build.sh` | Create | Build wrapper script |
| `README.md` | Modify | Add build instructions |

## Memory/Storage Impact

| Component | Size |
|-----------|------|
| ESP-IDF submodule | ~500MB (after install) |
| Additional scripts | ~2KB |

Note: ESP-IDF is large but necessary for reproducible builds.

## Security Considerations

1. **secrets.yaml** must never be committed (in .gitignore)
2. **secrets.yaml.example** contains only placeholder values
3. **config_secrets.h** is generated and gitignored

## Verification Tests

| Test ID | Description | Pass Criteria |
|---------|-------------|---------------|
| VT-014 | Submodule init | `git submodule update --init --recursive` succeeds |
| VT-015 | Config generation | `python scripts/generate_config.py` creates valid header |
| VT-016 | Build success | `idf.py build` completes without errors |

## Estimated Complexity

- **Submodule setup**: Low (git commands)
- **Scripts**: Low (bash wrappers)
- **Documentation**: Low (README updates)
- **Testing**: Medium (requires clean environment)

## Status

- [x] Phase 1: Git submodule setup (completed 2026-01-26)
- [x] Phase 2: Configuration updates (completed 2026-01-26)
- [x] Phase 3: Build script (completed 2026-01-26)
- [x] Phase 4: Documentation (completed 2026-01-26)
- [ ] Phase 5: Verification (pending hardware test)

## Implementation Notes

**Implementation completed** (2026-01-26):
- ESP-IDF v5.2 added as git submodule at `esp-idf/`
- `scripts/setup.sh` - One-time setup script (submodules, ESP-IDF tools, config)
- `scripts/build.sh` - Build wrapper with clean/flash/monitor options
- `scripts/flash.sh` - Quick flash script
- `config/secrets.yaml.example` - Template for secrets
- `.gitignore` updated to exclude `config/secrets.yaml` and `config_secrets.h`
- README.md updated with complete build instructions
