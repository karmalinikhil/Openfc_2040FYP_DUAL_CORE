# QGroundControl Connection Guide

**Connecting OpenFC2040 to QGroundControl via USB**

---

## Quick Connection Steps

### 1. Install QGroundControl

**Linux**:
```bash
# Download from: https://github.com/mavlink/qgroundcontrol/releases
# Or install via snap:
sudo snap install qgroundcontrol
```

**Windows/Mac**: Download from https://qgroundcontrol.com/

### 2. Connect Board via USB

1. **Power on OpenFC2040** via USB cable
2. **Wait 5-10 seconds** for firmware to boot
3. **Check USB device**:
   ```bash
   ls -l /dev/ttyACM*
   # Should show: /dev/ttyACM0
   
   lsusb | grep -i "px4\|openfc\|raspberry"
   # Should show USB device
   ```

### 3. Connect in QGroundControl

1. **Open QGroundControl**
2. **Click "Q" icon** (top left) → **"Vehicle Setup"**
3. **Go to "Comm Links"** (or "Settings" → "Comm Links")
4. **Click "Add"** to create new connection
5. **Configure**:
   - **Name**: OpenFC2040
   - **Type**: Serial
   - **Port**: `/dev/ttyACM0` (Linux) or `COMx` (Windows)
   - **Baud Rate**: `115200`
   - **Flow Control**: None
6. **Click "OK"** and **"Connect"**

---

## Expected Behavior

### On Successful Connection

1. **QGroundControl Status**:
   - Top bar shows "Connected" (green)
   - Vehicle icon appears
   - Heartbeat indicator active

2. **Vehicle Information**:
   - Firmware version displayed
   - Hardware type: "OpenFC2040" or "Raspberry Pi Pico"
   - System status visible

3. **Available Data** (depending on sensors):
   - **Battery**: Voltage/current (if ADC working)
   - **Attitude**: Roll/Pitch/Yaw (if IMU working)
   - **GPS**: Position (if GPS connected)
   - **System Status**: CPU load, memory usage

### What You Might See

**If sensors not working yet**:
- ⚠️ Yellow warnings for missing sensors (IMU, GPS, etc.)
- ✅ System status and heartbeat should work
- ✅ Basic vehicle information should display

**If everything works**:
- ✅ All sensors green
- ✅ Full telemetry data
- ✅ Can view parameters, calibrate sensors, etc.

---

## MAVLink Configuration

The firmware is configured with:

**Device**: `/dev/ttyACM0` (USB CDC-ACM)  
**Baud Rate**: 115200  
**Protocol**: MAVLink v2  
**Streams Enabled**:
- HEARTBEAT (1 Hz)
- SYS_STATUS (5 Hz)
- ATTITUDE (50 Hz)
- HIGHRES_IMU (50 Hz)
- GLOBAL_POSITION_INT (10 Hz)
- VFR_HUD (10 Hz)
- BATTERY_STATUS (5 Hz)

**Configuration File**: `board/init/rc.board_mavlink`

---

## Troubleshooting

### QGroundControl Doesn't Detect Board

1. **Check USB connection**:
   ```bash
   lsusb
   dmesg | tail -20 | grep -i "usb\|tty\|acm"
   ```

2. **Check permissions**:
   ```bash
   ls -l /dev/ttyACM0
   # Should be readable by your user or dialout group
   
   # If not:
   sudo usermod -a -G dialout $USER
   # Logout and login again
   ```

3. **Try different USB port/cable**
   - Use USB 2.0 port (not USB 3.0)
   - Use data cable (not charge-only)

### Connection Drops or Unstable

1. **Check baud rate** matches (115200)
2. **Reduce stream rates** in `rc.board_mavlink` if needed
3. **Check USB power** - use powered USB port or hub

### No Data Received

1. **Verify MAVLink is running**:
   - Check if `/dev/ttyACM0` exists
   - Check if data is being sent (you saw garbled output earlier - that's MAVLink!)

2. **Check QGroundControl connection settings**:
   - Port must be `/dev/ttyACM0` (Linux)
   - Baud rate must be 115200
   - Flow control: None

3. **Monitor MAVLink traffic**:
   ```bash
   # Install mavlink tools
   pip3 install pymavlink
   
   # Monitor MAVLink messages
   mavproxy.py --master=/dev/ttyACM0 --baudrate=115200
   ```

### Board Shows but No Sensor Data

This is **expected** if:
- IMU driver not ported yet (LSM6DS3)
- GPS not connected
- Sensors not calibrated

**What should work**:
- ✅ Heartbeat
- ✅ System status
- ✅ Battery voltage (if ADC configured correctly)
- ✅ Basic vehicle information

---

## Testing MAVLink Connection

### Method 1: QGroundControl

1. Connect as described above
2. Check "Vehicle Setup" → "Summary"
3. Look for:
   - Firmware version
   - Hardware type
   - System status

### Method 2: Command Line (mavproxy)

```bash
# Install mavproxy
pip3 install mavproxy

# Connect
mavproxy.py --master=/dev/ttyACM0 --baudrate=115200

# You should see:
# MAV> Connected to /dev/ttyACM0
# MAV> Heartbeat from system 1
```

### Method 3: Check Raw Data

```bash
# View raw MAVLink data (will look garbled)
hexdump -C /dev/ttyACM0 | head -20

# Or use cat (will show binary data)
cat /dev/ttyACM0 | od -A x -t x1z -v | head -20
```

---

## Next Steps After Connection

Once QGroundControl connects:

1. **Check Vehicle Summary**:
   - Firmware version
   - Hardware info
   - System status

2. **Review Parameters**:
   - Settings → Parameters
   - Search for relevant parameters
   - Check battery voltage divider settings

3. **Monitor Data**:
   - Flight View → Watch attitude, battery, etc.
   - Check which sensors are working

4. **Calibrate Sensors** (when drivers are ready):
   - Vehicle Setup → Sensors
   - Follow calibration wizard

---

## MAVLink Stream Configuration

Current streams in `rc.board_mavlink`:

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

**To modify streams** (after UART console works):
```bash
# In NSH shell:
mavlink stream -d /dev/ttyACM0 -s HEARTBEAT -r 10  # Increase rate
mavlink stop -d /dev/ttyACM0                       # Stop MAVLink
mavlink start -d /dev/ttyACM0 -b 115200             # Restart
```

---

## Expected MAVLink Messages

When connected, you should see:

1. **HEARTBEAT** (every 1 second)
   - System ID: 1
   - Component ID: 1
   - Type: MAV_TYPE_QUADROTOR

2. **SYS_STATUS** (every 0.2 seconds)
   - Battery voltage
   - System load
   - Sensor status flags

3. **ATTITUDE** (if IMU working)
   - Roll, Pitch, Yaw
   - Angular velocities

4. **BATTERY_STATUS** (if ADC working)
   - Voltage
   - Current
   - Remaining percentage

---

## Verification Checklist

After connecting to QGroundControl:

- [ ] QGroundControl shows "Connected" (green)
- [ ] Heartbeat indicator is active
- [ ] Vehicle information displays (firmware version, hardware type)
- [ ] System status shows (even if sensors missing)
- [ ] Can view parameters
- [ ] Can see basic telemetry data

---

## Common Issues

### "No Heartbeat" Error

**Cause**: MAVLink not starting or wrong port  
**Fix**: 
- Verify `/dev/ttyACM0` exists
- Check baud rate is 115200
- Reboot board

### "Vehicle Not Responding"

**Cause**: USB connection issue or firmware crash  
**Fix**:
- Reconnect USB
- Re-flash firmware
- Check LED status (should blink)

### Garbled Data (Expected!)

**Note**: The garbled output you saw on `/dev/ttyACM0` is **normal** - it's MAVLink binary data!  
**QGroundControl** will decode it properly.

---

## Summary

**Connection Method**: USB (`/dev/ttyACM0`)  
**Baud Rate**: 115200  
**Protocol**: MAVLink v2  
**Status**: ✅ Configured and ready

**Next**: Connect board, open QGroundControl, and check if vehicle is detected!

---

**Last Updated**: November 10, 2025

