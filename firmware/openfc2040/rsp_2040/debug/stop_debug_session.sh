#!/bin/bash
# Stop OpenOCD debug session

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENOCD_PID_FILE="$SCRIPT_DIR/openocd.pid"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}[INFO]${NC} Stopping OpenOCD debug session..."

if [ -f "$OPENOCD_PID_FILE" ]; then
    pid=$(cat "$OPENOCD_PID_FILE")
    if ps -p $pid > /dev/null 2>&1; then
        echo -e "${GREEN}[INFO]${NC} Killing OpenOCD (PID: $pid)"
        kill $pid 2>/dev/null || true
        sleep 1
        
        # Force kill if still running
        if ps -p $pid > /dev/null 2>&1; then
            echo -e "${GREEN}[INFO]${NC} Force killing OpenOCD"
            kill -9 $pid 2>/dev/null || true
        fi
    else
        echo -e "${GREEN}[INFO]${NC} OpenOCD not running (stale PID file)"
    fi
    rm -f "$OPENOCD_PID_FILE"
else
    echo -e "${GREEN}[INFO]${NC} No OpenOCD PID file found"
fi

# Also kill any stray openocd processes
pkill -f "openocd.*rp2040" 2>/dev/null || true

echo -e "${GREEN}[INFO]${NC} ✓ OpenOCD stopped"
