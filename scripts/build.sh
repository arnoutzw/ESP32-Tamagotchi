#!/bin/bash
# ESP32 Tamagotchi - Build Script
#
# This script builds the firmware by:
# 1. Sourcing ESP-IDF environment
# 2. Generating config from secrets.yaml
# 3. Building with idf.py
#
# Usage: ./scripts/build.sh [clean|flash|monitor]
#   clean   - Clean build directory before building
#   flash   - Flash firmware after building
#   monitor - Open serial monitor after flashing

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Parse arguments
CLEAN=false
FLASH=false
MONITOR=false
PORT=""

for arg in "$@"; do
    case $arg in
        clean)
            CLEAN=true
            ;;
        flash)
            FLASH=true
            ;;
        monitor)
            MONITOR=true
            ;;
        -p=*|--port=*)
            PORT="${arg#*=}"
            ;;
    esac
done

echo "=== ESP32 Tamagotchi Build ==="

# Check if ESP-IDF submodule exists
if [ ! -f "$PROJECT_ROOT/esp-idf/export.sh" ]; then
    echo "Error: ESP-IDF not found. Run ./scripts/setup.sh first."
    exit 1
fi

# Source ESP-IDF environment
echo "Sourcing ESP-IDF environment..."
source "$PROJECT_ROOT/esp-idf/export.sh"

# Generate config from YAML
echo "Generating config from secrets.yaml..."
python3 "$PROJECT_ROOT/scripts/generate_config.py"

# Change to firmware directory
cd "$PROJECT_ROOT/firmware"

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    idf.py fullclean
fi

# Build
echo "Building firmware..."
idf.py build

# Flash if requested
if [ "$FLASH" = true ]; then
    echo "Flashing firmware..."
    if [ -n "$PORT" ]; then
        idf.py -p "$PORT" flash
    else
        idf.py flash
    fi
fi

# Monitor if requested
if [ "$MONITOR" = true ]; then
    echo "Opening serial monitor..."
    if [ -n "$PORT" ]; then
        idf.py -p "$PORT" monitor
    else
        idf.py monitor
    fi
fi

echo ""
echo "=== Build Complete ==="
echo "Binary: firmware/build/esp32-tamagotchi.bin"
