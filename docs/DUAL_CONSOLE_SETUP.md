This guide documents how to configure the OpenFC2040 board to have:
- **NSH Shell** on UART0 (Telemetry Port) via USB-to-TTL adapter
- **MAVLink** on USB CDC for QGroundControl/Mission Planner

---

## Overview

| Interface | Device (NuttX) | Physical Port | Host Device | Baud Rate |
|-----------|---------------|---------------|-------------|-----------|
| **NSH Console** | `/dev/ttyS0` | UART0 (GPIO0/GPIO1) | `/dev/ttyUSB0` | 57600 |
| **MAVLink** | `/dev/ttyACM0` | USB CDC/ACM | `/dev/ttyACM0` | 115200 (ignored) |

---

## Hardware Connections

### UART0 (NSH Console) - Telemetry Port JST-GH6

```
USB-to-TTL Adapter    →    OpenFC2040 Board
──────────────────         ──────────────────
TX (Adapter)         →    GPIO1 (RX) - Pin 3 of JST-GH6
RX (Adapter)         →    GPIO0 (TX) - Pin 2 of JST-GH6  
GND                  →    GND - Pin 1 of JST-GH6
```

**⚠️ IMPORTANT**: 
- TX on adapter connects to RX on board (GPIO1)
- RX on adapter connects to TX on board (GPIO0)

### USB CDC (MAVLink)

Simply connect the board's microUSB port directly to your computer using a microUSB cable.

---

## Configuration Files Changed

### 1. `boards/raspberrypi/pico/nuttx-config/nsh/defconfig`


```makefile
# UART0 enabled but NOT as serial console
CONFIG_RP2040_UART0=y
CONFIG_RP2040_UART1=y
# CONFIG_UART0_SERIAL_CONSOLE is not set   # <-- Important!
CONFIG_UART0_BAUD=57600
CONFIG_UART0_BITS=8
CONFIG_UART0_PARITY=0
CONFIG_UART0_2STOP=0
CONFIG_UART0_RXBUFSIZE=256
CONFIG_UART0_TXBUFSIZE=256

# USB CDC enabled but not as NSH console
CONFIG_CDCACM=y
# CONFIG_NSH_USBCONSOLE is not set         # <-- Important!
CONFIG_DEV_CONSOLE=y

# System console disabled (nshterm provides shell)
CONFIG_SYSTEM_CDCACM=y
```

**Why these settings?**

| Setting | Value | Reason |
|---------|-------|--------|
| `CONFIG_UART0_SERIAL_CONSOLE` | not set | Prevents ALL system output going to UART0 |
| `CONFIG_NSH_USBCONSOLE` | not set | NSH doesn't auto-start on USB |
| `CONFIG_DEV_CONSOLE` | y | Enables /dev/console device |
| `CONFIG_UART0_BAUD` | 57600 | Standard PX4 debug console baud rate |

---

### 2. `boards/raspberrypi/pico/default.px4board`

**Remove Telemetry Port Assignment:**

```makefile
# ---------------------------------------------------------
# Serial Port Mappings (RP2040 has 2 UARTs)
# ---------------------------------------------------------
# UART0 (GPIO 0/1): /dev/ttyS0 - Used for NSH console (not telemetry)
# UART1 (GPS on GPIO 4/5): /dev/ttyS1
# Telemetry runs on USB CDC (/dev/ttyACM0) via MAVLink

# CONFIG_BOARD_SERIAL_TEL1 is not set - UART0 reserved for NSH
CONFIG_BOARD_SERIAL_GPS1="/dev/ttyS1"
```

**Why?**

If `CONFIG_BOARD_SERIAL_TEL1="/dev/ttyS0"` is set, PX4's telemetry system will send binary MAVLink data to UART0, causing garbled output mixed with NSH.

---

### 3. `boards/raspberrypi/pico/init/rc.board_defaults`

**Add nshterm Startup:**

```bash
#!/bin/sh
#
# board specific defaults
#------------------------------------------------------------------------------

param set-default BAT1_V_DIV 13.653333333
param set-default BAT1_A_PER_V 36.367515152

# system_power unavailable
param set-default CBRK_SUPPLY_CHK 894281

# Start RGB LED driver and set initial color to white
rgbled_gpio start
led_control on -c white

# Delay to let system boot complete before starting NSH on UART
sleep 2

# Start NSH terminal on UART0 (Telemetry Port: GPIO0/GPIO1)
# Connect via USB-to-TTL adapter at 57600 baud
nshterm /dev/ttyS0 &
```

**Key Points:**

| Item | Value | Reason |
|------|-------|--------|
| `sleep 2` | 2 seconds | Wait for PX4 boot to complete before starting shell |
| `nshterm /dev/ttyS0 &` | Background | Runs NSH shell on UART0 in background |
| No `-b` flag | N/A | `nshterm` doesn't support baud rate flag - uses defconfig setting |

---

### 4. `boards/raspberrypi/pico/init/rc.board_mavlink`

**MAVLink on USB CDC:**

```bash
#!/bin/sh
#
# board specific MAVLink startup script.
#------------------------------------------------------------------------------

# Start MAVLink on the USB CDC ACM port
# Note: Baud rate is ignored for USB CDC (runs at USB speed)
# Using 115200 for consistency with standard PX4 configs
mavlink start -d /dev/ttyACM0 -b 115200 -m normal
```

**Note:** Baud rate doesn't matter for USB CDC - it always runs at USB speed (12 Mbps Full-Speed USB).

---

## Connecting to the Console

### NSH Console (via USB-to-TTL)

```bash
# Using picocom 
picocom -b 57600 /dev/ttyUSB0

# Using screen
screen /dev/ttyUSB0 57600

# Using minicom
minicom -D /dev/ttyUSB0 -b 57600
```

**After connecting:**
- Wait a few seconds after board boot
- Press **Enter** to get `nsh>` prompt
- Type `help` to see available commands

### MAVLink (via USB Direct)

1. Connect board USB directly to computer
2. Open QGroundControl or Mission Planner
3. It should auto-detect on `/dev/ttyACM0` on Linux or `COM4` on Windows

---

## Useful NSH Commands

```bash
# System info
ver all              # Show version information
free                 # Memory usage
top                  # Process list
dmesg                # Boot log

# Sensor commands
listener sensor_baro          # Barometer data
listener sensor_accel         # Accelerometer data
listener sensor_gyro          # Gyroscope data
uorb top                      # All uORB topics

# Driver status
dps310 status        # Barometer driver status
lsm9ds1 status       # IMU driver status

# LED control
led_control on -c red
led_control on -c green
led_control on -c blue
led_control on -c white
led_control off
```

---

## Troubleshooting

### Problem: Garbled output on UART

**Cause:** Wrong baud rate or binary data from telemetry

**Solution:**
1. Ensure baud rate is 57600
2. Verify `CONFIG_BOARD_SERIAL_TEL1` is NOT set in `default.px4board`
3. Verify `CONFIG_UART0_SERIAL_CONSOLE` is NOT set in `defconfig`

### Problem: No output at all

**Cause:** nshterm not starting or wrong device

**Solution:**
1. Wait 5+ seconds after boot (sleep 2 + PX4 init time)
2. Press Enter multiple times
3. Check `rc.board_defaults` has `nshterm /dev/ttyS0 &`

### Problem: USB CDC (/dev/ttyACM0) not appearing

**Cause:** USB device not enumerating

**Solution:**
1. Check `CONFIG_CDCACM=y` in defconfig
2. Check `CONFIG_USBDEV=y` in defconfig
3. Verify USB cable is data-capable (not charge-only)

---

## Debug Output (Optional)

To enable SPI debug output for troubleshooting, uncomment lines in `boards/raspberrypi/pico/src/init.c`:

```c
// Uncomment for debugging SPI1 chip select (WARNING: very verbose!)
// syslog(LOG_DEBUG, "SPI1 select: devid=0x%08lx type=0x%02x selected=%d\n", 
//        devid, device_type, selected);
```

**⚠️ WARNING:** This will flood the console with output on every SPI transaction!

---

## Summary of Changes

| File | Change | Purpose |
|------|--------|---------|
| `nuttx-config/nsh/defconfig` | Remove `CONFIG_UART0_SERIAL_CONSOLE`, `CONFIG_NSH_USBCONSOLE` | Prevent system output on UART0 |
| `default.px4board` | Remove `CONFIG_BOARD_SERIAL_TEL1` | Prevent MAVLink on UART0 |
| `init/rc.board_defaults` | Add `sleep 2` and `nshterm /dev/ttyS0 &` | Start clean NSH shell after boot |
| `init/rc.board_mavlink` | Keep `mavlink start -d /dev/ttyACM0` | MAVLink on USB |
| `src/init.c` | Comment out debug syslog calls | Prevent debug spam |

---


