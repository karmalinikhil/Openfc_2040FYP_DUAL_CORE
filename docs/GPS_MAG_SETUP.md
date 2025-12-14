# GPS & Magnetometer Setup Guide for OpenFC2040

This document describes the hardware connections and software configuration for external GPS and magnetometer sensors (like u-blox NEO-M8N, NEO-6M/7M/8M series) on the OpenFC2040 flight controller.

## Hardware Overview

The OpenFC2040 board provides a dedicated JST-GH 6-pin connector for external GPS and magnetometer modules. The GPS communicates via UART, while the magnetometer uses I2C.

> **Important:** The GPS and magnetometer are **optional external modules**. The board will boot and operate normally even if no GPS/Mag module is connected. The drivers start in non-blocking mode and will automatically detect devices when connected.

### Pin Connections (JST-GH 6-pin Connector)

| Pin | Signal    | RP2040 GPIO | Function              |
|-----|-----------|-------------|-----------------------|
| 1   | VCC       | -           | 3.3V Power            |
| 2   | GPS_TX    | GPIO4       | UART1 TX → /dev/ttyS1 |
| 3   | GPS_RX    | GPIO5       | UART1 RX              |
| 4   | MAG_SDA   | GPIO6       | I2C1 SDA (Bus 1)      |
| 5   | MAG_SCL   | GPIO7       | I2C1 SCL              |
| 6   | GND       | -           | Ground                |


### Supported Magnetometer Chips

The firmware auto-detects the following magnetometer chips:

| Chip      | I2C Address | Common On                    |
|-----------|-------------|------------------------------|
| QMC5883L  | 0x0D        | Most budget GPS/Compass units|
| HMC5883L  | 0x1E        | Older modules                |
| IST8310   | 0x0E        | Holybro, DJI modules         |

## Software Configuration

### Non-Blocking Startup

The GPS and magnetometer drivers are configured to start in **non-blocking mode**:

- **GPS Driver:** Starts and waits for data in the background. If no GPS is connected, it doesn't block boot.
- **Magnetometer Drivers:** Use `-q` (quiet) flag to suppress errors if the chip isn't detected.

This ensures the flight controller boots normally regardless of whether external GPS/Mag hardware is connected.

### Files Modified

The following files were configured for GPS and magnetometer support:

1. **NuttX Configuration** (`boards/raspberrypi/pico/nuttx-config/nsh/defconfig`):
   - UART1 enabled at 38400 baud (default for most GPS modules)
   - I2C1 enabled on GPIO6/GPIO7

2. **Board Configuration** (`boards/raspberrypi/pico/src/board_config.h`):
   - I2C bus expansion defined as Bus 1
   - GPIO pin definitions for I2C1

3. **I2C Bus Setup** (`boards/raspberrypi/pico/src/i2c.cpp`):
   - External I2C bus 1 initialized for magnetometer

4. **Sensor Startup Script** (`boards/raspberrypi/pico/init/rc.board_sensors`):
   - GPS driver starts on /dev/ttyS1 with UBX protocol (non-blocking)
   - Magnetometer drivers attempt auto-detect with `-q` quiet mode
   - All external sensor commands are non-blocking

5. **PX4 Board Config** (`boards/raspberrypi/pico/default.px4board`):
   - GPS driver enabled
   - QMC5883L, HMC5883L, IST8310 magnetometer drivers enabled


### GPS Testing Commands

```bash
# Check GPS driver status
gps status

# View GPS data stream
listener sensor_gps

# Check for GPS fix (look for satellites_used > 0)
listener sensor_gps -n 1
```

Expected output when GPS has a fix:
```
sensor_gps
    timestamp: 12345678
    lat: 37.7749000 (degrees)
    lon: -122.4194000 (degrees)
    alt: 10.500 (m)
    satellites_used: 8
    fix_type: 3 (3D fix)
```

### Magnetometer Testing Commands

```bash
# Scan I2C bus for devices
i2cdetect -b 1

# Check magnetometer status (try each driver)
qmc5883l status
hmc5883 status
ist8310 status

# View magnetometer data
listener sensor_mag

# View calibrated magnetic field
listener vehicle_magnetometer
```

Expected `i2cdetect` output with QMC5883L:
```
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- 0d -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
...
```

### Verifying in Ground Station

1. Connect Mission Planner or QGroundControl via USB (MAVLink on /dev/ttyACM0)
2. Go to the sensor status page
3. Verify:
   - GPS: Shows coordinates and satellite count
   - Compass: Shows heading (0-360°)
   - Both sensors should show "Healthy"

## Calibration

### Magnetometer Calibration

1. In Mission Planner: Setup → Mandatory Hardware → Compass
2. Click "Start" for onboard calibration
3. Rotate the drone in all orientations (sphere pattern)
4. Wait for completion message

Or via NSH:
```bash
# Start compass calibration
commander calibrate mag
```

### GPS Testing Outdoors

For best GPS performance:
- Test outdoors with clear sky view
- Wait 1-2 minutes for initial fix (cold start)
- Verify HDOP < 2.0 for good accuracy

## Troubleshooting

### GPS Not Detected

1. Check wiring (TX/RX may need to be swapped)
2. Verify UART1 is enabled in defconfig
3. Check GPS module baud rate matches (try 9600 or 115200 if 38400 fails)

```bash
# Try different baud rates
gps stop
gps start -d /dev/ttyS1 -b 9600 -p ubx
```

### Magnetometer Not Detected

1. Run `i2cdetect -b 1` to scan for I2C devices
2. Verify the I2C address matches a supported chip
3. Check for I2C pull-up resistors on the GPS module
4. Try starting the driver manually:

```bash
# For QMC5883L
qmc5883l -X -b 1 start

# For HMC5883L  
hmc5883 -T -X -b 1 start

# For IST8310
ist8310 -X -b 1 start
```

### No I2C Devices Found

This is **normal** if no external GPS/Mag module is connected. The board will operate without magnetometer data.

If you have a module connected:
1. Check I2C wiring (SDA to GPIO6, SCL to GPIO7)
2. Verify 3.3V power to GPS module
3. Some modules need external pull-ups (4.7kΩ to VCC)

### GPS Shows "No Fix"

If no GPS module is connected, this is expected behavior. The system will operate in "No GPS" mode.

If you have a GPS connected:
1. Ensure outdoor testing with sky view
2. Wait 2-5 minutes for cold start acquisition
3. Check antenna connection on GPS module
4. Verify the GPS module LED is blinking (indicates searching)

## Parameter Configuration

Set these PX4 parameters for optimal GPS/Mag operation:

```bash
# GPS parameters
param set GPS_1_CONFIG 102       # UART1 (/dev/ttyS1)
param set GPS_1_GNSS 7           # GPS + GLONASS + Galileo
param set GPS_1_PROTOCOL 1       # UBX

# Compass parameters  
param set CAL_MAG0_ROT 0         # No rotation (adjust if needed)
param set CAL_MAG_COMP_TYP 0     # Disable throttle compensation
param set EKF2_MAG_TYPE 1        # Use magnetometer

# Save parameters
param save
```

## Hardware Schematic Reference

```
GPS Module                    OpenFC2040
┌─────────────┐              ┌─────────────┐
│ VCC ────────┼──────────────┼─ 3.3V       │
│ GND ────────┼──────────────┼─ GND        │
│ TX ─────────┼──────────────┼─ GPIO5 (RX) │
│ RX ─────────┼──────────────┼─ GPIO4 (TX) │
│ SDA ────────┼──────────────┼─ GPIO6      │
│ SCL ────────┼──────────────┼─ GPIO7      │
└─────────────┘              └─────────────┘
```

**Note:** GPS TX connects to MCU RX (GPIO5), GPS RX connects to MCU TX (GPIO4).

## References

- [PX4 GPS Driver Documentation](https://docs.px4.io/main/en/gps_compass/)
- [u-blox Protocol Specification](https://www.u-blox.com/en/docs/UBX-13003221)
- [QMC5883L Datasheet](https://www.filipeflop.com/img/files/download/Datasheet-QMC5883L-1.0%20.pdf)