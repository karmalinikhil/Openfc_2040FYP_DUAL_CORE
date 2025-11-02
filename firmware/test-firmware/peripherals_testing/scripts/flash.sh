#!/bin/bash

# OpenFC2040 Flash Script
# Simple script to flash the firmware to the board

set -e  # Exit on any error

echo "========================================="
echo "  OpenFC2040 Firmware Flasher"
echo "========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if firmware file exists
FIRMWARE_FILE="build/openfc2040_firmware.uf2"
if [ ! -f "$FIRMWARE_FILE" ]; then
    echo -e "${RED}[ERROR]${NC} Firmware file not found: $FIRMWARE_FILE"
    echo "Please build the firmware first using: ./scripts/build.sh"
    exit 1
fi

# Check for RPI-RP2 mount point
RPI_MOUNT="/media/$USER/RPI-RP2"
if [ ! -d "$RPI_MOUNT" ]; then
    echo -e "${YELLOW}[WARNING]${NC} RPI-RP2 drive not found at $RPI_MOUNT"
    echo ""
    echo "To flash the firmware:"
    echo "1. Hold BOOTSEL button on the RP2040 board"
    echo "2. Connect USB cable (while holding BOOTSEL)"
    echo "3. Release BOOTSEL - the board should appear as RPI-RP2 drive"
    echo "4. Run this script again"
    echo ""
    echo "Alternative manual flash:"
    echo "cp $FIRMWARE_FILE /path/to/RPI-RP2/"
    exit 1
fi

# Flash the firmware
echo -e "${BLUE}[INFO]${NC} Flashing firmware to $RPI_MOUNT..."
cp "$FIRMWARE_FILE" "$RPI_MOUNT/"

# Wait a moment for the copy to complete
sleep 2

echo -e "${GREEN}[SUCCESS]${NC} Firmware flashed successfully!"
echo ""
echo "The board should now reboot with the new firmware."
echo "Connect to the serial console to see the output."
