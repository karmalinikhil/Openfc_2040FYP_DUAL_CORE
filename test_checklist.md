# OpenFC2040 Test Checklist

## Pre-Flight Hardware Verification

### 1. Visual Inspection
- [ ] Board properly mounted
- [ ] All connectors secure
- [ ] No visible damage or shorts
- [ ] Proper orientation (arrow pointing forward)

### 2. Power System
- [ ] Battery voltage correct (3S: 11.1-12.6V)
- [ ] Power LED illuminated
- [ ] No excessive heat
- [ ] Current sensor reading properly

### 3. Connections
- [ ] ESCs connected to GPIO20-23
- [ ] GPS connected to UART1 (GPIO4/5)
- [ ] Telemetry connected to UART0 (GPIO0/1)
- [ ] RC receiver connected to GPIO24
- [ ] USB cable connected for initial setup

## Firmware Verification

### 1. Boot Sequence
```bash
# Connect via USB serial console (115200 baud)
# Expected output:
NuttShell (NSH) NuttX-10.3.0
nsh> 
```

- [ ] Board boots successfully
- [ ] No error messages during boot
- [ ] NSH prompt appears

### 2. System Information
```bash
nsh> ver all
```
- [ ] Correct hardware version displayed
- [ ] Firmware version matches build
- [ ] Git hash correct

### 3. LED Test
```bash
nsh> led_control test
```
- [ ] Red LED blinks
- [ ] Green LED blinks
- [ ] Blue LED blinks
- [ ] RGB sequence works

## Sensor Verification

### 1. IMU (LSM6DS3)
```bash
nsh> lsm6ds3 status
```
- [ ] Device detected on SPI1
- [ ] WHO_AM_I returns 0x6A
- [ ] No communication errors

```bash
nsh> sensors status
```
- [ ] Accelerometer data updating
- [ ] Gyroscope data updating
- [ ] Temperature reading valid
- [ ] Data rates correct (416Hz)

### 2. Barometer (DPS310)
```bash
nsh> dps310 status
```
- [ ] Device detected on SPI1
- [ ] Pressure reading reasonable (near 1013 hPa at sea level)
- [ ] Temperature consistent with IMU
- [ ] Altitude calculation working

### 3. Sensor Calibration
```bash
nsh> sensors calibrate accel
```
- [ ] Place board level, face up
- [ ] Place board on left side
- [ ] Place board on right side
- [ ] Place board nose up
- [ ] Place board nose down
- [ ] Place board upside down
- [ ] Calibration successful

```bash
nsh> sensors calibrate gyro
```
- [ ] Keep board still
- [ ] Calibration completes
- [ ] Bias values reasonable

## Communication Tests

### 1. USB Connection
- [ ] Device enumerated as CDC ACM
- [ ] Console communication working
- [ ] MAVLink over USB working

### 2. Telemetry Port
```bash
nsh> mavlink status
```
- [ ] MAVLink instance on /dev/ttyS0
- [ ] Baud rate 57600
- [ ] Heartbeat messages sending

### 3. GPS Port
```bash
nsh> gps status
```
- [ ] GPS module detected
- [ ] NMEA/UBX messages received
- [ ] Fix status updating (when outdoors)

### 4. Ground Control Station
- [ ] Connect QGroundControl via USB or telemetry
- [ ] Vehicle detected
- [ ] Parameters downloadable
- [ ] Sensor data visible
- [ ] Artificial horizon moving correctly

## Actuator Tests

### 1. PWM Outputs
```bash
nsh> pwm info
```
- [ ] 4 channels configured
- [ ] Rate 400Hz
- [ ] Range 1000-2000μs

```bash
# Test each channel (REMOVE PROPELLERS!)
nsh> pwm test -c 1 -p 1100
nsh> pwm test -c 2 -p 1100
nsh> pwm test -c 3 -p 1100
nsh> pwm test -c 4 -p 1100
```
- [ ] ESC1 beeps/spins (GPIO20)
- [ ] ESC2 beeps/spins (GPIO21)
- [ ] ESC3 beeps/spins (GPIO22)
- [ ] ESC4 beeps/spins (GPIO23)

### 2. Motor Test (via QGC)
- [ ] Motor A spins (front right)
- [ ] Motor B spins (back left)
- [ ] Motor C spins (front left)
- [ ] Motor D spins (back right)
- [ ] Correct rotation direction

### 3. RC Input
```bash
nsh> rc_input status
```
- [ ] RC receiver detected
- [ ] Channel values updating
- [ ] All channels responding
- [ ] Failsafe working

## Flight Readiness

### 1. Arming Checks
```bash
nsh> commander status
```
- [ ] All pre-arm checks pass
- [ ] No error flags
- [ ] Home position set (if GPS available)

### 2. Flight Modes
- [ ] Manual mode selectable
- [ ] Altitude mode selectable
- [ ] Position mode selectable (with GPS)
- [ ] Return mode selectable
- [ ] Mode switches working

### 3. Safety Features
- [ ] Kill switch working
- [ ] Failsafe triggers on RC loss
- [ ] Low battery warning
- [ ] Geofence (if configured)

## Bench Test Procedure

### Setup
1. **Remove all propellers**
2. Secure aircraft to bench
3. Connect battery
4. Connect GCS

### Test Sequence
1. **System Check**
   - [ ] Boot complete
   - [ ] Sensors online
   - [ ] GPS acquiring (if outdoors)

2. **Calibrations**
   - [ ] Accelerometer calibrated
   - [ ] Gyroscope calibrated
   - [ ] RC calibrated
   - [ ] ESCs calibrated

3. **Control Response** (motors spinning slowly)
   - [ ] Roll right: right motors speed up
   - [ ] Roll left: left motors speed up
   - [ ] Pitch forward: front motors slow down
   - [ ] Pitch back: back motors slow down
   - [ ] Yaw right: diagonal motors change speed
   - [ ] Yaw left: opposite diagonal response

4. **Arm/Disarm**
   - [ ] Arm via RC stick command
   - [ ] Motors spin at idle
   - [ ] Throttle response smooth
   - [ ] Disarm working

## First Flight

### Pre-Flight
- [ ] All bench tests passed
- [ ] Propellers installed correctly
- [ ] Battery fully charged
- [ ] Range check completed
- [ ] Flight area clear
- [ ] Weather conditions suitable

### Tethered Hover
1. Secure with fishing line (1m)
2. Arm in Stabilized mode
3. Slowly increase throttle
4. Check for:
   - [ ] Stable hover
   - [ ] No oscillations
   - [ ] Correct orientation
   - [ ] Responsive to inputs

### Free Flight
1. **Low Hover** (1m altitude)
   - [ ] Stable in position
   - [ ] Gentle control inputs work
   - [ ] Landing smooth

2. **Basic Maneuvers** (5m altitude)
   - [ ] Forward/backward flight
   - [ ] Left/right flight
   - [ ] Yaw rotation
   - [ ] Figure-8 pattern

3. **Mode Tests**
   - [ ] Altitude hold working
   - [ ] Position hold working (GPS)
   - [ ] Return to launch working

## Data Analysis

### Log Review
```bash
# Download logs
nsh> hardfault_log check
nsh> logger on
nsh> logger off
```

- [ ] No error messages
- [ ] Vibration levels acceptable
- [ ] Control loops stable
- [ ] Battery consumption normal

### Performance Metrics
- [ ] Loop rate achieving 400Hz
- [ ] CPU usage < 80%
- [ ] Memory usage stable
- [ ] No dropped messages

## Troubleshooting Guide

### Common Issues

1. **Won't Boot**
   - Check power connections
   - Verify firmware flash successful
   - Check for shorts
   - Try recovery mode (BOOTSEL)

2. **Sensors Not Detected**
   - Check SPI connections
   - Verify chip select pins
   - Check sensor power
   - Review board_config.h

3. **No PWM Output**
   - Verify timer configuration
   - Check GPIO assignments
   - Confirm mixer loaded
   - Test with pwm command

4. **GPS Not Working**
   - Check UART pins
   - Verify baud rate
   - Ensure outdoor location
   - Check antenna connection

5. **Won't Arm**
   - Review arming checks
   - Calibrate sensors
   - Check RC input
   - Verify safety switch

6. **Unstable Flight**
   - Check CG (center of gravity)
   - Verify prop direction
   - Review PID tuning
   - Check vibration levels
   - Confirm sensor orientation

## Sign-Off

### Test Engineer
- Name: _________________
- Date: _________________
- Signature: _________________

### Test Results
- [ ] All tests passed
- [ ] Issues found (document below)
- [ ] Approved for flight
- [ ] Requires rework

### Notes
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________

---

**Document Version**: 1.0.0  
**Board**: OpenFC2040  
**Firmware**: PX4 v1.14.x
