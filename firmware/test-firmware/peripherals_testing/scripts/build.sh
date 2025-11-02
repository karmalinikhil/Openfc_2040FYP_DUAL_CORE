#!/bin/bash

# OpenFC2040 Build Script
# Simple script to build the firmware

set -e  # Exit on any error

echo "========================================="
echo "  OpenFC2040 Firmware Builder"
echo "========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}[ERROR]${NC} Please run this script from the OPENFC2040 root directory"
    exit 1
fi

# Check if pico-sdk exists
if [ ! -d "pico-sdk" ]; then
    echo -e "${RED}[ERROR]${NC} Pico SDK not found. Please run: git submodule update --init --recursive"
    exit 1
fi

# Initialize submodules if needed
if [ ! -f "pico-sdk/pico_sdk_init.cmake" ]; then
    echo -e "${YELLOW}[INFO]${NC} Initializing Pico SDK submodules..."
    git submodule update --init --recursive
fi

# Create build directory
echo -e "${BLUE}[INFO]${NC} Setting up build directory..."
mkdir -p build
cd build

# Configure with CMake
echo -e "${BLUE}[INFO]${NC} Configuring build with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the firmware
echo -e "${BLUE}[INFO]${NC} Building firmware..."
make -j$(nproc)

# Check if build was successful
if [ -f "openfc2040_firmware.uf2" ]; then
    echo -e "${GREEN}[SUCCESS]${NC} Build completed successfully!"
    echo ""
    echo "Firmware files generated:"
    ls -la openfc2040_firmware.*
    echo ""
    echo -e "${YELLOW}[FLASH]${NC} To flash the firmware:"
    echo "1. Hold BOOTSEL button on RP2040 and connect USB"
    echo "2. Copy openfc2040_firmware.uf2 to the RPI-RP2 drive"
    echo "   Or run: cp openfc2040_firmware.uf2 /media/\$USER/RPI-RP2/"
else
    echo -e "${RED}[ERROR]${NC} Build failed!"
    exit 1
fi
