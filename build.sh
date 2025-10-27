#!/bin/bash
#
# OpenFC2040 PX4 Build Script
#
# This script automates the build process for the OpenFC2040 flight controller
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
BOARD_NAME="rsp_2040"
PX4_ROOT="../PX4-Autopilot-main"
BUILD_TYPE="default"
THREADS=$(nproc)

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

check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for required tools
    local tools=("cmake" "make" "arm-none-eabi-gcc" "python3")
    for tool in "${tools[@]}"; do
        if ! command -v $tool &> /dev/null; then
            print_error "$tool is not installed"
            exit 1
        fi
    done
    
    print_status "All dependencies found"
}

setup_board() {
    print_status "Setting up OpenFC2040 board files..."
    
    # Check if PX4 directory exists
    if [ ! -d "$PX4_ROOT" ]; then
        print_error "PX4-Autopilot directory not found at $PX4_ROOT"
        exit 1
    fi
    
    # Copy board files to PX4
    print_status "Copying board configuration..."
    cp -r . "$PX4_ROOT/boards/$BOARD_NAME"
    
    # Copy LSM6DS3 driver
    print_status "Copying LSM6DS3 driver..."
    mkdir -p "$PX4_ROOT/src/drivers/imu/st/lsm6ds3"
    cp -r drivers/imu/st/lsm6ds3/* "$PX4_ROOT/src/drivers/imu/st/lsm6ds3/"
    
    print_status "Board setup complete"
}

build_firmware() {
    print_status "Building firmware for OpenFC2040..."
    
    cd "$PX4_ROOT"
    
    # Clean previous build if requested
    if [ "$1" == "clean" ]; then
        print_status "Cleaning previous build..."
        make distclean
    fi
    
    # Build the firmware
    print_status "Starting build with $THREADS threads..."
    make ${BOARD_NAME}_${BUILD_TYPE} -j$THREADS
    
    if [ $? -eq 0 ]; then
        print_status "Build successful!"
        
        # Show build artifacts
        echo ""
        print_status "Build artifacts:"
        ls -lh "build/${BOARD_NAME}_${BUILD_TYPE}/${BOARD_NAME}_${BUILD_TYPE}."* 2>/dev/null || true
        
        # Calculate sizes
        if [ -f "build/${BOARD_NAME}_${BUILD_TYPE}/${BOARD_NAME}_${BUILD_TYPE}.bin" ]; then
            SIZE=$(stat -c%s "build/${BOARD_NAME}_${BUILD_TYPE}/${BOARD_NAME}_${BUILD_TYPE}.bin")
            SIZE_KB=$((SIZE / 1024))
            print_status "Firmware size: ${SIZE_KB}KB / 2048KB ($(((SIZE_KB * 100) / 2048))% used)"
        fi
    else
        print_error "Build failed!"
        exit 1
    fi
}

generate_uf2() {
    print_status "Generating UF2 file for drag-and-drop flashing..."
    
    cd "$PX4_ROOT"
    
    if [ -f "build/${BOARD_NAME}_${BUILD_TYPE}/${BOARD_NAME}_${BUILD_TYPE}.bin" ]; then
        # Use uf2conv.py if available
        if [ -f "Tools/uf2conv.py" ]; then
            python3 Tools/uf2conv.py \
                -b 0x10000000 \
                -f 0xe48bff56 \
                "build/${BOARD_NAME}_${BUILD_TYPE}/${BOARD_NAME}_${BUILD_TYPE}.bin" \
                -o "build/${BOARD_NAME}_${BUILD_TYPE}/${BOARD_NAME}_${BUILD_TYPE}.uf2"
        else
            print_warning "uf2conv.py not found, UF2 generation skipped"
        fi
    fi
}

flash_firmware() {
    print_status "Flashing firmware to OpenFC2040..."
    
    # Check for device in bootloader mode
    if [ -d "/media/$USER/RPI-RP2" ]; then
        print_status "RP2040 bootloader detected"
        cp "$PX4_ROOT/build/${BOARD_NAME}_${BUILD_TYPE}/${BOARD_NAME}_${BUILD_TYPE}.uf2" "/media/$USER/RPI-RP2/"
        print_status "Firmware flashed successfully!"
    else
        print_warning "RP2040 not in bootloader mode"
        print_status "To enter bootloader mode:"
        print_status "  1. Hold BOOTSEL button"
        print_status "  2. Connect USB or press RESET"
        print_status "  3. Release BOOTSEL"
        print_status "  4. Run this script again with 'flash' option"
    fi
}

run_tests() {
    print_status "Running tests..."
    
    cd "$PX4_ROOT"
    
    # Run unit tests if available
    if [ -f "build/${BOARD_NAME}_${BUILD_TYPE}/unit_tests" ]; then
        ./build/${BOARD_NAME}_${BUILD_TYPE}/unit_tests
    fi
    
    print_status "Tests complete"
}

show_usage() {
    echo "OpenFC2040 PX4 Build Script"
    echo ""
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  setup     - Copy board files to PX4-Autopilot"
    echo "  build     - Build the firmware"
    echo "  clean     - Clean and rebuild"
    echo "  flash     - Flash firmware via UF2 bootloader"
    echo "  test      - Run tests"
    echo "  all       - Setup, build, and generate UF2"
    echo ""
    echo "Options:"
    echo "  -p PATH   - Path to PX4-Autopilot (default: $PX4_ROOT)"
    echo "  -j N      - Number of build threads (default: $THREADS)"
    echo "  -t TYPE   - Build type (default: $BUILD_TYPE)"
    echo ""
    echo "Examples:"
    echo "  $0 all                    # Complete build"
    echo "  $0 build -j 4            # Build with 4 threads"
    echo "  $0 flash                 # Flash firmware"
}

# Parse command line arguments
COMMAND=${1:-help}
shift || true

while getopts "p:j:t:h" opt; do
    case $opt in
        p)
            PX4_ROOT="$OPTARG"
            ;;
        j)
            THREADS="$OPTARG"
            ;;
        t)
            BUILD_TYPE="$OPTARG"
            ;;
        h)
            show_usage
            exit 0
            ;;
        \?)
            print_error "Invalid option: -$OPTARG"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
case $COMMAND in
    setup)
        check_dependencies
        setup_board
        ;;
    build)
        check_dependencies
        build_firmware
        generate_uf2
        ;;
    clean)
        check_dependencies
        build_firmware clean
        generate_uf2
        ;;
    flash)
        flash_firmware
        ;;
    test)
        run_tests
        ;;
    all)
        check_dependencies
        setup_board
        build_firmware
        generate_uf2
        print_status "Build complete! Firmware ready for flashing."
        ;;
    help|*)
        show_usage
        ;;
esac

exit 0
