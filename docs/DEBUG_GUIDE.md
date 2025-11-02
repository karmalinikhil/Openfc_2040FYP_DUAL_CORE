# SWD Debugging Guide

**Complete setup for debugging OpenFC2040 firmware with Picoprobe/Debugprobe**

> 🐛 **Purpose**: Use real debugger to step through code, set breakpoints, inspect variables  
> 🔧 **Hardware Required**: Picoprobe or Debugprobe (another RP2040/RP2350 board)

---

## Table of Contents

- [Overview](#overview)
- [Hardware Options](#hardware-options)
- [Software Requirements](#software-requirements)
- [Hardware Setup](#hardware-setup)
- [Software Setup](#software-setup)
- [Starting a Debug Session](#starting-a-debug-session)
- [GDB Commands](#gdb-commands)
- [Troubleshooting](#troubleshooting)
- [Alternative Debugging Methods](#alternative-debugging-methods)

---

## Overview

### What is SWD?

**SWD (Serial Wire Debug)** is ARM's 2-wire debugging protocol:
- **SWCLK**: Clock signal
- **SWDIO**: Bidirectional data signal

Unlike JTAG (4-5 wires), SWD only needs 2 pins + GND.

### Debug Workflow

```
Your PC
   ↓
OpenOCD (connects to Picoprobe via USB)
   ↓
Picoprobe (RP2040 running debug firmware)
   ↓ (SWD: SWCLK + SWDIO)
OpenFC2040 Target (RP2040 running your PX4 firmware)
```

### What You Can Do

- ✅ Set breakpoints (pause execution at specific line)
- ✅ Step through code line-by-line
- ✅ Inspect variables and memory
- ✅ View CPU registers
- ✅ Halt/resume firmware
- ✅ Flash firmware via SWD
- ✅ View call stack
- ✅ Watch expressions

---

## Hardware Options

### Option 1: Raspberry Pi Pico as Picoprobe (Recommended)

**What you need**:
- Raspberry Pi Pico (RP2040) - $4
- USB Micro cable
- 3x female-to-female jumper wires

**Pros**:
- Cheap ($4)
- Easy to find
- Well-documented

**Cons**:
- Slightly slower than Debugprobe

### Option 2: Raspberry Pi Pico 2 as Debugprobe

**What you need**:
- Raspberry Pi Pico 2 (RP2350) - $5
- USB Micro cable
- 3x jumper wires

**Pros**:
- Faster debugging
- More features
- Native RISC-V support (not needed for RP2040)

**Cons**:
- Slightly more expensive
- Firmware slightly different

### Option 3: Dedicated Debug Probe

**Options**:
- Black Magic Probe (~$60)
- J-Link EDU Mini (~$20)
- ST-Link V2 clone (~$3-5, may not work well)

**Pros**:
- Professional-grade
- Very fast

**Cons**:
- More expensive
- Overkill for RP2040

**Recommendation**: Use Raspberry Pi Pico (Option 1) - best value!

---

## Software Requirements

### On Linux (Ubuntu 20.04+)

#### 1. Install OpenOCD

```bash
# Install from package manager
sudo apt update
sudo apt install openocd

# Verify version (need 0.11.0+)
openocd --version
# Should show: Open On-Chip Debugger 0.11.0 or newer
```

**What is OpenOCD?**  
Open On-Chip Debugger - connects GDB to hardware debug probes.

#### 2. Install GDB for ARM

```bash
# Install GDB multiarch
sudo apt install gdb-multiarch

# Verify
gdb-multiarch --version
# Should show: GNU gdb with ARM support
```

**Why multiarch?**  
Standard GDB only debugs x86. We need ARM version for RP2040 (Cortex-M0+).

#### 3. Install USB Permissions

```bash
# Create udev rules for Picoprobe
sudo tee /etc/udev/rules.d/99-picoprobe.rules > /dev/null << 'EOF'
# Raspberry Pi Picoprobe
SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", ATTR{idProduct}=="0004", MODE="0666"
SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", ATTR{idProduct}=="000c", MODE="0666"
EOF

# Reload rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Add user to plugdev group
sudo usermod -aG plugdev $USER

# Log out and back in for group to take effect
```

**Why needed?**  
Without this, only root can access USB devices. This allows normal users to connect to Picoprobe.

---

## Hardware Setup

### Step 1: Flash Picoprobe Firmware

#### Download Firmware

**For Raspberry Pi Pico (RP2040)**:
- Pre-built firmware already in: `tools/debuggers/debugprobe_on_pico2.uf2`
- Or download: https://github.com/raspberrypi/picoprobe/releases

**For Raspberry Pi Pico 2 (RP2350)**:
- Use: `tools/debuggers/debugprobe_on_pico2.uf2`

#### Flash to Pico

1. **Hold BOOTSEL** button on Raspberry Pi Pico
2. **Connect USB** cable to PC
3. **Release BOOTSEL** → Pico appears as USB drive `RPI-RP2`
4. **Copy firmware**:
   ```bash
   cp tools/debuggers/debugprobe_on_pico2.uf2 /media/$USER/RPI-RP2/
   ```
5. **Wait** for Pico to reboot
6. **Verify** Picoprobe detected:
   ```bash
   lsusb | grep "Picoprobe"
   # Should see: "Raspberry Pi Picoprobe"
   ```

### Step 2: Wire Picoprobe to OpenFC2040

**Important**: Use SHORT wires (< 10cm) for reliable debugging!

| Picoprobe (Debug) | OpenFC2040 (Target) | Wire Color (suggestion) |
|-------------------|---------------------|-------------------------|
| GND | GND | Black |
| GP2 (SWCLK) | SWCLK | Yellow |
| GP3 (SWDIO) | SWDIO | Orange |

**Picoprobe Pinout**:
```
Raspberry Pi Pico (Picoprobe)
┌─────────────────────┐
│  USB    │           │
│ ┌───┐   │           │
│ └───┘   │           │
│         │           │
│  GP0 ●  │  ● VBUS   │
│  GP1 ●  │  ● VSYS   │
│  GND ●  │  ● GND    │ ← Connect to target GND
│  GP2 ●  │  ● 3V3_EN │ ← SWCLK (to target)
│  GP3 ●  │  ● 3V3    │ ← SWDIO (to target)
│  GP4 ●  │  ● VREF   │
│  ...    │  ...      │
└─────────────────────┘
```

**OpenFC2040 Debug Header** (see `docs/PIN_CONNECTIONS.md`):
- Pin 1: GND
- Pin 2: SWCLK
- Pin 3: SWDIO

**⚠️ Important**:
- Do NOT connect VCC/3V3 between boards!
- Only GND, SWCLK, SWDIO
- OpenFC2040 powered separately via USB

### Step 3: Verify Connection

```bash
# Connect both boards to PC via USB
# - Picoprobe: via USB (debug adapter)
# - OpenFC2040: via USB (powers target)

# Test OpenOCD connection
cd firmware/openfc2040/rsp_2040/debug
openocd -f openocd_rp2040.cfg

# Should see:
# Info : Listening on port 3333 for gdb connections
# Info : rp2040.core0: hardware has 4 breakpoints, 2 watchpoints
```

If successful, **Ctrl+C** to stop. If errors, see [Troubleshooting](#troubleshooting).

---

## Software Setup

### OpenOCD Configuration

Configuration file: `firmware/openfc2040/rsp_2040/debug/openocd_rp2040.cfg`

```tcl
# Picoprobe interface
source [find interface/cmsis-dap.cfg]

# RP2040 target
source [find target/rp2040.cfg]

# Set adapter speed
adapter speed 5000

# Reset configuration
reset_config srst_only
```

**Already created and tested!** No changes needed.

### GDB Configuration (Optional)

Create `~/.gdbinit`:
```gdb
# Pretty printing
set print pretty on
set print array on

# Auto-load local .gdbinit files
set auto-load safe-path /

# TUI mode by default
# tui enable
```

---

## Starting a Debug Session

### Method 1: Using Helper Scripts (Recommended)

#### Start OpenOCD (Terminal 1)

```bash
cd firmware/openfc2040/rsp_2040/debug
./start_debug_session.sh

# Output:
# Starting OpenOCD for RP2040 debugging...
# OpenOCD listening on port 3333
# Ready for GDB connection!
```

**Leave this terminal running!**

#### Start GDB (Terminal 2)

```bash
cd firmware/openfc2040/rsp_2040

# Launch GDB with firmware ELF file
gdb-multiarch px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.elf

# In GDB prompt:
(gdb) target remote :3333          # Connect to OpenOCD
(gdb) monitor reset halt           # Reset and halt CPU
(gdb) load                          # Flash firmware (optional)
(gdb) break main                    # Set breakpoint at main()
(gdb) continue                      # Run until breakpoint
```

### Method 2: Manual OpenOCD Start

```bash
cd firmware/openfc2040/rsp_2040/debug

# Start OpenOCD manually
openocd -f openocd_rp2040.cfg

# In another terminal:
gdb-multiarch ../px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.elf
(gdb) target remote :3333
```

---

## GDB Commands

### Essential Commands

| Command | Shortcut | Description | Example |
|---------|----------|-------------|---------|
| `break` | `b` | Set breakpoint | `b main` |
| `continue` | `c` | Resume execution | `c` |
| `next` | `n` | Step over (don't enter functions) | `n` |
| `step` | `s` | Step into functions | `s` |
| `finish` | `fin` | Run until current function returns | `fin` |
| `print` | `p` | Print variable | `p my_var` |
| `backtrace` | `bt` | Show call stack | `bt` |
| `info locals` | - | Show local variables | `info locals` |
| `list` | `l` | Show source code | `l` |
| `quit` | `q` | Exit GDB | `q` |

### Advanced Commands

| Command | Description | Example |
|---------|-------------|---------|
| `watch` | Set watchpoint (break when variable changes) | `watch my_var` |
| `info breakpoints` | List all breakpoints | `info b` |
| `delete` | Delete breakpoint | `delete 1` |
| `disable` | Disable breakpoint | `disable 1` |
| `set variable` | Modify variable | `set var my_var = 42` |
| `monitor reset` | Reset target via OpenOCD | `monitor reset halt` |
| `x/10x 0x20000000` | Examine memory (10 words hex) | `x/10x $sp` |

### Example Debug Session

```gdb
# Start GDB
$ gdb-multiarch px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.elf

# Connect to target
(gdb) target remote :3333
Remote debugging using :3333

# Reset and halt
(gdb) monitor reset halt
target halted due to debug-request, current mode: Thread
xPSR: 0x01000000 pc: 0x000000ee msp: 0x20042000

# Load new firmware (optional)
(gdb) load
Loading section .boot2, size 0x100 lma 0x10000000
Loading section .text, size 0x3a840 lma 0x10000100
...
Transfer rate: 45 KB/sec, 2048 bytes/write.

# Set breakpoint at main
(gdb) break main
Breakpoint 1 at 0x10001234: file main.c, line 42.

# Run
(gdb) continue
Continuing.

Breakpoint 1, main () at main.c:42
42      int ret = px4_platform_init();

# Show local variables
(gdb) info locals
ret = 0

# Step through code
(gdb) next
45      if (ret < 0) {

# Print variable
(gdb) print ret
$1 = 0

# Continue execution
(gdb) continue
```

### Debugging Crashes

If firmware crashes:

```gdb
# In GDB, target is halted at crash point
(gdb) backtrace
#0  0x100023a4 in some_function () at file.c:123
#1  0x10001856 in caller_function () at main.c:45
#2  0x10001234 in main () at main.c:42

# Examine why it crashed
(gdb) frame 0
(gdb) info locals
(gdb) print $pc    # Program counter
(gdb) print $sp    # Stack pointer
```

---

## Troubleshooting

### "Error: unable to find a matching CMSIS-DAP device"

**Cause**: Picoprobe not detected

**Fix**:
```bash
# Check if Picoprobe connected
lsusb | grep -i "picoprobe\|cmsis"

# If not found:
# 1. Reconnect Picoprobe USB
# 2. Verify Picoprobe firmware flashed correctly
# 3. Check udev rules installed:
ls /etc/udev/rules.d/99-picoprobe.rules
```

### "Error: Failed to connect multidrop rp2040.dap0"

**Cause**: SWD wiring issue

**Fix**:
1. Verify wiring:
   - GND connected
   - SWCLK connected to correct pin
   - SWDIO connected to correct pin
2. Use SHORT wires (< 10cm)
3. Ensure OpenFC2040 powered on
4. Try lower adapter speed in `openocd_rp2040.cfg`:
   ```tcl
   adapter speed 1000  # Slower speed (was 5000)
   ```

### "Cannot access memory at address 0x..."

**Cause**: Target not halted or crashed

**Fix**:
```gdb
(gdb) monitor reset halt
(gdb) info registers  # Check if PC valid
```

### "Remote connection closed"

**Cause**: OpenOCD crashed or stopped

**Fix**:
1. Restart OpenOCD in terminal 1
2. Reconnect GDB:
   ```gdb
   (gdb) target remote :3333
   ```

### "No symbol table loaded"

**Cause**: Debugging without ELF file

**Fix**:
```bash
# Must specify ELF file (not .uf2!):
gdb-multiarch px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.elf
```

### Slow Debugging

**Cause**: Adapter speed too slow or USB issues

**Fix**:
1. Increase adapter speed in `openocd_rp2040.cfg`:
   ```tcl
   adapter speed 10000  # Faster (if stable)
   ```
2. Use USB 2.0 port (not USB 3.0)
3. Remove USB hubs

---

## Alternative Debugging Methods

### 1. UART Console Debugging

**When to use**: Quick printf-style debugging

```c
// In your code:
#include <px4_platform_common/log.h>

PX4_INFO("Debug: value = %d", my_variable);
```

**View on console**:
```bash
picocom -b 115200 /dev/ttyUSB0

# See debug messages in real-time
```

**Pros**: Simple, no special hardware  
**Cons**: Can't set breakpoints, inspect memory

### 2. LED Blinking Debug

**When to use**: Quick status indication

```c
// Blink LED to show progress
#define LED_PIN 13

void debug_blink(int times) {
    for (int i = 0; i < times; i++) {
        gpio_write(LED_PIN, 0);  // LED ON (active LOW)
        usleep(100000);          // 100ms
        gpio_write(LED_PIN, 1);  // LED OFF
        usleep(100000);
    }
}

// In code:
debug_blink(3);  // 3 blinks = reached this point
```

**Pros**: Works without console  
**Cons**: Limited information

### 3. GDB Logging

**When to use**: Capture debug session for analysis

```bash
# Start GDB with logging
gdb-multiarch -ex "set logging on" \
  -ex "set logging file debug.log" \
  px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.elf

# All GDB output saved to debug.log
```

### 4. OpenOCD Telnet

**When to use**: Send commands to OpenOCD

```bash
# OpenOCD running in terminal 1

# Terminal 2: Connect via telnet
telnet localhost 4444

# OpenOCD commands:
> reset halt
> mdw 0x20000000 10    # Read memory
> halt
> resume
```

---

## Tips and Best Practices

### 1. Always Use ELF Files

**Bad**:
```bash
gdb-multiarch build/firmware.uf2  # NO DEBUG SYMBOLS!
```

**Good**:
```bash
gdb-multiarch build/firmware.elf  # HAS DEBUG SYMBOLS!
```

### 2. Enable Debug Build

For better debugging, build with debug symbols:

```bash
# Edit CMakeLists.txt or defconfig
# Enable: -g -O0 (no optimization)

# Or set in build command:
make raspberrypi_pico_minimal VERBOSE=1
```

**Note**: Debug builds are LARGER and SLOWER. Use minimal builds for debugging.

### 3. Use TUI Mode

GDB Text User Interface shows source code:

```gdb
(gdb) tui enable

# Now you see:
# ┌─ Source ─────────────────┐
# │ 42: int ret = 0;          │
# │ 43: ret = init();         │
# │ 44: return ret;           │
# └───────────────────────────┘
# (gdb)
```

**Toggle**: `Ctrl+X Ctrl+A`

### 4. Save Breakpoints

```gdb
# Save breakpoints to file
(gdb) save breakpoints breakpoints.txt

# Load later
(gdb) source breakpoints.txt
```

### 5. Use Watchpoints for Hard Bugs

```gdb
# Break when variable changes
(gdb) watch my_variable

# Break when memory location changes
(gdb) watch *(int*)0x20001000
```

---

## Quick Reference Card

```
┌─────────────────────────────────────────────────────────┐
│ OpenOCD + GDB Quick Reference                            │
├─────────────────────────────────────────────────────────┤
│ TERMINAL 1 (OpenOCD):                                    │
│   cd firmware/openfc2040/rsp_2040/debug                  │
│   ./start_debug_session.sh                               │
│                                                           │
│ TERMINAL 2 (GDB):                                        │
│   gdb-multiarch px4-autopilot/build/.../firmware.elf     │
│   (gdb) target remote :3333                              │
│   (gdb) monitor reset halt                               │
│   (gdb) load                   # Flash firmware          │
│   (gdb) break main             # Set breakpoint          │
│   (gdb) continue               # Run                     │
│                                                           │
│ COMMON GDB COMMANDS:                                     │
│   n        Step over (next line)                         │
│   s        Step into function                            │
│   c        Continue execution                            │
│   bt       Backtrace (call stack)                        │
│   p var    Print variable                                │
│   info locals   Show all local variables                 │
│   list     Show source code                              │
│   q        Quit GDB                                      │
│                                                           │
│ OPENOCD MONITOR COMMANDS:                                │
│   monitor reset halt    Reset and halt CPU               │
│   monitor reset         Reset and run                    │
│   monitor halt          Halt execution                   │
│   monitor resume        Resume execution                 │
│                                                           │
│ WIRING:                                                   │
│   Picoprobe GP2 (SWCLK) → OpenFC2040 SWCLK               │
│   Picoprobe GP3 (SWDIO) → OpenFC2040 SWDIO               │
│   Picoprobe GND         → OpenFC2040 GND                 │
│                                                           │
│ TROUBLESHOOTING:                                          │
│   - Use SHORT wires (< 10cm)                             │
│   - Both boards powered separately                       │
│   - Check udev rules: /etc/udev/rules.d/99-picoprobe.rules │
│   - Verify Picoprobe: lsusb | grep Picoprobe             │
└─────────────────────────────────────────────────────────┘
```

---

## Resources

### Documentation
- **OpenOCD Manual**: https://openocd.org/doc/html/index.html
- **GDB Manual**: https://sourceware.org/gdb/documentation/
- **Picoprobe**: https://github.com/raspberrypi/picoprobe
- **RP2040 Datasheet**: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf

### Files in This Project
- OpenOCD config: `firmware/openfc2040/rsp_2040/debug/openocd_rp2040.cfg`
- Start script: `firmware/openfc2040/rsp_2040/debug/start_debug_session.sh`
- Wiring guide: `firmware/openfc2040/rsp_2040/debug/WIRING_GUIDE.md`

---

**Happy Debugging!** 🐛

For more help, see:
- [README.md](../README.md) - Main project documentation
- [PIN_CONNECTIONS.md](PIN_CONNECTIONS.md) - Hardware pin mappings
- [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) - Architecture overview
