#!/bin/bash
#
# OpenFC2040 Clean Script
#
# This script cleans build artifacts and temporary files
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
BUILD_DIR="$PROJECT_ROOT/build"

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

clean_px4_build() {
    print_header "Cleaning PX4 Build Artifacts"
    
    if [ -d "$PX4_DIR" ]; then
        cd "$PX4_DIR"
        
        if [ -d "build" ]; then
            print_status "Removing PX4 build directory..."
            rm -rf build
            print_status "PX4 build directory removed"
        else
            print_status "No PX4 build directory to clean"
        fi
        
        # Clean other PX4 artifacts
        if [ "$DEEP_CLEAN" = true ]; then
            print_status "Performing deep clean..."
            make distclean 2>/dev/null || true
        fi
    else
        print_warning "PX4 directory not found"
    fi
}

clean_local_build() {
    print_header "Cleaning Local Build Directory"
    
    if [ -d "$BUILD_DIR" ]; then
        print_status "Removing local build directory..."
        rm -rf "$BUILD_DIR"
        print_status "Local build directory removed"
    else
        print_status "No local build directory to clean"
    fi
}

clean_temp_files() {
    print_header "Cleaning Temporary Files"
    
    cd "$PROJECT_ROOT"
    
    # Remove common temporary files
    print_status "Removing temporary files..."
    find . -type f -name "*.pyc" -delete 2>/dev/null || true
    find . -type f -name "*.pyo" -delete 2>/dev/null || true
    find . -type f -name "*.log" -delete 2>/dev/null || true
    find . -type f -name "*.bak" -delete 2>/dev/null || true
    find . -type f -name "*.backup" -delete 2>/dev/null || true
    find . -type f -name "*~" -delete 2>/dev/null || true
    find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
    
    print_status "Temporary files removed"
}

clean_cmake_cache() {
    print_header "Cleaning CMake Cache"
    
    cd "$PROJECT_ROOT"
    
    find . -name "CMakeCache.txt" -delete 2>/dev/null || true
    find . -type d -name "CMakeFiles" -exec rm -rf {} + 2>/dev/null || true
    
    print_status "CMake cache cleaned"
}

show_disk_space() {
    print_header "Disk Space Summary"
    
    if [ -d "$PX4_DIR" ]; then
        local px4_size=$(du -sh "$PX4_DIR" 2>/dev/null | cut -f1)
        print_status "PX4 directory size: $px4_size"
    fi
    
    if [ -d "$PROJECT_ROOT" ]; then
        local project_size=$(du -sh "$PROJECT_ROOT" 2>/dev/null | cut -f1)
        print_status "Total project size: $project_size"
    fi
}

show_usage() {
    echo "OpenFC2040 Clean Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -d, --deep     Deep clean (includes PX4 distclean)"
    echo "  -a, --all      Clean everything including submodules"
    echo ""
    echo "Examples:"
    echo "  $0              # Standard clean"
    echo "  $0 --deep       # Deep clean with distclean"
    echo "  $0 --all        # Clean everything"
}

# Parse command line arguments
DEEP_CLEAN=false
CLEAN_ALL=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -d|--deep)
            DEEP_CLEAN=true
            shift
            ;;
        -a|--all)
            CLEAN_ALL=true
            DEEP_CLEAN=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
print_header "OpenFC2040 Clean Script"

clean_px4_build
clean_local_build
clean_temp_files

if [ "$DEEP_CLEAN" = true ]; then
    clean_cmake_cache
fi

show_disk_space

print_header "Clean Complete!"

if [ "$CLEAN_ALL" = true ]; then
    print_warning "To rebuild, you may need to run setup again:"
    print_status "  ./scripts/setup.sh"
fi

exit 0
