# QGroundControl Connection Fix

## Issue Found
The `rc.board_mavlink` file in the build was missing the baud rate and stream configuration. This has been fixed.

## Solution Applied
✅ Updated MAVLink configuration with:
- Baud rate: 115200
- Stream rates configured
- Proper MAVLink mode settings

## Next Steps

### 1. Flash the Updated Firmware

**Option A: UF2 Bootloader (Easiest)**
```bash
# Put board in bootloader mode:
# 1. Hold BOOTSEL button
# 2. Connect USB (or press RESET while holding BOOTSEL)
# 3. Release BOOTSEL
# 4. Board appears as 'RPI-RP2' USB drive

# Then flash:
cd firmware/openfc2040/rsp_2040
cp px4-autopilot/build/raspberrypi_pico_default/raspberrypi_pico_default.uf2 /media/$USER/RPI-RP2/
```

**Option B: picotool (if installed)**
```bash
sudo picotool load -r firmware/openfc2040/rsp_2040/px4-autopilot/build/raspberrypi_pico_default/raspberrypi_pico_default.uf2
```

### 2. Close QGroundControl Before Flashing
- Close QGroundControl completely
- This releases `/dev/ttyACM0` so firmware can boot properly

### 3. Wait for Boot
- After flashing, wait 10-15 seconds for firmware to fully boot
- LED should blink or show activity

### 4. Connect in QGroundControl

1. **Open QGroundControl**
2. **Settings → Comm Links → Add** (or edit existing)
3. **Configure**:
   - **Name**: OpenFC2040
   - **Type**: Serial
   - **Port**: `/dev/ttyACM0` (Linux) or `COMx` (Windows)
   - **Baud Rate**: `115200` ⚠️ **IMPORTANT: Must be 115200**
   - **Flow Control**: None
4. **Click "OK"**
5. **Click "Connect"**

### 5. What to Expect

**If connection works:**
- ✅ Green "Connected" status
- ✅ Heartbeat indicator active
- ✅ Vehicle information displays
- ✅ System status visible

**If still not connecting:**
- Check baud rate is exactly 115200
- Try disconnecting and reconnecting USB
- Check `/dev/ttyACM0` exists: `ls -l /dev/ttyACM0`
- Verify permissions: `groups | grep dialout`

### 6. Verify MAVLink is Running

**Check if device is sending data:**
```bash
# Should see binary data (garbled is normal - it's MAVLink!)
timeout 2 hexdump -C /dev/ttyACM0 | head -5
```

**Or use mavproxy (if installed):**
```bash
mavproxy.py --master=/dev/ttyACM0 --baudrate=115200
# Should see: "Heartbeat from system 1"
```

## Troubleshooting

### "Device or resource busy"
- Close QGroundControl
- Close any other programs using `/dev/ttyACM0`
- Check: `fuser /dev/ttyACM0` or `lsof /dev/ttyACM0`

### "No heartbeat" in QGroundControl
- Verify baud rate is 115200
- Reboot board (disconnect/reconnect USB)
- Check MAVLink started: Look for "Board mavlink" in boot logs (if you have UART console)

### Connection drops
- Use USB 2.0 port (not USB 3.0)
- Use data cable (not charge-only)
- Check USB power (use powered port/hub)

## Current MAVLink Configuration

The firmware now includes:
```bash
mavlink start -d /dev/ttyACM0 -b 115200 -m normal -x -z
mavlink stream -d /dev/ttyACM0 -s HEARTBEAT -r 1
mavlink stream -d /dev/ttyACM0 -s SYS_STATUS -r 5
mavlink stream -d /dev/ttyACM0 -s ATTITUDE -r 50
mavlink stream -d /dev/ttyACM0 -s HIGHRES_IMU -r 50
mavlink stream -d /dev/ttyACM0 -s GLOBAL_POSITION_INT -r 10
mavlink stream -d /dev/ttyACM0 -s VFR_HUD -r 10
mavlink stream -d /dev/ttyACM0 -s BATTERY_STATUS -r 5
```

This ensures QGroundControl receives all necessary telemetry streams.

---

**Status**: ✅ MAVLink configuration fixed and ready to flash

