# OpenFC2040 Sensor Calibration Guide

## 🎯 Easy Calibration System

All sensor calibration constants are now located at the **top of `src/main.c`** for easy access and tuning.

## 📍 Location of Calibration Constants

Open `src/main.c` and look for this section near the top:

```c
// ========================================
// SENSOR CALIBRATION CONSTANTS
// ========================================
// Adjust these values to fine-tune sensor readings
```

## 🔧 How to Calibrate

### 1. **Check Current Values**
```bash
calib    # Shows all current calibration constants
```

### 2. **IMU Calibration**

**Accelerometer Offsets** (if readings are biased when stationary):
```c
#define IMU_ACCEL_OFFSET_X    0.0f     // mg - Adjust if X-axis shows bias
#define IMU_ACCEL_OFFSET_Y    0.0f     // mg - Adjust if Y-axis shows bias  
#define IMU_ACCEL_OFFSET_Z    0.0f     // mg - Adjust if Z-axis shows bias
```

**Gyroscope Offsets** (if readings drift when stationary):
```c
#define IMU_GYRO_OFFSET_X     0.0f     // mdps - Adjust if X-axis drifts
#define IMU_GYRO_OFFSET_Y     0.0f     // mdps - Adjust if Y-axis drifts
#define IMU_GYRO_OFFSET_Z     0.0f     // mdps - Adjust if Z-axis drifts
```

### 3. **Barometer Calibration**

**Temperature Calibration**:
```c
#define BARO_TEMP_BASELINE_OFFSET  25.0f        // °C - Set to current room temperature
#define BARO_TEMP_FINE_TUNE        0.0f         // °C - Fine adjustment (+/-)
```

**Pressure Calibration**:
```c
#define BARO_PRESS_BASELINE_OFFSET 101325.0f    // Pa - Standard atmospheric pressure
#define BARO_PRESS_FINE_TUNE       0.0f         // Pa - Fine adjustment (+/-)
```

## 📊 Calibration Process

### **Step 1: Temperature Calibration**
1. Measure actual room temperature with a thermometer
2. Run `baro` command to see current reading
3. Adjust `BARO_TEMP_BASELINE_OFFSET` to match actual temperature
4. Use `BARO_TEMP_FINE_TUNE` for small adjustments

### **Step 2: Pressure Calibration**
1. Look up local atmospheric pressure (weather app/website)
2. Run `baro` command to see current reading
3. Adjust `BARO_PRESS_FINE_TUNE` to match actual pressure

### **Step 3: IMU Calibration**
1. Place board on flat, level surface
2. Run `imu` command multiple times
3. If accelerometer shows consistent bias, adjust offsets
4. If gyroscope shows drift when stationary, adjust offsets

## 🔄 Calibration Workflow

1. **Edit** calibration constants in `src/main.c`
2. **Build** firmware: `./scripts/build.sh`
3. **Flash** firmware: `./scripts/flash.sh` (with BOOTSEL)
4. **Test** with `baro`, `imu`, or `live` commands
5. **Repeat** until values are accurate

## 📋 Useful Commands

```bash
help        # Show all commands
calib       # Show current calibration constants
baro        # Test barometer readings
imu         # Test IMU readings
live        # Real-time sensor data
debug       # Raw sensor values for troubleshooting

# New buzzer commands (QMB-09B-03 Passive Buzzer)
beep        # Standard 1kHz beep
melody      # Startup melody (multi-tone)
tone        # Play low/mid/high frequency sequence
```

## 💡 Tips

- **Small changes**: Make small adjustments (±1-5 units) and test
- **Real references**: Use actual thermometer and weather data for reference
- **Stable environment**: Calibrate in stable temperature/pressure conditions
- **Multiple readings**: Take several readings to ensure consistency

## 📝 Example Calibration

If your barometer reads 23.5°C but actual temperature is 25.0°C:
```c
#define BARO_TEMP_FINE_TUNE        1.5f         // +1.5°C adjustment
```

If your pressure reads 101000 Pa but actual is 101325 Pa:
```c
#define BARO_PRESS_FINE_TUNE       325.0f       // +325 Pa adjustment
```

## 🎯 Result

After calibration, your sensors should show:
- **Temperature**: Within ±1°C of actual
- **Pressure**: Within ±50 Pa of actual atmospheric pressure
- **IMU**: Minimal drift when stationary, accurate gravity reading (~1000mg on Z-axis)
