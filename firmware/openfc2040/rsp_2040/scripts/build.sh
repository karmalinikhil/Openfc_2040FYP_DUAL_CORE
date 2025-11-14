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
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BOARD_VENDOR="raspberrypi"
BOARD_MODEL="pico"
BOARD_LABEL="default"
BOARD_TARGET="${BOARD_VENDOR}_${BOARD_MODEL}_${BOARD_LABEL}"
PX4_ROOT="$PROJECT_ROOT/px4-autopilot"
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

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
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

check_px4_submodule() {
    # Check if PX4-Autopilot directory exists and has content
    if [ ! -d "$PX4_ROOT" ]; then
        print_error "PX4-Autopilot directory not found"
        print_status "Please run setup first:"
        print_status "  ./scripts/setup.sh"
        exit 1
    fi
    
    # Check if it has actual PX4 content (CMakeLists.txt is a key file)
    if [ ! -f "$PX4_ROOT/CMakeLists.txt" ]; then
        print_error "PX4-Autopilot directory exists but appears empty"
        print_status "Please run setup first:"
        print_status "  ./scripts/setup.sh"
        exit 1
    fi
}

setup_board() {
    print_header "Setting up OpenFC2040 Board Files"
    
    check_px4_submodule
    
    # Copy only board configuration (not source files)
    print_status "Copying board configuration to raspberrypi/pico..."
    cp "$PROJECT_ROOT/board/src/board_config.h" "$PX4_ROOT/boards/raspberrypi/pico/src/"
    cp "$PROJECT_ROOT/board/default.px4board" "$PX4_ROOT/boards/raspberrypi/pico/"
    
    print_status "Board setup complete (using Pico board source files)"
}

build_firmware() {
    print_header "Building Firmware for OpenFC2040"
    
    check_px4_submodule
    
    cd "$PX4_ROOT"
    
    # Clean previous build if requested
    if [ "$1" == "clean" ]; then
        print_status "Cleaning previous build..."
        make distclean
    fi
    
    # Build the firmware
    print_status "Starting build with $THREADS threads..."
    print_status "Build target: $BOARD_TARGET"
    make ${BOARD_TARGET} -j$THREADS
    
    if [ $? -eq 0 ]; then
        print_status "Build successful!"
        
        # Show build artifacts
        echo ""
        print_status "Build artifacts:"
        ls -lh "build/${BOARD_TARGET}/${BOARD_TARGET}."* 2>/dev/null || true
        
        # Calculate sizes
        if [ -f "build/${BOARD_TARGET}/${BOARD_TARGET}.bin" ]; then
            SIZE=$(stat -c%s "build/${BOARD_TARGET}/${BOARD_TARGET}.bin" 2>/dev/null || stat -f%z "build/${BOARD_TARGET}/${BOARD_TARGET}.bin" 2>/dev/null)
            SIZE_KB=$((SIZE / 1024))
            print_status "Firmware size: ${SIZE_KB}KB / 2048KB ($(((SIZE_KB * 100) / 2048))% used)"
        fi
    else
        print_error "Build failed!"
        exit 1
    fi
}

generate_uf2() {
    print_header "Generating UF2 File"
    
    cd "$PX4_ROOT"
    
    if [ -f "build/${BOARD_TARGET}/${BOARD_TARGET}.bin" ]; then
        # Use uf2conv.py if available (prefer PX4 Tools/, fallback to local project copy)
        UF2CONV=""
        if [ -f "Tools/uf2conv.py" ]; then
            UF2CONV="Tools/uf2conv.py"
        elif [ -f "${PROJECT_ROOT}/uf2conv.py" ]; then
            UF2CONV="${PROJECT_ROOT}/uf2conv.py"
        fi

        if [ -n "$UF2CONV" ]; then
            print_status "Converting binary to UF2 format using: $UF2CONV"
            python3 "$UF2CONV" \
                -b 0x10000000 \
                -f 0xe48bff56 \
                "build/${BOARD_TARGET}/${BOARD_TARGET}.bin" \
                -o "build/${BOARD_TARGET}/${BOARD_TARGET}.uf2"

            if [ -f "build/${BOARD_TARGET}/${BOARD_TARGET}.uf2" ]; then
                print_status "UF2 file generated successfully"
                ls -lh "build/${BOARD_TARGET}/${BOARD_TARGET}.uf2"
            else
                print_warning "UF2 conversion did not produce output file"
            fi
        else
            print_warning "uf2conv.py not found in PX4 Tools/ or project root; UF2 generation skipped"
        fi
    else
        print_error "Binary file not found at: build/${BOARD_TARGET}/${BOARD_TARGET}.bin"
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
    echo "  test      - Run tests"
    echo "  all       - Setup, build, and generate UF2 (default)"
    echo ""
    echo "Options:"
    echo "  -j N      - Number of build threads (default: auto-detect)"
    echo "  -t TYPE   - Build type (default: $BUILD_TYPE)"
    echo ""
    echo "Examples:"
    echo "  $0 all                    # Complete build"
    echo "  $0 build -j 4            # Build with 4 threads"
    echo ""
    echo "Note: Use ./scripts/flash.sh to flash firmware to board"
}

# Parse command line arguments
COMMAND=${1:-all}
shift || true

while getopts "j:t:h" opt; do
    case $opt in
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
    test)
        run_tests
        ;;
    all)
        check_dependencies
        setup_board
        build_firmware
        generate_uf2
        print_header "Build Complete!"
        print_status "Firmware ready for flashing"
        print_status "Flash with: ./scripts/flash.sh"
        ;;
    help|*)
        show_usage
        ;;
esac

exit 0
