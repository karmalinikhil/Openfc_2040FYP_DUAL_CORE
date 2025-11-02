# OpenFC2040 SWD Debugger Wiring Guide

## Physical Connection Diagram

```
╔══════════════════════════════════════════════════════════════════╗
║                    PICOPROBE (RP2350 - PICO 2)                  ║
║                                                                  ║
║  [USB]────┐                                                     ║
║           │ (Power + Serial)                                    ║
║           │                                                      ║
║  Pin Functions:                                                  ║
║  ┌────────────────────────────────────────┐                    ║
║  │ GPIO2  (SWCLK) ────────────────────┐   │                    ║
║  │ GPIO3  (SWDIO) ──────────────────┐ │   │                    ║
║  │ GPIO4  (UART TX - optional) ───┐ │ │   │                    ║
║  │ GPIO5  (UART RX - optional) ─┐ │ │ │   │                    ║
║  │ GND    (Ground)  ───────────┐│ │ │ │   │                    ║
║  │ VBUS   (3.3V ref - opt)  ─┐││ │ │ │   │                    ║
║  └────────────────────────────┼┼┼─┼─┼─┼───┘                    ║
║                               │││ │ │ │                         ║
╚═══════════════════════════════╪╪╪═╪═╪═╪═════════════════════════╝
                                │││ │ │ │
                                │││ │ │ │ Jumper Wires
                                │││ │ │ │ (Keep < 15cm, twist pairs)
                                │││ │ │ │
╔═══════════════════════════════╪╪╪═╪═╪═╪═════════════════════════╗
║                    OPENFC2040 (RP2040 - Flight Controller)      ║
║                                │││ │ │ │                         ║
║  SWD Header (usually 3-pin):   │││ │ │ │                         ║
║  ┌─────────────────────────────┼┼┼─┼─┼─┼────┐                   ║
║  │ Pin 1: SWCLK ◄──────────────┘││ │ │ │    │                   ║
║  │ Pin 2: SWDIO ◄────────────────┘│ │ │ │    │                   ║
║  │ Pin 3: GND   ◄──────────────────┘ │ │ │    │                   ║
║  │ Pin 4: 3V3   ◄──────────────────────┘ │ │    │ (VTREF - if present)
║  └──────────────────────────────────────┼─┼────┘                   ║
║                                          │ │                      ║
║  Optional UART Debug (GPIO0/1):          │ │                      ║
║  ┌───────────────────────────────────────┼─┼────┐                   ║
║  │ GPIO0 (TX) ◄──────────────────────────┘ │    │                   ║
║  │ GPIO1 (RX) ◄────────────────────────────┘    │                   ║
║  └──────────────────────────────────────────────┘                   ║
║                                                                  ║
║  [USB]────┐                                                     ║
║           │ (Power + USB Console - being debugged)             ║
║           │                                                      ║
╚══════════════════════════════════════════════════════════════════╝
```

## Pin Mapping Table

| Picoprobe Pin | Function | Wire Color (Suggested) | OpenFC2040 Pin | Notes |
|---------------|----------|------------------------|----------------|-------|
| **GPIO2** | SWCLK | Yellow | **SWCLK** (SWD header) | ⚠️ **CRITICAL** - Clock signal |
| **GPIO3** | SWDIO | Orange | **SWDIO** (SWD header) | ⚠️ **CRITICAL** - Data signal |
| **GND** | Ground | Black | **GND** (SWD header) | ⚠️ **MANDATORY** - Common ground |
| **VBUS/3V3** | Voltage Ref | Red | **3V3** (if available) | Optional - for VTREF |
| GPIO4 | UART TX | Green | GPIO1 (RX) | Optional - UART debug |
| GPIO5 | UART RX | Blue | GPIO0 (TX) | Optional - UART debug |

## Connection Steps

### Step 1: Prepare Picoprobe

1. **Flash Debugprobe Firmware** (if not already done):
   ```bash
   # Put Pico 2 in bootloader mode (hold BOOTSEL, connect USB)
   cp tools/debuggers/debugprobe_on_pico2.uf2 /media/$USER/RPI-RP2/
   # Wait for reboot
   ```

2. **Verify Picoprobe is detected**:
   ```bash
   lsusb | grep "2e8a:000c"
   # Expected output:
   # Bus 001 Device 005: ID 2e8a:000c Raspberry Pi Debug Probe
   ```

### Step 2: Identify SWD Header on OpenFC2040

The OpenFC2040 board should have a **3-pin or 4-pin header** labeled as:
- "SWD"
- "DEBUG"  
- Or near RP2040 chip with labels: SWCLK, SWDIO, GND

**Typical header pinout** (verify with your board!):
```
Pin 1: SWCLK  ○───  (Usually square pad)
Pin 2: SWDIO  ○───
Pin 3: GND    ○───
Pin 4: 3V3    ○───  (Optional - may not be present)
```

**⚠️ IMPORTANT**: Double-check your board schematic or silkscreen labels!

### Step 3: Wire Connection

**MINIMUM REQUIRED** (3 wires):
1. Picoprobe **GPIO2** → OpenFC2040 **SWCLK**
2. Picoprobe **GPIO3** → OpenFC2040 **SWDIO**
3. Picoprobe **GND** → OpenFC2040 **GND**

**RECOMMENDED** (4 wires - add VTREF):
4. Picoprobe **VBUS** (or 3V3 pin) → OpenFC2040 **3V3**

**OPTIONAL** (6 wires - add UART debug):
5. Picoprobe **GPIO4** → OpenFC2040 **GPIO1** (RX)
6. Picoprobe **GPIO5** → OpenFC2040 **GPIO0** (TX)

**Best Practices**:
- ✅ Use **female-to-female jumper wires**
- ✅ Keep wires **short** (< 15cm recommended)
- ✅ **Twist** SWCLK and SWDIO wires together (reduces noise)
- ✅ Avoid breadboards (direct connection only)
- ✅ Use different colors for easy identification
- ❌ Don't connect when boards are powered on (connect first, then power)

### Step 4: Power Up Sequence

1. **Disconnect both boards** from USB
2. **Wire all connections** as above
3. **Double-check wiring** (especially SWCLK/SWDIO - swapping them won't work!)
4. **Connect Picoprobe to PC via USB** (powers up Picoprobe)
5. **Connect OpenFC2040 to PC via USB** (powers up target)
6. **Verify both enumerate**:
   ```bash
   lsusb | grep "2e8a"
   # Should show TWO devices:
   # - 2e8a:000c (Picoprobe)
   # - 2e8a:0005 or 2e8a:000a (OpenFC2040 in bootloader or running firmware)
   ```

### Step 5: Test Connection

```bash
cd firmware/openfc2040/rsp_2040
./debug/start_debug_session.sh
```

**Expected output**:
```
========================================
Checking Dependencies
========================================
[INFO] openocd: Open On-Chip Debugger 0.11.0
[INFO] gdb-multiarch: GNU gdb (Ubuntu ...) ...

========================================
Checking Hardware
========================================
[INFO] ✓ Picoprobe detected (VID:PID = 2e8a:000c)

========================================
Starting OpenOCD
========================================
[INFO] OpenOCD started (PID: 12345)
[INFO] ✓ OpenOCD ready on port 3333

========================================
Debug Session Ready
========================================
```

**If errors occur**, see Troubleshooting section below.

## Troubleshooting

### Error: "Picoprobe NOT detected"

**Symptoms**: `lsusb` doesn't show `2e8a:000c`

**Fixes**:
1. Re-flash Picoprobe firmware:
   ```bash
   # Hold BOOTSEL on Pico 2, connect USB
   cp tools/debuggers/debugprobe_on_pico2.uf2 /media/$USER/RPI-RP2/
   ```
2. Try different USB port
3. Check USB cable (must be data cable, not charge-only)

### Error: "Error: Could not find MEM-AP to control the core"

**Symptoms**: OpenOCD can't communicate with RP2040

**Fixes**:
1. **Check wiring**:
   - SWCLK connected? (Picoprobe GPIO2 → OpenFC2040 SWCLK)
   - SWDIO connected? (Picoprobe GPIO3 → OpenFC2040 SWDIO)
   - Are they swapped? (swap and try again)
   - GND connected?

2. **Check power**:
   - Is OpenFC2040 powered? (LED should be on)
   - Try connecting VTREF (3V3 wire)

3. **Lower SWD speed**:
   - Edit `firmware/openfc2040/rsp_2040/debug/openocd_rp2040.cfg`
   - Change `adapter speed 2000` to `adapter speed 1000` or `500`

4. **Power cycle**:
   ```bash
   # Disconnect both boards from USB
   # Wait 5 seconds
   # Reconnect Picoprobe first, then OpenFC2040
   ```

### Error: "libusb_open() failed with LIBUSB_ERROR_ACCESS"

**Symptoms**: Permission denied accessing USB device

**Fix**:
```bash
# Add udev rules for Picoprobe
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", ATTR{idProduct}=="000c", MODE="0666"' | sudo tee /etc/udev/rules.d/99-picoprobe.rules

# Reload rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Reconnect Picoprobe
```

### Error: "Warn : CMSIS-DAP command CMD_INFO failed"

**Symptoms**: Communication error with Picoprobe

**Fixes**:
1. Use `cmsis_dap_backend hid` (default in our config)
2. If still fails, try `cmsis_dap_backend libusb`:
   - Edit `openocd_rp2040.cfg`
   - Change line to: `cmsis_dap_backend libusb`
3. Install libusb if needed:
   ```bash
   sudo apt install libusb-1.0-0-dev
   ```

### OpenOCD connects but GDB can't connect

**Symptoms**: OpenOCD starts but `target extended-remote :3333` fails

**Fixes**:
1. Check OpenOCD is running:
   ```bash
   ps aux | grep openocd
   netstat -tuln | grep 3333
   ```

2. Check OpenOCD log:
   ```bash
   cat firmware/openfc2040/rsp_2040/debug/openocd.log
   ```

3. Firewall blocking port 3333?
   ```bash
   sudo ufw allow 3333/tcp
   ```

## Verification Tests

Once wiring is complete and OpenOCD connects:

### Test 1: Read RP2040 Chip ID

```bash
# In OpenOCD telnet (telnet localhost 4444):
> mdw 0x40000000 4
# Should show RP2040 hardware registers
```

### Test 2: Halt and Resume

```bash
# In GDB:
(gdb) monitor halt
# CPU should stop
(gdb) monitor resume
# CPU should continue
```

### Test 3: Read Flash Memory

```bash
# In GDB:
(gdb) x/16xw 0x10000000
# Should show firmware code at flash base address
```

### Test 4: Read SRAM

```bash
# In GDB:
(gdb) x/16xw 0x20000000
# Should show data in SRAM
```

If all tests pass: ✅ **SWD debugging is working!**

## Next Steps

Once SWD is confirmed working:

1. **Capture PX4 boot sequence** → See where console initialization happens
2. **Set breakpoints in USB code** → Find where data gets corrupted
3. **Compare with working firmware** → Check init order differences
4. **Test fixes incrementally** → Apply changes and verify via SWD

See main debug README for detailed debugging workflows.

---

**Wiring Visual Reference**: https://datasheets.raspberrypi.com/debug/debug-connector-specification.pdf
