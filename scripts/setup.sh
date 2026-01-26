#!/bin/bash
# ESP32 Tamagotchi - Development Environment Setup
#
# This script sets up the development environment by:
# 1. Initializing git submodules (ESP-IDF)
# 2. Installing ESP-IDF tools
# 3. Generating config from secrets.yaml
#
# Usage: ./scripts/setup.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== ESP32 Tamagotchi Setup ==="
echo "Project root: $PROJECT_ROOT"
echo ""

# Step 1: Initialize submodules
echo "Step 1: Initializing git submodules..."
cd "$PROJECT_ROOT"
git submodule update --init --recursive
echo "Submodules initialized."
echo ""

# Step 2: Install ESP-IDF tools
echo "Step 2: Installing ESP-IDF tools..."
cd "$PROJECT_ROOT/esp-idf"
./install.sh esp32
echo "ESP-IDF tools installed."
echo ""

# Step 3: Generate config
echo "Step 3: Generating config..."
cd "$PROJECT_ROOT"

if [ ! -f "config/secrets.yaml" ]; then
    echo "Creating config/secrets.yaml from template..."
    cp config/secrets.yaml.example config/secrets.yaml
    echo "NOTE: Edit config/secrets.yaml with your WiFi credentials!"
fi

python3 scripts/generate_config.py
echo ""

echo "=== Setup Complete ==="
echo ""
echo "To build the firmware:"
echo "  ./scripts/build.sh"
echo ""
echo "Or manually:"
echo "  source esp-idf/export.sh"
echo "  cd firmware && idf.py build"
