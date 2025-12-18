# OpenFC2040 Flight Controller Integration Checklist

A step-by-step checklist for bringing up all hardware and features on the OpenFC2040 board under PX4.

## Core fixes and updates
- [x] Barometer (DPS310) driver working
- [x] IMU (LSM6DS3TR-C) driver working
- [x] NSH shell available on UART0 (/dev/ttyS0)
- [x] MAVLink over USB CDC (/dev/ttyACM0) working
- [x] Attitude (roll/pitch/yaw) visible in Mission Planner (MAVLink mode: Normal)
- [x] IMU publication rate increased (vehicle_imu interval ~20000 us)

## Next steps

### Motor Outputs
- [x] PWM output pin mapping defined (GPIO20-23)
- [x] pwm_out driver running (Param prefix: PWM_MAIN)
- [ ] Assign actuator functions (PWM_MAIN_FUNC1..4 -> Motor 1..4) via airframe/actuator setup
- [ ] Test motor outputs with PX4 actuator test (props off)
- [ ] Confirm correct motor mapping and direction

### GPS & Magnetometer
- [x] GPS UART1 mapping available (GPIO4/5 -> /dev/ttyS1)
- [x] External I2C1 mapping available for magnetometer (GPIO6/7)
- [ ] Verify GPS wiring + baud (gps status currently shows NOT OK / 0 B/s)
- [ ] Verify magnetometer presence and calibration (currently operating without mag)

### SD Card Logging
- [ ] Connect SD card to RP2040 SPI pins
- [ ] Enable SD card logging in PX4
- [ ] Confirm log file creation and data integrity

### RGB LED & Buzzer
- [x] Connect RGB LED and buzzer to assigned pins
- [x] Configure PX4 for status LED and buzzer
- [x] Test LED color/status and buzzer alerts (needs refining still, 16 Dec)

### Battery Monitoring
- [x] ADC channels defined (GPIO27 voltage, GPIO28 current)
- [x] Board defaults set for voltage divider/current scaling
- [ ] Verify battery status in ground station

### RC Input
- [ ] Connect RC receiver to input pins
- [ ] Configure PX4 for RC input type (PPM on GPIO24 expected by board)
- [ ] Test RC channel mapping and failsafe

### Telemetry
- [x] MAVLink telemetry via USB CDC
- [ ] Optional: configure external telemetry radio on UART (if required)

## Final Validation
- [ ] Calibrate all sensors (IMU, mag, baro, RC)
- [ ] Test arming/disarming and flight modes
- [ ] Perform bench test of all features
- [ ] Document pin mappings and configuration in `docs/rp2040-pinout.txt`

## Current Blockers / Notes
- EKF2 is not enabled by default (EKF2_EN=0, ATT_EN=1) and EKF2 bring-up is still pending due to CPU/load constraints on RP2040.
- PWM outputs are present, but motor functions are not assigned yet (pwm_out shows func 0 on channels).
- GPS currently shows NOT OK (0 B/s) and needs wiring/serial parameter verification.

---

## Technical Implementation Plan

This section details the files to create or modify and the steps to implement each subsystem:

### 1. PWM Motor Outputs (ESCs)
- **Files:**
	- `PX4-Autopilot/boards/raspberrypi/pico/src/board_config.h` (PWM pin assignments)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/init.c` (PWM init)
	- `PX4-Autopilot/ROMFS/px4fmu_common/mixers/openfc2040.main.mix` (custom mixer)
	- `PX4-Autopilot/boards/raspberrypi/pico/default.px4board` (board config)
- **Steps:**
	1. Assign PWM pins in board config.
	2. Initialize PWM in board bring-up code.
	3. Add a mixer file for your motor layout.
	4. Build, flash, and test with `pwm test` or actuator commands.

### 2. GPS & Magnetometer ✅ COMPLETED
- **Files Modified:**
	- `PX4-Autopilot/boards/raspberrypi/pico/nuttx-config/nsh/defconfig` (UART1/I2C1 config)
	- `PX4-Autopilot/boards/raspberrypi/pico/nuttx-config/include/board.h` (GPIO assignments)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/board_config.h` (I2C bus definition)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/i2c.cpp` (I2C bus initialization)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/init.c` (GPIO function setup)
	- `PX4-Autopilot/boards/raspberrypi/pico/init/rc.board_sensors` (driver startup)
	- `PX4-Autopilot/boards/raspberrypi/pico/default.px4board` (driver enables)
- **Pin Mapping:**
	- GPS UART1: TX=GPIO4, RX=GPIO5 → /dev/ttyS1
	- Magnetometer I2C1: SDA=GPIO6, SCL=GPIO7 → Bus 1 (External)
- **Drivers Enabled:**
	- GPS: UBX protocol (u-blox compatible)
	- Magnetometers: QMC5883L, HMC5883L, IST8310 (auto-detect)
- **Testing:**
	- External GPS/Mag module is optional
	- Drivers start in non-blocking mode
	- Test commands: `gps status`, `i2cdetect -b 1`, `listener sensor_gps`

### 3. SD Card Logging
- **Files:**
	- `PX4-Autopilot/boards/raspberrypi/pico/src/board_config.h` (SPI pins)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/init.c` (SPI/SD init)
	- `PX4-Autopilot/src/drivers/sdcard/` or `/block/` (driver)
	- `PX4-Autopilot/boards/raspberrypi/pico/init/rc.board_sensors` (mount SD)
- **Steps:**
	1. Connect SD card to SPI0.
	2. Initialize SPI/SD in board code.
	3. Enable logging in PX4 config.
	4. Test with log file creation.

### 4. RGB LED & Buzzer ✅ COMPLETED
- **Files:**
	- `PX4-Autopilot/boards/raspberrypi/pico/src/board_config.h` (pin assignments)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/init.c` (GPIO init)
	- `PX4-Autopilot/src/drivers/lights/rgbled_gpio/` (LED driver)
	- `PX4-Autopilot/src/drivers/tone_alarm/` (buzzer driver)
- **Pin Mapping:**
	- Red LED: GPIO13 (Active LOW, 162Ω resistor)
	- Green LED: GPIO14 (Active LOW, 60.4Ω resistor)
	- Blue LED: GPIO15 (Active LOW, 100Ω resistor)
	- Buzzer: GPIO25 (Active HIGH, passive buzzer)
- **Testing:**
	- `led_control on -c red`
	- `led_control on -c green`
	- `led_control on -c blue`
	- `led_control on -c white`
	- `tune_control play -f 2700 -d 1000`

### 5. Battery Monitoring
- **Files:**
	- `PX4-Autopilot/boards/raspberrypi/pico/src/board_config.h` (ADC pins)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/init.c` (ADC init)
	- `PX4-Autopilot/src/modules/battery_status/` (module)
- **Steps:**
	1. Connect sense lines to ADC.
	2. Initialize ADC in board code.
	3. Configure battery parameters in PX4.
	4. Test with `battery_status`.

### 6. RC Input
- **Files:**
	- `PX4-Autopilot/boards/raspberrypi/pico/src/board_config.h` (RC input pin)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/init.c` (input capture init)
	- `PX4-Autopilot/src/drivers/rc_input/` (driver)
- **Steps:**
	1. Connect RC receiver.
	2. Initialize input capture in board code.
	3. Enable driver in PX4.
	4. Test with `rc_input status`.

### 7. Telemetry
- **Files:**
	- `PX4-Autopilot/boards/raspberrypi/pico/src/board_config.h` (UART pins)
	- `PX4-Autopilot/boards/raspberrypi/pico/src/init.c` (UART init)
	- `PX4-Autopilot/boards/raspberrypi/pico/init/rc.board_mavlink` (MAVLink startup)
- **Steps:**
	1. Connect telemetry radio.
	2. Initialize UART in board code.
	3. Enable telemetry in PX4 config.
	4. Test with ground station.

### 8. Final Validation & Documentation
- **Files:**
	- `docs/rp2040-pinout.txt` (final pin mappings)
	- `docs/TASK_CHECKLIST.md` (mark completed items)
	- `docs/HARDWARE-CONNECTION-FUNCTION.md` (comprehensive reference)

---

## Build and Flash Commands

```bash
# Navigate to PX4-Autopilot directory
cd PX4-Autopilot

# Build firmware
make raspberrypi_pico_default

# Convert to UF2 (requires picotool)
picotool uf2 convert build/raspberrypi_pico_default/raspberrypi_pico_default.bin \
    ../firmware/test-firmware/openfc2040.uf2 --family rp2040 --offset 0x10000000

# Flash: Hold BOOT button, connect USB, release, copy .uf2 file to drive
```

---

For each subsystem, update pin assignments and hardware initialization in your board config and bring-up files. If PX4 has a suitable driver, enable/configure it; otherwise, adapt or write a new driver in the appropriate `src/drivers/` subdirectory. Build and flash after each major change, testing in NSH or with PX4 tools.