#!/bin/bash
#
# OpenFC2040 Flash Script
#
# This script flashes the firmware to the OpenFC2040 board via UF2 bootloader
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PX4_DIR="$PROJECT_ROOT/px4-autopilot"
BOARD_VENDOR="raspberrypi"
BOARD_MODEL="pico"
BOARD_LABEL="default"
BOARD_TARGET="${BOARD_VENDOR}_${BOARD_MODEL}_${BOARD_LABEL}"
UF2_FILE="$PX4_DIR/build/${BOARD_TARGET}/${BOARD_TARGET}.uf2"

# Functions
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

check_uf2_exists() {
    if [ ! -f "$UF2_FILE" ]; then
        print_error "UF2 file not found: $UF2_FILE"
        print_status "Please build the firmware first:"
        print_status "  ./scripts/build.sh"
        exit 1
    fi
    
    local size=$(stat -c%s "$UF2_FILE" 2>/dev/null || stat -f%z "$UF2_FILE" 2>/dev/null)
    local size_kb=$((size / 1024))
    print_status "Found UF2 file: ${size_kb}KB"
}

find_bootloader() {
    # Print to stderr so command substitution doesn't capture status text
    print_status "Looking for RP2040 in bootloader mode..." 1>&2
    
    # Check for RPI-RP2 mount point
    local mount_points=(
        "/media/$USER/RPI-RP2"
        "/media/RPI-RP2"
        "/mnt/RPI-RP2"
        "/Volumes/RPI-RP2"  # macOS
    )
    
    for mount_point in "${mount_points[@]}"; do
        if [ -d "$mount_point" ]; then
            echo "$mount_point"
            return 0
        fi
    done
    
    # Check USB devices
    if lsusb 2>/dev/null | grep -q "2e8a:0003"; then
        print_warning "RP2040 detected in bootloader mode but not mounted" 1>&2
        print_status "Trying to find mount point..." 1>&2
        
        # Try to find it in /proc/mounts or mount output
        local found_mount=$(mount | grep -i "rpi-rp2" | awk '{print $3}' | head -n1)
        if [ -n "$found_mount" ]; then
            echo "$found_mount"
            return 0
        fi
    fi
    
    return 1
}

flash_firmware() {
    print_header "Flashing Firmware to OpenFC2040"
    
    check_uf2_exists
    
    local bootloader_mount=$(find_bootloader)
    
    if [ -n "$bootloader_mount" ]; then
        print_status "RP2040 bootloader detected at: $bootloader_mount"
        print_status "Copying firmware..."
        
        cp "$UF2_FILE" "$bootloader_mount/"
        
        if [ $? -eq 0 ]; then
            print_status "Firmware flashed successfully!"
            print_status "Board will reboot automatically"
            
            # Wait a moment for the device to reboot
            sleep 2
            
            # Check for serial device
            print_status "Checking for serial device..."
            if ls /dev/ttyACM* &>/dev/null || ls /dev/ttyUSB* &>/dev/null; then
                local serial_dev=$(ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -n1)
                print_status "Board detected at: $serial_dev"
                print_status "Connect with: screen $serial_dev 115200"
            fi
        else
            print_error "Failed to copy firmware"
            exit 1
        fi
    else
        print_warning "RP2040 not in bootloader mode"
        echo ""
        print_status "To enter bootloader mode:"
        print_status "  1. Hold the BOOTSEL button on the board"
        print_status "  2. While holding BOOTSEL, connect USB cable (or press RESET)"
        print_status "  3. Release BOOTSEL button"
        print_status "  4. The board should appear as a USB drive named 'RPI-RP2'"
        print_status "  5. Run this script again"
        echo ""
        print_status "Alternative: Use picotool"
        print_status "  sudo picotool load -r $UF2_FILE"
        exit 1
    fi
}

show_usage() {
    echo "OpenFC2040 Flash Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -f FILE        Flash specific UF2 file"
    echo ""
    echo "Examples:"
    echo "  $0                           # Flash default build"
    echo "  $0 -f custom.uf2            # Flash custom UF2 file"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -f)
            UF2_FILE="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
flash_firmware

exit 0
