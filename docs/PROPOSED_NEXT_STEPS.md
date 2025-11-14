# Proposed Next Steps for OpenFC2040 Project

**Generated**: Based on comprehensive repository analysis  
**Date**: Current session  
**Status**: Ready for implementation

---

## Executive Summary

The OpenFC2040 flight controller project is in a **good state** with:
- ✅ PX4 firmware builds successfully (1.1MB UF2)
- ✅ Peripheral test firmware working (USB console functional)
- ✅ Comprehensive documentation
- ⏳ Currently working on JST-GH6 telemetry connection
- ⚠️ **CRITICAL**: Battery voltage divider needs verification before connecting 4S LiPo

---

## Current Project Status

### What's Working ✅
1. **Build System**: PX4 compiles successfully for RP2040
2. **Test Firmware**: Peripheral testing firmware works with USB console
3. **Documentation**: Comprehensive guides in `docs/` directory
4. **Hardware Verification**: Test firmware confirms all GPIO pins functional
5. **SWD Debugging**: Infrastructure ready (OpenOCD + Picoprobe)

### What's Not Working ❌
1. **PX4 USB Console**: Garbled output (known NuttX limitation)
2. **UART Console Access**: Blocked on JST-GH6 connection (IN PROGRESS)
3. **LSM6DS3 IMU Driver**: Not ported to PX4 yet
4. **Battery Monitoring**: Voltage divider config potentially incorrect (CRITICAL)

### Current Blockers
1. **JST-GH6 Telemetry Connection** (IN PROGRESS - Session: Nov 4, 2025)
   - Status: Wiring JST-GH6 to USB-Serial adapter
   - Impact: Blocks all hardware testing
   - Resolution: Complete wiring, test console access

2. **Battery Voltage Divider** (CRITICAL SAFETY ISSUE)
   - Current config: `BOARD_BATTERY_V_DIV = 1.74` (max safe: 5.74V)
   - Problem: 4S LiPo max = 16.8V → would damage ADC!
   - Required action: Verify from schematic, calculate correct divider
   - **DO NOT CONNECT BATTERY UNTIL RESOLVED**

---

## Proposed Next Steps (Prioritized)

### Phase 1: Console Access & Hardware Verification (Priority 1 - BLOCKING)

#### Step 1.1: Complete JST-GH6 Telemetry Connection ⏳ IN PROGRESS
**Status**: Currently wiring  
**Time**: 1-2 hours  
**Dependencies**: None

**Tasks**:
- [ ] Complete JST-GH6 to USB-Serial adapter wiring
  - Adapter TX → Board GPIO1 (RX)
  - Adapter RX → Board GPIO0 (TX)
  - Adapter GND → Board GND
- [ ] Verify connections with multimeter (continuity check)
- [ ] Power on OpenFC2040 (via USB)
- [ ] Connect adapter to PC
- [ ] Test serial connection: `picocom -b 115200 /dev/ttyUSB0`

**Success Criteria**:
- Clean console output (no garbled characters)
- NSH prompt appears: `nsh>`
- Commands respond (`help`, `ver`, `free`)

**Next After**: Proceed to Step 1.2

---

#### Step 1.2: UART Console Verification
**Status**: Ready (depends on Step 1.1)  
**Time**: 30 minutes  
**Dependencies**: Step 1.1

**Tasks**:
- [ ] Flash UART firmware: `raspberrypi_pico_minimal_UART.uf2`
- [ ] Connect via picocom: `picocom -b 115200 /dev/ttyUSB0`
- [ ] Test basic commands:
  ```
  nsh> ver all       # Verify firmware version
  nsh> free          # Check memory usage
  nsh> dmesg         # Review boot log
  nsh> help          # List available commands
  ```

**Success Criteria**:
- Clean console output
- All commands respond correctly
- Boot log shows no critical errors

**Next After**: Proceed to Step 1.3

---

#### Step 1.3: Hardware Peripheral Tests
**Status**: Ready (depends on Step 1.2)  
**Time**: 4-6 hours  
**Dependencies**: Step 1.2

**Test Sequence**:

1. **RGB LED Test**:
   ```
   nsh> led_control test
   ```
   - Verify Red, Green, Blue LEDs light up
   - Remember: Active LOW (0 = ON)

2. **PWM Output Test** (⚠️ NO PROPELLERS!):
   ```
   nsh> pwm test
   ```
   - Connect oscilloscope or LED to GPIO20-23
   - Verify 50 Hz PWM signal, 1000-2000µs pulse width

3. **Barometer Test**:
   ```
   nsh> dps310 start
   nsh> dps310 status
   nsh> listener sensor_baro
   ```
   - Verify pressure readings (~1013 hPa at sea level)
   - Verify temperature readings (~20-25°C)

4. **SD Card Test**:
   ```
   nsh> sd_bench
   ```
   - Insert microSD card (FAT32 formatted)
   - Verify read/write speeds

5. **RC Input Test**:
   ```
   nsh> input_rc status
   ```
   - Connect RC receiver to GPIO24
   - Verify channel values update

**Success Criteria**:
- All peripherals respond without errors
- All sensor data looks reasonable
- No hardware faults in dmesg

**Next After**: Proceed to Phase 2

---

### Phase 2: Critical Safety Fixes (Priority 1 - CRITICAL)

#### Step 2.1: Fix Battery Voltage Divider ⚠️ CRITICAL
**Status**: Identified as critical issue  
**Time**: 2-3 hours  
**Priority**: **CRITICAL** (risk of ADC damage)

**Current Situation**:
- Firmware config: `BOARD_BATTERY_V_DIV = 1.74` (max input: 5.74V)
- 4S LiPo range: 14.8V nominal, 16.8V fully charged
- **Problem**: 16.8V × 1.74 = **29.23V at ADC!** (Will damage RP2040!)

**Action Plan**:
1. [ ] Review schematic: `hardware/openfc2040-official/openFC2040.json`
2. [ ] Identify resistor network for battery voltage sense
3. [ ] Extract resistor values from BOM: `BOM_PCB_openFC2040_2022-05-01.csv`
4. [ ] Calculate actual divider ratio:
   ```
   V_adc = V_battery × (R_bottom / (R_top + R_bottom))
   ```
5. [ ] For 16.8V max input, 3.3V ADC:
   ```
   Required divider: 16.8V / 3.3V = 5.09 minimum
   ```
6. [ ] Update `firmware/openfc2040/rsp_2040/board/board_config.h`:
   ```c
   #define BOARD_BATTERY_V_DIV (calculated_value)
   ```
7. [ ] Test with multimeter and known voltage source (before connecting battery!)

**Success Criteria**:
- ADC reading matches multimeter (±0.1V)
- 16.8V input reads correctly
- No ADC saturation
- **Verified safe before connecting 4S LiPo**

**Next After**: Proceed to Phase 3

---

### Phase 3: Driver Development (Priority 2)

#### Step 3.1: Port LSM6DS3 IMU Driver
**Status**: Not started  
**Time**: 6-8 hours  
**Dependencies**: Step 1.2 (console working)

**Source**: `firmware/test-firmware/peripherals_testing/src/main.c` (working driver)

**Steps**:
1. [ ] Create driver structure:
   ```
   firmware/openfc2040/rsp_2040/drivers/imu/st/lsm6ds3/
   ├── LSM6DS3.cpp
   ├── LSM6DS3.hpp
   └── CMakeLists.txt
   ```
2. [ ] Port SPI communication code from test firmware
3. [ ] Implement PX4 driver interface (poll(), ioctl(), etc.)
4. [ ] Add uORB topic publication (sensor_accel, sensor_gyro)
5. [ ] Add calibration constants
6. [ ] Test on hardware

**Success Criteria**:
- Driver compiles and links
- `lsm6ds3 start` succeeds
- `lsm6ds3 status` shows device detected
- `listener sensor_accel` shows ~9.8 m/s² gravity
- `listener sensor_gyro` shows ~0 when stationary

**Next After**: Proceed to Step 3.2

---

#### Step 3.2: Verify All Sensors
**Status**: Not started  
**Time**: 2-3 hours  
**Dependencies**: Step 3.1

**Test Each Sensor**:
- [ ] IMU (LSM6DS3): Gyro + Accelerometer
- [ ] Barometer (DPS310): Pressure + Temperature
- [ ] Battery voltage (ADC1): Voltage reading
- [ ] Battery current (ADC2): Current reading (if sensor installed)

**Calibration**:
```
nsh> commander calibrate accel
nsh> commander calibrate gyro
nsh> commander calibrate baro
```

**Success Criteria**:
- All sensors publish valid data
- Calibration completes successfully
- Data looks reasonable

**Next After**: Proceed to Phase 4

---

### Phase 4: System Integration (Priority 3)

#### Step 4.1: MAVLink Telemetry Test
**Status**: Not started  
**Time**: 2-3 hours  
**Dependencies**: All sensors working

**Setup**:
1. [ ] Connect telemetry radio to UART0 (GPIO0/1)
2. [ ] Configure MAVLink:
   ```
   nsh> mavlink start -d /dev/ttyS0 -b 57600
   ```
3. [ ] Connect QGroundControl on PC
4. [ ] Verify all sensor data visible

**Success Criteria**:
- QGroundControl connects successfully
- All sensor data visible in QGC
- Telemetry stable

**Next After**: Proceed to Step 4.2

---

#### Step 4.2: Full Integration Test
**Status**: Not started  
**Time**: 4-6 hours  
**Dependencies**: All above tasks complete

**Test Checklist**:
- [ ] All sensors publishing data
- [ ] QGroundControl connects
- [ ] Can arm/disarm via RC
- [ ] PWM outputs respond to stick input (NO PROPS!)
- [ ] SD logging works
- [ ] Battery voltage displayed correctly
- [ ] All pre-flight checks pass

**Success Criteria**:
- Ready for bench testing with motors (NO FLIGHT)
- All systems green in QGroundControl

---

## Immediate Action Items (This Week)

### Day 1-2: Console Access
1. ✅ Complete JST-GH6 wiring (IN PROGRESS)
2. ⏳ Test UART console connection
3. ⏳ Verify NSH shell access
4. ⏳ Run basic hardware tests (LED, PWM)

### Day 3-4: Critical Safety
1. ⏳ Review battery voltage divider schematic
2. ⏳ Calculate correct divider ratio
3. ⏳ Update firmware configuration
4. ⏳ Test with multimeter (before battery!)

### Day 5-7: Driver Development
1. ⏳ Port LSM6DS3 IMU driver
2. ⏳ Test sensor data acquisition
3. ⏳ Calibrate sensors

---

## Risk Assessment

### High Risk ⚠️
1. **Battery Voltage Divider**: Risk of hardware damage if incorrect
   - **Mitigation**: Verify from schematic before any battery connection
   - **Status**: Needs immediate attention

### Medium Risk 🟡
1. **IMU Driver Porting**: Complex task, may have integration issues
   - **Mitigation**: Use working test firmware as reference
   - **Status**: Well-documented approach

2. **Hardware Testing**: Some peripherals may have issues
   - **Mitigation**: Test incrementally, document failures
   - **Status**: Test firmware proves hardware works

### Low Risk 🟢
1. **Build System**: Stable and working
2. **Documentation**: Comprehensive and up-to-date
3. **SWD Debugging**: Infrastructure ready if needed

---

## Resource Requirements

### Hardware Needed
- [x] OpenFC2040 board
- [x] USB-to-Serial adapter (for JST-GH6 connection)
- [ ] Multimeter (for voltage divider verification)
- [ ] Oscilloscope (optional, for PWM testing)
- [ ] RC receiver (for RC input testing)
- [ ] GPS module (optional, for GPS testing)

### Software/Tools
- [x] ARM toolchain (gcc-arm-none-eabi)
- [x] CMake, Ninja
- [x] OpenOCD, GDB (for debugging)
- [x] picocom (for serial console)
- [ ] QGroundControl (for telemetry testing)

### Time Estimates
- **Phase 1** (Console + Hardware Tests): 6-8 hours
- **Phase 2** (Safety Fixes): 2-3 hours
- **Phase 3** (Driver Development): 8-11 hours
- **Phase 4** (System Integration): 6-8 hours
- **Total**: ~22-30 hours of focused work

---

## Success Metrics

### Short-term (This Week)
- ✅ UART console working
- ✅ All basic peripherals tested
- ✅ Battery voltage divider fixed
- ✅ IMU driver ported

### Medium-term (This Month)
- ✅ All sensors working
- ✅ MAVLink telemetry functional
- ✅ QGroundControl integration
- ✅ Ready for bench testing

### Long-term (Next Month)
- ✅ Full flight control system operational
- ✅ Test stand validation
- ✅ Ready for test flights (with proper safety measures)

---

## Notes and Recommendations

### Best Practices
1. **Test Incrementally**: Don't test everything at once
2. **Document Everything**: Update `docs/NEXT_STEPS.md` after each session
3. **Safety First**: Never connect battery until voltage divider verified
4. **Use Test Firmware**: Reference working code from `peripherals_testing/`
5. **Git Workflow**: Commit frequently with clear messages

### Common Pitfalls to Avoid
1. ❌ Don't connect 4S LiPo before verifying voltage divider
2. ❌ Don't test ESC outputs with propellers attached
3. ❌ Don't skip hardware verification steps
4. ❌ Don't modify PX4 submodule directly (use board/ directory)

### Getting Help
- **Documentation**: See `docs/` directory for detailed guides
- **PX4 Community**: https://discuss.px4.io/
- **Hardware Issues**: Review `hardware/openfc2040-official/` repository
- **Debugging**: Use SWD debugger (see `docs/DEBUG_GUIDE.md`)

---

## Conclusion

The project is in a **strong position** with:
- Working build system
- Comprehensive documentation
- Clear path forward
- Well-defined priorities

**Immediate focus**: Complete console access and fix critical battery voltage divider issue.

**Next milestone**: Hardware peripheral verification complete.

---

**Last Updated**: Current session  
**Next Review**: After completing Phase 1

