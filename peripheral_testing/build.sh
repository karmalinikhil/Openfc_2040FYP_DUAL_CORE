#!/bin/bash
# Build script for OpenFC2040 peripheral testing firmware

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=== OpenFC2040 Peripheral Test Build ==="

# Check for Pico SDK
if [ -z "$PICO_SDK_PATH" ]; then
    echo "PICO_SDK_PATH not set. Will fetch SDK during build."
    export PICO_SDK_FETCH_FROM_GIT=1
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "Configuring..."
cmake .. -DPICO_SDK_FETCH_FROM_GIT=1

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "=== Build Complete ==="
echo "Firmware: $BUILD_DIR/peripheral_test.uf2"
echo ""
echo "To flash:"
echo "  1. Hold BOOTSEL and connect USB"
echo "  2. Copy peripheral_test.uf2 to RPI-RP2 drive"
