# OpenFC2040 Flight Controller Integration Checklist

A step-by-step checklist for bringing up all hardware and features on the OpenFC2040 board under PX4.

## Core fixes and updates
- [x] Barometer (DPS310) driver working
- [x] IMU (LSM6DS3TR-C) driver working
- [x] NSH shell stable and responsive (on telemetry port-UART0)
- [x] Sensor data visible in Mission Planner

## Next steps

### Motor Outputs
- [ ] Connect PWM ESC outputs to RP2040 pins (see `openfc2040_pinout.txt`)
- [ ] Configure PX4 mixer for motor outputs
- [ ] Test motor outputs with PX4 actuator test
- [ ] Confirm correct motor mapping and direction

### GPS & Magnetometer
- [ ] Connect GPS and magnetometer to correct pins
- [ ] Configure PX4 for GPS/mag (set serial port, baud rate)
- [ ] Verify GPS lock and heading in ground station

### SD Card Logging
- [ ] Connect SD card to RP2040 SPI pins
- [ ] Enable SD card logging in PX4
- [ ] Confirm log file creation and data integrity

### RGB LED & Buzzer
- [ ] Connect RGB LED and buzzer to assigned pins
- [ ] Configure PX4 for status LED and buzzer
- [ ] Test LED color/status and buzzer alerts

### Battery Monitoring
- [ ] Connect battery voltage/current sense to ADC pins
- [ ] Configure PX4 battery parameters
- [ ] Verify battery status in ground station

### RC Input
- [ ] Connect RC receiver to input pins
- [ ] Configure PX4 for RC input type (PWM/SBUS/CRSF)
- [ ] Test RC channel mapping and failsafe

### Telemetry
- [ ] Connect telemetry radio to UART pins
- [ ] Configure PX4 telemetry port
- [ ] Verify telemetry link to ground station

### Final Validation
- [ ] Calibrate all sensors (IMU, mag, baro, RC)
- [ ] Test arming/disarming and flight modes
- [ ] Perform bench test of all features
- [ ] Document pin mappings and configuration in `openfc2040_pinout.txt`

