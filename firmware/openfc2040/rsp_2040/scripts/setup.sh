#!/bin/bash
#
# OpenFC2040 Setup Script
#
# This script performs first-time setup for the OpenFC2040 firmware development environment
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
    print_header "Checking Dependencies"
    
    local missing_deps=()
    
    # Check for required tools
    local tools=("git" "cmake" "make" "arm-none-eabi-gcc" "python3")
    for tool in "${tools[@]}"; do
        if ! command -v $tool &> /dev/null; then
            missing_deps+=("$tool")
            print_error "$tool is not installed"
        else
            local version=$($tool --version 2>&1 | head -n1)
            print_status "$tool: $version"
        fi
    done
    
    # Check Python packages
    print_status "Checking Python packages..."
    python3 -c "import em" 2>/dev/null || missing_deps+=("python3-empy")
    python3 -c "import yaml" 2>/dev/null || missing_deps+=("python3-yaml")
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install missing dependencies and run this script again"
        exit 1
    fi
    
    print_status "All dependencies found!"
}

init_submodules() {
    print_header "Downloading PX4-Autopilot"
    
    cd "$PROJECT_ROOT"
    
    if [ ! -d "$PX4_DIR/.git" ]; then
        print_status "Cloning PX4-Autopilot (this will take several minutes)..."
        git clone --recursive https://github.com/PX4/PX4-Autopilot.git px4-autopilot
        
        if [ $? -eq 0 ]; then
            print_status "PX4-Autopilot cloned successfully"
        else
            print_error "Failed to clone PX4-Autopilot"
            exit 1
        fi
    else
        print_status "PX4-Autopilot already exists"
        print_status "Updating repository..."
        cd "$PX4_DIR"
        git pull
        git submodule update --init --recursive
        cd "$PROJECT_ROOT"
    fi
}

setup_px4() {
    print_header "Setting Up PX4-Autopilot"
    
    if [ ! -d "$PX4_DIR" ]; then
        print_error "PX4-Autopilot directory not found"
        exit 1
    fi
    
    cd "$PX4_DIR"
    
    # Initialize PX4 submodules (if not already done during clone)
    print_status "Ensuring all PX4 submodules are initialized..."
    git submodule update --init --recursive
    
    print_status "PX4-Autopilot setup complete"
}

copy_board_files() {
    print_header "Copying Board Files to PX4"
    
    # Copy board configuration
    print_status "Copying board configuration..."
    mkdir -p "$PX4_DIR/boards/rsp_2040/rsp_2040"
    cp -r "$PROJECT_ROOT/board/"* "$PX4_DIR/boards/rsp_2040/rsp_2040/"
    
    # Copy custom drivers
    print_status "Copying LSM6DS3 driver..."
    mkdir -p "$PX4_DIR/src/drivers/imu/st/lsm6ds3"
    cp -r "$PROJECT_ROOT/drivers/imu/st/lsm6ds3/"* "$PX4_DIR/src/drivers/imu/st/lsm6ds3/"
    
    print_status "Board files copied successfully"
}

create_build_dir() {
    print_header "Creating Build Directory"
    
    mkdir -p "$PROJECT_ROOT/build"
    print_status "Build directory created"
}

show_next_steps() {
    print_header "Setup Complete!"
    
    echo ""
    echo -e "${GREEN}Next steps:${NC}"
    echo ""
    echo "  1. Build the firmware:"
    echo "     ${BLUE}./scripts/build.sh${NC}"
    echo ""
    echo "  2. Flash to board:"
    echo "     ${BLUE}./scripts/flash.sh${NC}"
    echo ""
    echo "  3. Clean build artifacts:"
    echo "     ${BLUE}./scripts/clean.sh${NC}"
    echo ""
    echo -e "${GREEN}For more information, see:${NC}"
    echo "  - README.md"
    echo "  - CONTRIBUTING.md"
    echo "  - docs/"
    echo ""
}

# Main execution
main() {
    print_header "OpenFC2040 Firmware Setup"
    
    check_dependencies
    init_submodules
    setup_px4
    copy_board_files
    create_build_dir
    show_next_steps
}

# Run main function
main

exit 0
