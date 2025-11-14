# UART Console Troubleshooting Guide

**Issue**: No NSH shell prompt when connecting via UART (USB-to-TTL converter)

---

## Quick Checklist

### 1. Hardware Connections ✅

Verify your USB-to-TTL adapter is wired correctly:

```
USB-to-TTL Adapter    →    OpenFC2040 Board
──────────────────         ──────────────────
TX (Adapter)        →    GPIO1 (RX) - Pin 3 of JST-GH6
RX (Adapter)        →    GPIO0 (TX) - Pin 2 of JST-GH6  
GND                 →    GND - Pin 1 of JST-GH6
```

**⚠️ IMPORTANT**: 
- **TX on adapter connects to RX on board** (GPIO1)
- **RX on adapter connects to TX on board** (GPIO0)
- **Do NOT connect VCC/3.3V** - board is powered separately via USB

### 2. Serial Port Settings ✅

**Baud Rate**: Must be **115200** (8N1)
- 8 data bits
- No parity
- 1 stop bit
- No flow control

**Connection Command**:
```bash
# Using screen
screen /dev/ttyUSB0 115200

# Using picocom (recommended)
picocom -b 115200 /dev/ttyUSB0 --imap lfcrlf

# Using minicom
minicom -D /dev/ttyUSB0 -b 115200
```

### 3. Verify Serial Port Detection

```bash
# Check if adapter is detected
ls -l /dev/ttyUSB*

# Check USB devices
lsusb | grep -i "serial\|ftdi\|cp210\|ch340"

# Check kernel messages
dmesg | tail -20 | grep -i "tty\|serial"
```

### 4. Check Firmware Configuration

The firmware should be configured with:
- UART0 as serial console (GPIO0/GPIO1)
- NSH terminal started on `/dev/ttyS0` at 115200 baud
- USB console disabled or secondary

---

## Common Issues and Solutions

### Issue 1: No Output at All

**Symptoms**: 
- Serial port opens but shows nothing
- No boot messages
- No prompt

**Possible Causes**:
1. **Wrong baud rate** - Try different rates: 9600, 57600, 115200, 230400
2. **TX/RX swapped** - Swap the TX and RX wires
3. **GND not connected** - Ensure GND is connected
4. **Firmware not running** - Check if board is powered and firmware flashed correctly
5. **Wrong serial port** - Verify you're using the correct `/dev/ttyUSB*` device

**Solutions**:
```bash
# Test with different baud rates
for baud in 9600 57600 115200 230400; do
    echo "Trying $baud baud..."
    timeout 2 screen /dev/ttyUSB0 $baud || true
done

# Check if firmware is running (LED should blink)
# Check boot messages via USB console (if available)
```

### Issue 2: Garbled/Corrupted Output

**Symptoms**:
- Random characters
- Unreadable text
- Partial messages

**Possible Causes**:
1. **Baud rate mismatch** - Most likely cause
2. **Noise/interference** - Check wiring, use shorter cables
3. **Ground loop** - Ensure single GND connection

**Solutions**:
- Verify baud rate is exactly 115200
- Try different USB port (avoid USB 3.0 hubs)
- Check cable quality and length (< 1m recommended)

### Issue 3: Console Goes to USB Instead of UART

**Symptoms**:
- USB console works but UART doesn't
- No output on UART

**Possible Causes**:
- NuttX configuration has USB console as primary
- `CONFIG_NSH_USBCONSOLE=y` might be taking precedence

**Solutions**:
- Check NuttX defconfig: `CONFIG_UART0_SERIAL_CONSOLE=y` should be set
- Verify `CONFIG_NSH_USBCONSOLE` is not conflicting
- Check if `nshterm` is actually starting on UART0

### Issue 4: NSH Terminal Not Starting

**Symptoms**:
- Boot messages appear but no `nsh>` prompt
- System boots but no shell

**Possible Causes**:
- `nshterm` command not executing
- UART device not available
- Configuration issue in startup scripts

**Solutions**:
- Check `rc.board_defaults` has: `nshterm start -d /dev/ttyS0 -b 115200`
- Verify UART0 is properly initialized in board code
- Check boot log for errors (if accessible)

---

## Debugging Steps

### Step 1: Verify Hardware

```bash
# 1. Check adapter is detected
lsusb
# Should show: FTDI, CP2102, CH340, or similar

# 2. Check serial port exists
ls -l /dev/ttyUSB*
# Should show: /dev/ttyUSB0 (or similar)

# 3. Check permissions
ls -l /dev/ttyUSB0
# Should be readable/writable by your user or dialout group
```

### Step 2: Test Serial Connection

```bash
# Connect and look for ANY output
picocom -b 115200 /dev/ttyUSB0

# If nothing appears, try:
# - Press Enter a few times
# - Press Ctrl+C
# - Try different baud rates
```

### Step 3: Check Firmware is Running

**Visual Check**:
- RGB LED should show startup sequence (Red → Green → Blue)
- LED should blink or show status

**If no LED activity**:
- Firmware might not be running
- Re-flash firmware
- Check power supply

### Step 4: Verify UART Configuration

Check the built firmware configuration:

```bash
cd firmware/openfc2040/rsp_2040
grep -r "UART0_SERIAL_CONSOLE\|NSH_USBCONSOLE" px4-autopilot/build/rsp_2040_default/.config 2>/dev/null
```

Should show:
```
CONFIG_UART0_SERIAL_CONSOLE=y
# CONFIG_NSH_USBCONSOLE is not set  (or commented out)
```

### Step 5: Check Startup Scripts

```bash
# Verify nshterm is configured to start
cat board/init/rc.board_defaults | grep nshterm
# Should show: nshterm start -d /dev/ttyS0 -b 115200
```

---

## Advanced Debugging

### Enable Verbose Boot Messages

If you can access boot messages, check for:
- UART initialization errors
- Device creation errors
- NSH startup errors

### Use SWD Debugger

If you have Picoprobe/SWD debugger:
1. Connect via SWD
2. Use GDB to check if UART is initialized
3. Set breakpoints in UART initialization code
4. Check register values

### Test with Known Working Firmware

Try the UART console firmware that was previously built:
```bash
# Flash the known-working UART firmware
cp firmware/openfc2040/rsp_2040/raspberrypi_pico_minimal_UART.uf2 /media/$USER/RPI-RP2/
```

If this works, the issue is in the new firmware configuration.

---

## Configuration Fixes

### Fix 1: Ensure UART Console is Primary

Edit `board/nuttx-config/nsh/defconfig`:
```bash
# Ensure these are set:
CONFIG_UART0_SERIAL_CONSOLE=y
# CONFIG_NSH_USBCONSOLE is not set  (comment out or remove)
```

### Fix 2: Verify NSH Terminal Startup

Edit `board/init/rc.board_defaults`:
```bash
# Ensure this line exists:
nshterm start -d /dev/ttyS0 -b 115200
```

### Fix 3: Rebuild Firmware

After making changes:
```bash
cd firmware/openfc2040/rsp_2040
./scripts/clean.sh
./scripts/build.sh all
./scripts/flash.sh
```

---

## Expected Behavior

### On Successful Connection

You should see:
1. **Boot messages** (NuttX startup, PX4 initialization)
2. **NSH prompt**: `nsh>`
3. **Commands work**: `help`, `ver`, `free`, etc.

### Boot Sequence Example

```
NuttShell (NSH) NuttX-12.x.x
nsh> help
help usage:  help [-v] [<cmd>]

  [           cd          help        ls          mh          ps
  cat         cp          hexdump     mb          mv          pwd
  ...
nsh> ver all
HW arch: PX4
HW type: OpenFC2040
HW version: 0x00000000
HW vendor: Raspberry Pi
FW git-hash: abc123def
FW version: 1.17.0
FW vendor: PX4
OS: NuttX
OS version: Release 12.x.x
OS git-hash: xyz789abc
Build datetime: Nov 10 2025 15:03:00
Build uri: localhost
Toolchain: gcc-arm-none-eabi 13.2.1
PX4GUID: 00010000000035313833363951070028002f
nsh>
```

---

## Quick Test Commands

Once you get the prompt, test with:

```bash
nsh> help          # List all commands
nsh> ver all       # Show version info
nsh> free          # Check memory
nsh> dmesg         # View boot log
nsh> uorb top      # Monitor message bus
nsh> led_control test  # Test RGB LEDs
```

---

## Still Not Working?

If none of the above works:

1. **Double-check wiring** - Most common issue!
2. **Try different USB-to-TTL adapter** - Some adapters have issues
3. **Check board power** - Ensure board is powered via USB
4. **Verify firmware flashed correctly** - Re-flash if unsure
5. **Check for hardware damage** - Test GPIO0/GPIO1 with multimeter
6. **Use SWD debugger** - For low-level debugging

---

## Contact and Resources

- **Documentation**: See `docs/PIN_CONNECTIONS.md` for pin mappings
- **Hardware Guide**: See `docs/DEBUG_GUIDE.md` for SWD debugging
- **Project Context**: See `docs/PROJECT_CONTEXT.md` for architecture

---

**Last Updated**: November 10, 2025

