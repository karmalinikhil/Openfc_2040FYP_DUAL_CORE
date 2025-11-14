# UART Console Fix Summary

**Date**: November 10, 2025  
**Issue**: No NSH shell prompt on UART (GPIO0/GPIO1) via USB-to-TTL adapter

---

## Root Cause Analysis

The firmware had several configuration issues preventing UART console from working:

1. **USB Console Conflict**: Both USB and UART console were enabled, causing conflicts
2. **UART Initialization**: GPIO pin configuration didn't match the Pico board pattern
3. **Baud Rate**: Not explicitly configured in defconfig
4. **Redundant nshterm**: Manual nshterm start was conflicting with automatic console

---

## Fixes Applied

### 1. Disabled USB Console
**File**: `board/nuttx-config/nsh/defconfig`
- Changed: `CONFIG_NSH_USBCONSOLE=y` → `# CONFIG_NSH_USBCONSOLE is not set`
- **Reason**: USB console was taking precedence over UART console

### 2. Fixed UART GPIO Initialization
**File**: `board/src/init.c`
- **Before**: Hardcoded GPIO0/GPIO1
- **After**: Uses `CONFIG_RP2040_UART0_GPIO` pattern (matches Pico board)
- **Code**:
  ```c
  rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO, RP2040_GPIO_FUNC_UART);     /* TX */
  rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO + 1, RP2040_GPIO_FUNC_UART); /* RX */
  ```
- **Reason**: Ensures proper UART initialization matching NuttX expectations

### 3. Added Explicit Baud Rate Configuration
**File**: `board/nuttx-config/nsh/defconfig`
- Added: `CONFIG_RP2040_UART0_BAUD=115200`
- Added: `CONFIG_UART0_BAUD=115200`
- **Reason**: Ensures baud rate is explicitly set (though default is 115200)

### 4. Removed Redundant nshterm Command
**File**: `board/init/rc.board_defaults`
- **Before**: `nshterm start -d /dev/ttyS0 -b 115200`
- **After**: Commented out (console starts automatically)
- **Reason**: When `CONFIG_UART0_SERIAL_CONSOLE=y` is set, NSH automatically starts on the console

---

## Configuration Summary

### Current NuttX Configuration
```bash
CONFIG_UART0_SERIAL_CONSOLE=y          # UART0 is the console
CONFIG_RP2040_UART0=y                  # UART0 enabled
CONFIG_RP2040_UART0_GPIO=0             # GPIO0/GPIO1 (from board.h)
CONFIG_RP2040_UART0_BAUD=115200        # Baud rate
# CONFIG_NSH_USBCONSOLE is not set     # USB console disabled
```

### Hardware Configuration
- **UART0 TX**: GPIO0 (Pin 2 of JST-GH6 telemetry port)
- **UART0 RX**: GPIO1 (Pin 3 of JST-GH6 telemetry port)
- **Baud Rate**: 115200 (8N1)
- **Device**: `/dev/ttyS0`

---

## Next Steps

### 1. Rebuild Firmware
```bash
cd firmware/openfc2040/rsp_2040
./scripts/clean.sh
./scripts/build.sh all
```

### 2. Flash Firmware
```bash
./scripts/flash.sh
```

### 3. Test UART Connection

**Hardware Wiring**:
```
USB-to-TTL Adapter  →  OpenFC2040 JST-GH6
─────────────────      ──────────────────
TX (Adapter)      →    Pin 3 (RX/GPIO1)
RX (Adapter)      →    Pin 2 (TX/GPIO0)
GND               →    Pin 1 (GND)
```

**Connection Command**:
```bash
screen /dev/ttyUSB0 115200
# or
picocom -b 115200 /dev/ttyUSB0 --imap lfcrlf
```

### 4. Expected Output

You should see:
1. **Boot messages** (NuttX startup, PX4 initialization)
2. **NSH prompt**: `nsh>`
3. **Commands work**: `help`, `ver`, `free`, etc.

---

## Troubleshooting

If still no output:

1. **Verify wiring** (most common issue):
   - TX on adapter → RX on board (GPIO1)
   - RX on adapter → TX on board (GPIO0)
   - GND connected

2. **Check baud rate**:
   - Must be exactly 115200
   - Try: `screen /dev/ttyUSB0 115200`

3. **Verify device**:
   ```bash
   ls -l /dev/ttyUSB*
   dmesg | tail -20 | grep tty
   ```

4. **Check firmware flashed**:
   - Verify LED startup sequence (Red → Green → Blue)
   - Re-flash if unsure

5. **Try different baud rates** (if 115200 doesn't work):
   ```bash
   for baud in 9600 57600 115200 230400; do
       echo "Trying $baud..."
       timeout 3 screen /dev/ttyUSB0 $baud || true
   done
   ```

---

## Files Modified

1. `board/nuttx-config/nsh/defconfig` - Console configuration
2. `board/src/init.c` - UART GPIO initialization
3. `board/init/rc.board_defaults` - Removed redundant nshterm

---

## Technical Details

### How NuttX Console Works

When `CONFIG_UART0_SERIAL_CONSOLE=y` is set:
1. NuttX automatically initializes UART0 as the console
2. `arm_lowputc()` sends characters to UART0
3. NSH shell automatically starts on the console
4. `/dev/ttyS0` is registered as UART0 device
5. No manual `nshterm start` needed

### Why nshterm Was Removed

- `nshterm` is used to start NSH on a **non-console** device
- When console is on UART0, NSH is already running
- Starting `nshterm` on the same device can cause conflicts

---

**Status**: ✅ Configuration fixed, ready for rebuild and test

