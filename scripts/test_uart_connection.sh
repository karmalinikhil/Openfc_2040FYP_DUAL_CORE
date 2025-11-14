#!/bin/bash
#
# Quick UART Connection Test Script
# Tests if USB-to-TTL adapter is properly connected and configured
#

echo "=========================================="
echo "UART Connection Diagnostic Tool"
echo "=========================================="
echo ""

# Check for serial devices
echo "1. Checking for serial devices..."
SERIAL_DEVICES=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null)
if [ -z "$SERIAL_DEVICES" ]; then
    echo "   ❌ No serial devices found!"
    echo "   → Check USB-to-TTL adapter connection"
    echo "   → Run: lsusb (to verify adapter detected)"
    exit 1
else
    echo "   ✅ Found serial devices:"
    for dev in $SERIAL_DEVICES; do
        echo "      - $dev"
    done
fi
echo ""

# Check USB devices
echo "2. Checking USB devices..."
USB_DEVICES=$(lsusb | grep -iE "serial|ftdi|cp210|ch340|pl2303")
if [ -z "$USB_DEVICES" ]; then
    echo "   ⚠️  No USB-to-Serial adapters detected in lsusb"
    echo "   → Adapter might not be recognized"
else
    echo "   ✅ USB-to-Serial adapter detected:"
    echo "      $USB_DEVICES"
fi
echo ""

# Check permissions
echo "3. Checking device permissions..."
FIRST_DEV=$(echo $SERIAL_DEVICES | awk '{print $1}')
if [ -n "$FIRST_DEV" ]; then
    PERMS=$(ls -l $FIRST_DEV 2>/dev/null | awk '{print $1}')
    OWNER=$(ls -l $FIRST_DEV 2>/dev/null | awk '{print $3}')
    if [ "$OWNER" = "$USER" ] || groups | grep -q dialout; then
        echo "   ✅ Device $FIRST_DEV is accessible"
    else
        echo "   ⚠️  Permission issue with $FIRST_DEV"
        echo "   → Run: sudo usermod -a -G dialout $USER"
        echo "   → Then logout and login again"
    fi
fi
echo ""

# Test connection
echo "4. Testing serial connection..."
if [ -n "$FIRST_DEV" ]; then
    echo "   Attempting to connect to $FIRST_DEV at 115200 baud..."
    echo "   (Press Ctrl+A then K to exit screen)"
    echo ""
    echo "   If you see boot messages or 'nsh>' prompt, connection works!"
    echo "   If nothing appears, check:"
    echo "      - TX/RX wires are swapped (TX→RX, RX→TX)"
    echo "      - GND is connected"
    echo "      - Board is powered on"
    echo "      - Firmware is flashed correctly"
    echo ""
    read -p "   Press Enter to start screen (or Ctrl+C to cancel)..."
    screen $FIRST_DEV 115200
else
    echo "   ❌ No serial device available for testing"
fi

