#!/bin/bash
# ESP32 Tamagotchi - Flash Script
#
# Quick flash script for development.
#
# Usage: ./scripts/flash.sh [port]
#   port - Serial port (default: auto-detect)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

PORT="${1:-}"

# Source ESP-IDF environment
source "$PROJECT_ROOT/esp-idf/export.sh" > /dev/null 2>&1

cd "$PROJECT_ROOT/firmware"

if [ -n "$PORT" ]; then
    idf.py -p "$PORT" flash monitor
else
    idf.py flash monitor
fi
