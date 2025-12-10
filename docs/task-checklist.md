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


## Final Validation
 - [ ] Calibrate all sensors (IMU, mag, baro, RC)
 - [ ] Test arming/disarming and flight modes
 - [ ] Perform bench test of all features
 - [ ] Document pin mappings and configuration in `openfc2040_pinout.txt`

---

## Technical Implementation Plan

This section details the files to create or modify and the steps to implement each subsystem:

### 1. PWM Motor Outputs (ESCs)
- **Files:**
	- `firmware/openfc2040/rsp_2040/configs/board_config.h` (PWM pin assignments)
	- `firmware/openfc2040/rsp_2040/board/board_init.c` or `.cpp` (PWM init)
	- `PX4-Autopilot/ROMFS/px4fmu_common/mixers/openfc2040.main.mix` (custom mixer)
	- `PX4-Autopilot/boards/openfc2040/rsp_2040/default.cmake` (reference mixer)
- **Steps:**
	1. Assign PWM pins in board config.
	2. Initialize PWM in board bring-up code.
	3. Add a mixer file for your motor layout.
	4. Build, flash, and test with `pwm test` or actuator commands.

### 2. GPS & Magnetometer
- **Files:**
	- `firmware/openfc2040/rsp_2040/configs/board_config.h` (UART/I2C pins)
	- `firmware/openfc2040/rsp_2040/board/board_init.c` (UART/I2C init)
	- `PX4-Autopilot/src/drivers/gps/` and `/magnetometer/` (drivers)
	- `PX4-Autopilot/ROMFS/px4fmu_common/init.d/rc.board_sensors` (enable startup)
- **Steps:**
	1. Connect hardware as per pinout.
	2. Set up UART/I2C in board config/init.
	3. Enable or adapt drivers.
	4. Test with `gps status` and `mag status`.

### 3. SD Card Logging
- **Files:**
	- `firmware/openfc2040/rsp_2040/configs/board_config.h` (SPI pins)
	- `firmware/openfc2040/rsp_2040/board/board_init.c` (SPI/SD init)
	- `PX4-Autopilot/src/drivers/sdcard/` or `/block/` (driver)
	- `PX4-Autopilot/ROMFS/px4fmu_common/init.d/rc.board_sensors` (mount SD)
- **Steps:**
	1. Connect SD card to SPI.
	2. Initialize SPI/SD in board code.
	3. Enable logging in PX4 config.
	4. Test with log file creation.

### 4. RGB LED & Buzzer
- **Files:**
	- `firmware/openfc2040/rsp_2040/configs/board_config.h` (pin assignments)
	- `firmware/openfc2040/rsp_2040/board/board_init.c` (GPIO init)
	- `PX4-Autopilot/src/drivers/led/` and `/tone_alarm/` (drivers)
- **Steps:**
	1. Assign pins and initialize in board code.
	2. Enable drivers in PX4.
	3. Test with `led_control` and `tone_alarm`.

### 5. Battery Monitoring
- **Files:**
	- `firmware/openfc2040/rsp_2040/configs/board_config.h` (ADC pins)
	- `firmware/openfc2040/rsp_2040/board/board_init.c` (ADC init)
	- `PX4-Autopilot/src/drivers/battery_status/` (driver)
- **Steps:**
	1. Connect sense lines to ADC.
	2. Initialize ADC in board code.
	3. Configure battery parameters in PX4.
	4. Test with `battery_status`.

### 6. RC Input
- **Files:**
	- `firmware/openfc2040/rsp_2040/configs/board_config.h` (RC input pin)
	- `firmware/openfc2040/rsp_2040/board/board_init.c` (input capture init)
	- `PX4-Autopilot/src/drivers/rc_input/` (driver)
- **Steps:**
	1. Connect RC receiver.
	2. Initialize input capture in board code.
	3. Enable driver in PX4.
	4. Test with `rc_input status`.

### 7. Telemetry
- **Files:**
	- `firmware/openfc2040/rsp_2040/configs/board_config.h` (UART pins)
	- `firmware/openfc2040/rsp_2040/board/board_init.c` (UART init)
	- `PX4-Autopilot/ROMFS/px4fmu_common/init.d/rc.board_sensors` (enable telemetry)
- **Steps:**
	1. Connect telemetry radio.
	2. Initialize UART in board code.
	3. Enable telemetry in PX4 config.
	4. Test with ground station.

### 8. Final Validation & Documentation
- **Files:**
	- `hardware/openfc2040_pinout.txt` (final pin mappings)
	- `docs/FC2040_Integration_Checklist.md` (mark completed items)
	- `docs/` (add new documentation as needed)

---
For each subsystem, update pin assignments and hardware initialization in your board config and bring-up files. If PX4 has a suitable driver, enable/configure it; otherwise, adapt or write a new driver in the appropriate `src/drivers/` subdirectory. Add or update test scripts in `scripts/` or `test_builds/` to validate each feature. Build and flash after each major change, testing in NSH or with PX4 tools.

