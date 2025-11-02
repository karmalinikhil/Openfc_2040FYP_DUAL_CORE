#!/bin/bash
# OpenFC2040 Debug Session Launcher
# 
# This script launches OpenOCD in the background and provides a GDB interface
# for debugging PX4 firmware on the OpenFC2040 board via Picoprobe SWD.
#
# Usage:
#   ./debug/start_debug_session.sh
#
# Requirements:
#   - Picoprobe (RP2350) flashed with debugprobe_on_pico2.uf2
#   - Wiring: Picoprobe GPIO2/3 → OpenFC2040 SWCLK/SWDIO
#   - openocd installed: sudo apt install openocd
#   - gdb-multiarch installed: sudo apt install gdb-multiarch

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OPENOCD_CFG="$SCRIPT_DIR/openocd_rp2040.cfg"
ELF_FILE="$PROJECT_ROOT/px4-autopilot/build/raspberrypi_pico_default/raspberrypi_pico_default.elf"
GDB_SCRIPT="$SCRIPT_DIR/gdb_init.gdb"
OPENOCD_LOG="$SCRIPT_DIR/openocd.log"
OPENOCD_PID_FILE="$SCRIPT_DIR/openocd.pid"

# Functions
print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    print_header "Checking Dependencies"
    
    local missing=0
    
    if ! command -v openocd &> /dev/null; then
        print_error "openocd not found. Install: sudo apt install openocd"
        missing=1
    else
        print_info "openocd: $(openocd --version 2>&1 | head -n1)"
    fi
    
    if ! command -v gdb-multiarch &> /dev/null; then
        print_error "gdb-multiarch not found. Install: sudo apt install gdb-multiarch"
        missing=1
    else
        print_info "gdb-multiarch: $(gdb-multiarch --version | head -n1)"
    fi
    
    if [ $missing -eq 1 ]; then
        exit 1
    fi
}

check_hardware() {
    print_header "Checking Hardware"
    
    # Check if Picoprobe is connected
    if lsusb | grep -q "2e8a:000c"; then
        print_info "✓ Picoprobe detected (VID:PID = 2e8a:000c)"
    else
        print_warn "Picoprobe NOT detected!"
        print_warn "Expected USB device: 2e8a:000c (Raspberry Pi Debug Probe)"
        print_warn ""
        print_warn "Please ensure:"
        print_warn "  1. Picoprobe is flashed with debugprobe_on_pico2.uf2"
        print_warn "  2. Picoprobe is connected via USB"
        print_warn "  3. Wiring: Picoprobe GPIO2/3 → OpenFC2040 SWCLK/SWDIO"
        echo ""
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

check_firmware() {
    print_header "Checking Firmware"
    
    if [ ! -f "$ELF_FILE" ]; then
        print_error "ELF file not found: $ELF_FILE"
        print_error "Please build PX4 firmware first:"
        print_error "  cd $PROJECT_ROOT"
        print_error "  ./scripts/build.sh"
        exit 1
    fi
    
    print_info "ELF file: $ELF_FILE"
    print_info "Size: $(du -h "$ELF_FILE" | cut -f1)"
}

stop_existing_openocd() {
    if [ -f "$OPENOCD_PID_FILE" ]; then
        local pid=$(cat "$OPENOCD_PID_FILE")
        if ps -p $pid > /dev/null 2>&1; then
            print_info "Stopping existing OpenOCD (PID: $pid)"
            kill $pid 2>/dev/null || true
            sleep 1
        fi
        rm -f "$OPENOCD_PID_FILE"
    fi
    
    # Also check for any OpenOCD processes
    pkill -f "openocd.*rp2040" 2>/dev/null || true
}

start_openocd() {
    print_header "Starting OpenOCD"
    
    print_info "Config: $OPENOCD_CFG"
    print_info "Log: $OPENOCD_LOG"
    
    # Start OpenOCD in background
    openocd -f "$OPENOCD_CFG" > "$OPENOCD_LOG" 2>&1 &
    local pid=$!
    echo $pid > "$OPENOCD_PID_FILE"
    
    print_info "OpenOCD started (PID: $pid)"
    
    # Wait for OpenOCD to initialize
    print_info "Waiting for OpenOCD to initialize..."
    sleep 2
    
    # Check if OpenOCD is still running
    if ! ps -p $pid > /dev/null 2>&1; then
        print_error "OpenOCD failed to start!"
        print_error "Check log: $OPENOCD_LOG"
        cat "$OPENOCD_LOG"
        exit 1
    fi
    
    print_info "✓ OpenOCD ready on port 3333"
}

create_gdb_script() {
    print_header "Creating GDB Script"
    
    cat > "$GDB_SCRIPT" << 'EOF'
# GDB initialization script for OpenFC2040 debugging

# Connect to OpenOCD
target extended-remote :3333

# Load symbol file (already specified on command line)
# file is loaded automatically when gdb-multiarch is called with ELF file

# Reset and halt the CPU
monitor reset halt

# Enable pretty printing
set print pretty on
set print array on

# Helpful aliases
define reset_halt
    monitor reset halt
end

define reset_run
    monitor reset run
end

define show_usb_state
    # Print USB-related variables from firmware
    # This is a placeholder - adjust based on actual symbols
    info variables usb
end

define show_console_state
    # Print console-related variables
    info variables console
end

# Print welcome message
echo \n
echo ========================================\n
echo OpenFC2040 Debug Session\n
echo ========================================\n
echo Useful commands:\n
echo   continue          - Resume execution\n
echo   bt                - Backtrace (call stack)\n
echo   info threads      - Show all threads\n
echo   reset_halt        - Reset and halt\n
echo   reset_run         - Reset and run\n
echo   show_usb_state    - Show USB variables\n
echo \n
echo Firmware halted. Ready for debugging.\n
echo ========================================\n
echo \n

# List current location
list
EOF

    print_info "GDB script created: $GDB_SCRIPT"
}

show_instructions() {
    print_header "Debug Session Ready"
    
    echo -e "${GREEN}OpenOCD is running in the background${NC}"
    echo ""
    echo -e "${YELLOW}To start GDB debugging:${NC}"
    echo ""
    echo -e "  cd $PROJECT_ROOT"
    echo -e "  gdb-multiarch -x $GDB_SCRIPT $ELF_FILE"
    echo ""
    echo -e "${YELLOW}GDB Quick Reference:${NC}"
    echo -e "  (gdb) continue          - Resume execution"
    echo -e "  (gdb) break main        - Set breakpoint at main()"
    echo -e "  (gdb) info threads      - Show all threads (NuttX tasks)"
    echo -e "  (gdb) bt                - Show call stack"
    echo -e "  (gdb) print variable    - Print variable value"
    echo -e "  (gdb) x/16xw 0x20000000 - Dump 16 words from SRAM"
    echo -e "  (gdb) reset_halt        - Reset and halt CPU"
    echo -e "  (gdb) quit              - Exit GDB"
    echo ""
    echo -e "${YELLOW}To stop OpenOCD:${NC}"
    echo -e "  $SCRIPT_DIR/stop_debug_session.sh"
    echo ""
    echo -e "${YELLOW}OpenOCD log:${NC}"
    echo -e "  tail -f $OPENOCD_LOG"
    echo ""
}

main() {
    print_header "OpenFC2040 Debug Session Launcher"
    
    check_dependencies
    check_hardware
    check_firmware
    stop_existing_openocd
    start_openocd
    create_gdb_script
    show_instructions
    
    print_info "✓ Debug session ready!"
}

main "$@"
