# OpenFC2040 SWD Debugging Guide

This directory contains tools and configuration for debugging the OpenFC2040 PX4 firmware using SWD (Serial Wire Debug) via a Picoprobe debugger.

## Hardware Setup

### Required Hardware
1. **OpenFC2040 Board** - Your flight controller (RP2040-based)
2. **Picoprobe** - Raspberry Pi Pico 2 (RP2350) flashed with debug firmware
3. **USB Cables** - Two micro-USB cables (one for each board)
4. **Jumper Wires** - 4-5 female-to-female jumpers

### Picoprobe Preparation

1. **Flash Picoprobe Firmware**:
   ```bash
   # The debugprobe firmware is already built in this repo
   # File: tools/debuggers/debugprobe_on_pico2.uf2
   
   # Put Pico 2 in bootloader mode (hold BOOTSEL + connect USB)
   # Copy firmware to RPI-RP2 drive
   cp tools/debuggers/debugprobe_on_pico2.uf2 /media/$USER/RPI-RP2/
   ```

2. **Verify Picoprobe**:
   ```bash
   # After flashing, reconnect Picoprobe
   # It should enumerate as:
   lsusb | grep "2e8a:000c"
   # Expected: "Raspberry Pi Debug Probe"
   ```

### Wiring Diagram

```
Picoprobe (RP2350)         OpenFC2040 (RP2040)
==================         ===================
GPIO2 (SWCLK)      ------> SWCLK (SWD header pin 1)
GPIO3 (SWDIO)      ------> SWDIO (SWD header pin 2)
GND                ------> GND   (SWD header pin 3)
VBUS (optional)    ------> 3V3   (for VTREF, if needed)

Optional UART Debug:
GPIO4 (TX)         ------> GPIO1 (RX) on OpenFC2040
GPIO5 (RX)         ------> GPIO0 (TX) on OpenFC2040
```

**Important Notes**:
- **SWCLK/SWDIO** are critical - ensure solid connections
- **GND** is mandatory for proper signal reference
- **VTREF** (3V3) may be needed if OpenOCD reports voltage issues
- Keep wires **short** (< 15cm) and **twisted** (SWCLK+SWDIO together)
- Avoid breadboards - use direct jumper connections

## Software Setup

### Install Dependencies

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install openocd gdb-multiarch

# Verify installation
openocd --version  # Should be 0.11.0 or later
gdb-multiarch --version
```

### File Structure

```
debug/
├── README.md                   # This file
├── openocd_rp2040.cfg          # OpenOCD configuration for RP2040
├── start_debug_session.sh      # Launch OpenOCD + GDB
├── stop_debug_session.sh       # Stop OpenOCD
├── gdb_init.gdb                # Auto-generated GDB script
├── openocd.log                 # OpenOCD output log
└── openocd.pid                 # OpenOCD process ID
```

## Usage

### Quick Start

1. **Wire hardware** as shown in wiring diagram above
2. **Connect both boards** via USB (Picoprobe + OpenFC2040)
3. **Start debug session** (from repo root):
   ```bash
   cd firmware/openfc2040/rsp_2040
   ./debug/start_debug_session.sh
   ```

4. **Launch GDB** (in another terminal, from repo root):
   ```bash
   cd firmware/openfc2040/rsp_2040
   gdb-multiarch -x debug/gdb_init.gdb px4-autopilot/build/raspberrypi_pico_default/raspberrypi_pico_default.elf
   ```

### GDB Debugging Commands

Once in GDB prompt `(gdb)`:

```gdb
# Basic Control
continue            # Resume execution (let firmware run)
Ctrl+C              # Pause execution
step                # Step one line (into functions)
next                # Step one line (over functions)
finish              # Run until current function returns

# Breakpoints
break main                          # Break at main()
break board_config.h:100            # Break at specific line
break rp2040_usb_init               # Break at function
info breakpoints                    # List all breakpoints
delete 1                            # Delete breakpoint #1

# Inspection
bt                              # Backtrace (call stack)
info threads                    # Show all NuttX tasks
thread 2                        # Switch to thread 2
print variable_name             # Print variable
print /x variable_name          # Print in hex
print *struct_ptr               # Dereference pointer

# Memory Inspection
x/16xw 0x20000000              # Dump 16 words from SRAM start
x/s 0x20001000                 # Dump string at address
x/i $pc                        # Disassemble at program counter

# Registers
info registers                 # Show all registers
print /x $pc                   # Print program counter
print /x $sp                   # Print stack pointer

# NuttX-specific
info threads                   # Show all tasks
thread apply all bt            # Backtrace all threads
```

### Common Debugging Scenarios

#### Scenario 1: Capture Boot Sequence

```gdb
# In GDB:
(gdb) break rp2040_usb_init        # Break when USB initializes
(gdb) break cdcacm_bind            # Break at CDC ACM driver binding
(gdb) continue                     # Let it boot
# When it hits breakpoint:
(gdb) bt                           # See call stack
(gdb) info locals                  # See local variables
(gdb) continue                     # Continue to next breakpoint
```

#### Scenario 2: Inspect Console Output

```gdb
# Find where console writes happen
(gdb) break uart_write
(gdb) break usb_write
(gdb) continue
# When it breaks:
(gdb) print buffer                 # See what's being written
(gdb) x/s buffer                   # Print as string
```

#### Scenario 3: Diagnose Crash/Hang

```gdb
# If firmware hangs or crashes:
Ctrl+C                             # Halt execution
(gdb) bt                           # Where did it hang?
(gdb) info threads                 # Which threads are running?
(gdb) thread apply all bt          # Backtrace all threads
```

### Stopping Debug Session

```bash
# Stop OpenOCD when done
./debug/stop_debug_session.sh
```

## Troubleshooting

### OpenOCD Connection Issues

**Error**: `Error: libusb_open() failed with LIBUSB_ERROR_ACCESS`
```bash
# Add udev rules for Picoprobe
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", ATTR{idProduct}=="000c", MODE="0666"' | sudo tee /etc/udev/rules.d/99-picoprobe.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
# Reconnect Picoprobe
```

**Error**: `Error: Could not find MEM-AP to control the core`
```bash
# Check wiring (especially SWDIO/SWCLK)
# Try lowering SWD speed in openocd_rp2040.cfg:
adapter speed 1000  # Instead of 2000
```

**Error**: `Error: Target not examined yet`
```bash
# Power cycle OpenFC2040
# Ensure VTREF (3V3) wire is connected
# Try: monitor reset halt
```

### GDB Connection Issues

**Error**: `Connection refused` when connecting to `:3333`
```bash
# Check if OpenOCD is running
ps aux | grep openocd
# Check OpenOCD log
cat debug/openocd.log
# Restart OpenOCD
./debug/stop_debug_session.sh
./debug/start_debug_session.sh
```

### Firmware Debugging Tips

1. **Console Output Goes to UART?**
   - Set breakpoint at `uart_putc` or `uart_write`
   - Inspect which UART is being used

2. **Console Output Goes to USB?**
   - Set breakpoint at `cdcacm_write` or `usbdev_write`
   - Check buffer contents

3. **Firmware Crashes Early**:
   - Break at `_start` (very first instruction)
   - Step through boot process
   - Watch for hardfaults

4. **Need to Flash Different Firmware**:
   ```bash
   # Via OpenOCD (when connected):
   (gdb) monitor reset halt
   (gdb) monitor flash write_image erase /path/to/firmware.bin 0x10000000
   (gdb) monitor reset run
   ```

## Advanced: Manual OpenOCD

If you need more control:

```bash
# Start OpenOCD manually
openocd -f debug/openocd_rp2040.cfg

# In another terminal, connect via telnet
telnet localhost 4444

# OpenOCD commands
> reset halt
> reg
> mdw 0x20000000 16
> flash write_image erase firmware.bin 0x10000000
> reset run
> exit
```

## Next Steps

Once SWD debugging is working:

1. **Capture boot logs** - See where USB init happens
2. **Compare with working firmware** - Check initialization order
3. **Identify USB console issue** - Find where data gets corrupted
4. **Test fixes** - Apply changes and verify via SWD

## References

- [OpenOCD Documentation](https://openocd.org/doc/html/index.html)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [Picoprobe Project](https://github.com/raspberrypi/picoprobe)
- [GDB Cheat Sheet](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf)
