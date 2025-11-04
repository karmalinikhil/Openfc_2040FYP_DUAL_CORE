# Next Steps

> ⚠️ **CRITICAL**: This file MUST be updated after EVERY commit!  
> **Last Updated**: November 1, 2025  
> **Updated By**: Development Team

---

## 🔴 IMPORTANT: How to Use This File

This document tracks:
1. **What was done last** - So you know where we left off
2. **What to do next** - Clear actionable steps
3. **Current blockers** - What's preventing progress

**After every work session**:
1. Update "What Was Done Last" section
2. Move completed items from "Next Actions" to "What Was Done Last"
3. Add new items to "Next Actions"
4. Update blockers
5. **Commit this file with your changes!**

---

## 📝 What Was Done Last

**Session Date**: November 4, 2025

### Completed Tasks

1. ✅ **Documentation Consolidation**
   - Merged scattered .md files from `firmware/` and `hardware/` into `docs/` folder
   - Updated `docs/README.md` with comprehensive navigation hub
   - Expanded `docs/PROJECT_CONTEXT.md` with firmware port overview, build workflows, flashing methods
   - Integrated hardware bring-up checklist, peripheral test firmware guide, and calibration workflow
   - Cleaned up `CONTRIBUTING.md` for clearer collaboration guidelines
   - Committed and pushed changes to main branch (commit: 1a7f970b)

2. ✅ **Workspace Reorganization** *(Previous session)*
   - Moved all projects to professional structure
   - Created firmware/, hardware/, tools/, docs/, testing/ directories
   - All functionality verified working after reorganization

3. ✅ **Essential Documentation Created** *(Previous session)*
   - `README.md`: Complete project overview with beginner-friendly setup guide
   - `docs/PROJECT_CONTEXT.md`: Detailed build process explanation (addresses "single file" misconception)
   - `docs/PIN_CONNECTIONS.md`: Complete GPIO mapping from hardware review
   - Removed 15+ unnecessary markdown files from development

4. ✅ **Hardware Repository Cloned** *(Previous session)*
   - Cloned official openFC2040 repo: git@github.com:vxj9800/openFC2040.git
   - Location: `hardware/openfc2040-official/`
   - Reviewed BOM, identified all components

5. ✅ **UART Console Configuration** *(Previous session)*
   - Built minimal firmware with UART console: `raspberrypi_pico_minimal_UART.uf2`
   - Size: 1.1 MB (52.41% flash, 6.44% SRAM)
   - Console on GPIO0/GPIO1, 115200 baud
   - Ready to flash and test (blocked on USB-Serial adapter)

### Issues Encountered

1. **USB Console Unusable** *(Previous session)*:
   - Tested both configurations (CONFIG_DEV_CONSOLE enabled/disabled)
   - Result: Garbled output on /dev/ttyACM0
   - Root cause: NuttX USB CDC-ACM driver unreliable on RP2040
   - Solution: Use UART console (industry standard)
   - Documented in: `firmware/openfc2040/rsp_2040/USB_CONSOLE_INVESTIGATION.md`

2. **Battery Voltage Divider Concerns** *(Previous session)*:
   - Current firmware config: `BOARD_BATTERY_V_DIV = 1.74` (max 5.74V)
   - From BOM: R3=27.4Ω, R4=27.4Ω (need to verify circuit)
   - **Problem**: 4S LiPo max = 16.8V, will exceed 3.3V ADC limit!
   - **Action**: Need schematic review to determine actual divider ratio

---

## 🎯 Next Actions (Priority Order)

### Priority 1: BLOCKING (Do First!)

#### 1. Connect JST-GH6 Telemetry Port
**Status**: IN PROGRESS  
**Time**: 1-2 hours  
**Session Date**: November 4, 2025

**Objective**: Access UART telemetry pins via JST-GH 6-pin connector to enable debugging console.

**Hardware Required**:
- JST-GH 6-pin connector (SM06B-GHS-TB) or breakout cable
- USB-to-Serial adapter (FTDI FT232RL, CP2102, or CH340G)
- Jumper wires (if using breakout)

**Telemetry Port Pinout** (from `docs/PIN_CONNECTIONS.md`):

| Pin | Signal | GPIO | Direction | Voltage | Purpose |
|-----|--------|------|-----------|---------|---------|
| 1 | GND | - | - | 0V | Common ground |
| 2 | TX | GPIO0 | Output | 3.3V | Board transmits to adapter RX |
| 3 | RX | GPIO1 | Input | 3.3V | Board receives from adapter TX |
| 4 | SDA/CTS | GPIO2 | Bidir | 3.3V | Optional I2C or flow control |
| 5 | SCL/RTS | GPIO3 | Bidir | 3.3V | Optional I2C or flow control |
| 6 | +5V | - | Power | 5V | Power output (from VBUS/Battery) |

**Wiring Diagram**:
```
OpenFC2040 Telemetry (JST-GH6)          USB-Serial Adapter
================================        ==================
Pin 1: GND       ------------------>    GND
Pin 2: TX (GPIO0) ----------------->    RX
Pin 3: RX (GPIO1) ----------------->    TX
Pin 4: SDA/CTS    (leave disconnected for basic console)
Pin 5: SCL/RTS    (leave disconnected for basic console)
Pin 6: +5V        (DO NOT CONNECT - adapter has own power)
```

**Steps**:
1. **Identify JST-GH6 connector** on OpenFC2040 board (labeled "TELEM" or "UART0")
2. **Option A - Breakout cable**:
   - Use JST-GH to Dupont breakout cable
   - Connect individual wires to USB-Serial adapter
3. **Option B - Direct solder**:
   - Solder thin wires directly to JST-GH pads (Pin 1, 2, 3)
   - Use heat shrink or electrical tape for insulation
4. **Connect USB-Serial adapter**:
   - Adapter GND → Pin 1 (GND)
   - Adapter RX → Pin 2 (TX/GPIO0)
   - Adapter TX → Pin 3 (RX/GPIO1)
   - **Do NOT connect +5V pin** (avoid ground loops)
5. **Verify connections** with multimeter:
   - Check continuity between adapter GND and board GND
   - Ensure no shorts between TX/RX pins
6. **Power on OpenFC2040** (via USB or battery)
7. **Connect adapter** to PC via USB
8. **Open serial terminal**:
   ```bash
   picocom -b 115200 /dev/ttyUSB0 --imap lfcrlf
   ```
   or
   ```bash
   screen /dev/ttyUSB0 115200
   ```

**Expected Result**:
- NSH prompt appears: `nsh>`
- No garbled characters
- Commands respond (try `help`, `ver`, `free`)

**Success Criteria**:
- [ ] JST-GH6 connector accessible via breakout or direct wiring
- [ ] USB-Serial adapter connected to GPIO0 (TX) and GPIO1 (RX)
- [ ] Clean console output in terminal (115200 baud, 8N1)
- [ ] NSH shell responds to commands
- [ ] Connection stable for extended debugging sessions

**Blockers**:
- None (hardware in hand)

**Next After This**:
- Run hardware peripheral tests (LED, PWM, sensors)
- Continue with barometer/IMU driver debugging

---

#### 2. Hardware BOM Deep Review
**Status**: Started (BOM cloned, needs analysis)  
**Time**: 3-4 hours  
**Assigned**: Hardware team

**Tasks**:
- [ ] Parse complete BOM CSV
- [ ] Extract all resistor values (R3, R4, R6, R7, R14-R17)
- [ ] Identify current sensor IC (if any)
- [ ] Calculate battery voltage divider ratio
- [ ] Verify LED resistor values
- [ ] Cross-reference with schematic
- [ ] Update `docs/PIN_CONNECTIONS.md` with verified values
- [ ] Update `board/board_config.h` with correct divider ratio

**Success criteria**:
- Battery voltage divider safe for 4S LiPo (16.8V max)
- All component specifications documented
- Firmware config matches hardware

#### 3. UART Console Verification (Depends on Task #1)
**Status**: Ready to test (once JST-GH6 connected)  
**Time**: 30 minutes  
**Depends on**: Task #1 (JST-GH6 telemetry connection)

**Prerequisites**:
- JST-GH6 breakout or direct wiring complete
- USB-Serial adapter connected
- Firmware flashed: `firmware/openfc2040/rsp_2040/raspberrypi_pico_minimal_UART.uf2`

**Steps**:
1. Wire adapter to OpenFC2040:
   - Adapter TX → Board GPIO1 (RX)
   - Adapter RX → Board GPIO0 (TX)
   - Adapter GND → Board GND
2. Flash firmware: `firmware/openfc2040/rsp_2040/raspberrypi_pico_minimal_UART.uf2`
3. Connect: `picocom -b 115200 /dev/ttyUSB0 --imap lfcrlf`
4. Test commands:
   ```
   nsh> ver all       # Verify firmware version
   nsh> free          # Check memory usage
   nsh> dmesg         # Review boot log
   nsh> help          # List available commands
   ```

**Success criteria**:
- Clean console output (no garbled text)
- NSH prompt appears
- All commands respond correctly
- Boot log shows no critical errors

---

### Priority 2: Testing & Verification

#### 4. Hardware Peripheral Tests (Depends on Task #3)
**Status**: Not started  
**Time**: 4-6 hours  
**Depends on**: Task #3 (UART console working)

**Test sequence**:

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
   - Check for errors

5. **RC Input Test**:
   ```
   nsh> input_rc status
   ```
   - Connect RC receiver to GPIO24
   - Verify channel values update

**Success criteria**:
- All peripherals respond without errors
- All sensor data looks reasonable
- No hardware faults in dmesg

#### 5. Fix Battery Voltage Divider
**Status**: Identified as critical issue  
**Time**: 2-3 hours  
**Priority**: CRITICAL (risk of ADC damage)

**Current situation**:
- Firmware config: `BOARD_BATTERY_V_DIV = 1.74` (max input: 5.74V)
- 4S LiPo range: 14.8V nominal, 16.8V fully charged
- **Problem**: 16.8V × 1.74 = **29.23V at ADC!** (Will damage RP2040!)

**Action plan**:
1. Review schematic (`hardware/openfc2040-official/openFC2040.json`)
2. Identify resistor network for battery voltage sense
3. Calculate actual divider ratio:
   ```
   V_adc = V_battery × (R_bottom / (R_top + R_bottom))
   ```
4. For 16.8V max input, 3.3V ADC:
   ```
   Required divider: 16.8V / 3.3V = 5.09 minimum
   ```
5. Update `firmware/openfc2040/rsp_2040/board/board_config.h`:
   ```c
   #define BOARD_BATTERY_V_DIV (calculated_value)
   ```
6. Test with multimeter and known voltage source

**Success criteria**:
- ADC reading matches multimeter (±0.1V)
- 16.8V input reads correctly
- No ADC saturation

### Priority 3: Driver Development

#### 6. Port LSM6DS3 IMU Driver
**Status**: Not started  
**Time**: 6-8 hours  
**Depends on**: Task #3 (console working)

**Source**: `firmware/test-firmware/peripherals_testing/src/main.c` (working driver)

**Steps**:
1. Create driver structure:
   ```
   firmware/openfc2040/rsp_2040/drivers/imu/st/lsm6ds3/
   ├── LSM6DS3.cpp
   ├── LSM6DS3.hpp
   └── CMakeLists.txt
   ```
2. Port SPI communication code
3. Implement PX4 driver interface (poll(), ioctl(), etc.)
4. Add uORB topic publication (sensor_accel, sensor_gyro)
5. Add calibration constants
6. Test on hardware

**Success criteria**:
- Driver compiles and links
- `lsm6ds3 start` succeeds
- `lsm6ds3 status` shows device detected
- `listener sensor_accel` shows ~9.8 m/s² gravity
- `listener sensor_gyro` shows ~0 when stationary

#### 7. Verify All Sensors
**Status**: Not started  
**Time**: 2-3 hours  
**Depends on**: Task #6 (IMU driver)

**Test each sensor**:
- IMU (LSM6DS3): Gyro + Accelerometer
- Barometer (DPS310): Pressure + Temperature
- Battery voltage (ADC1): Voltage reading
- Battery current (ADC2): Current reading (if sensor installed)

**Calibration**:
```
nsh> commander calibrate accel
nsh> commander calibrate gyro
nsh> commander calibrate baro
```

### Priority 4: System Integration

#### 8. MAVLink Telemetry Test
**Status**: Not started  
**Time**: 2-3 hours  
**Depends on**: All sensors working

**Setup**:
1. Connect telemetry radio to UART0 (GPIO0/1)
2. Configure MAVLink:
   ```
   nsh> mavlink start -d /dev/ttyS0 -b 57600
   ```
3. Connect QGroundControl on PC
4. Verify all sensor data visible

#### 9. Full Integration Test
**Status**: Not started  
**Time**: 4-6 hours  
**Depends on**: All above tasks complete

**Test checklist**:
- [ ] All sensors publishing data
- [ ] QGroundControl connects
- [ ] Can arm/disarm via RC
- [ ] PWM outputs respond to stick input (NO PROPS!)
- [ ] SD logging works
- [ ] Battery voltage displayed correctly
- [ ] All pre-flight checks pass

**Success criteria**:
- Ready for bench testing with motors (NO FLIGHT)
- All systems green in QGroundControl

---

## 🚧 Current Blockers

### Active Work
1. **JST-GH6 Telemetry Connection (IN PROGRESS)**
   - Status: Currently connecting JST-GH6 to USB adapter
   - Impact: Once complete, will enable UART console access
   - **Resolution**: In progress - Session Date: November 4, 2025
   - **Next**: Test console connection, then proceed to peripheral tests

### Blocking Testing (Until Console Access)
2. **Hardware Peripheral Tests**
   - Impact: Cannot test LED, PWM, sensors without console
   - Cannot verify firmware is running correctly
   - **Resolution**: Blocked on Task #1 (JST-GH6 connection)

### Blocking All Testing (Previous - resolved with JST-GH6 approach)
~~1. **No USB-to-Serial Adapter**~~
   - **Update**: Using JST-GH6 telemetry port instead
   - JST-GH6 provides direct access to UART0 (GPIO0/GPIO1)
   - No separate adapter needed if using breakout cable

### Critical Safety Issues
3. **Battery Voltage Divider Incorrect**
   - Impact: Risk of ADC damage from overvoltage
   - Cannot safely connect 4S LiPo battery
   - **Resolution**: Review schematic, calculate correct divider ratio
   - **Priority**: CRITICAL - do before battery testing!

### Missing Drivers
4. **LSM6DS3 IMU Driver Not Ported**
   - Impact: No gyro/accelerometer data
   - Cannot run attitude estimation
   - Cannot fly without IMU
   - **Resolution**: Port driver from test firmware (6-8 hours)

---

## 📊 Project Health

### What's Working ✅
- Build system (PX4 compiles for RP2040)
- UART console configuration (115200 baud)
- SWD debugging infrastructure (OpenOCD + GDB)
- Minimal firmware builds successfully (1.1 MB)
- All GPIO pins mapped and documented

### What's Not Working ❌
- USB console (garbled output - driver issue)
- Hardware testing (blocked on serial adapter)
- IMU driver (not ported yet)
- Battery monitoring (config likely incorrect)

### Risk Level: 🟡 MEDIUM
- **Red flags**: Battery voltage divider potentially dangerous
- **Yellow flags**: Testing blocked on hardware acquisition
- **Green lights**: Build system stable, documentation complete

---

## 📅 Milestones

### Milestone 1: Console Access ⏳
**Target**: November 5, 2025  
**Status**: 80% complete (JST-GH6 connection in progress)  
**Blockers**: Task #1 completion (JST-GH6 wiring)

### Milestone 2: Hardware Verification ⏳
**Target**: November 5, 2025  
**Status**: 20% complete (pins mapped, drivers needed)  
**Blockers**: Console access, IMU driver

### Milestone 3: Full System Integration ⏳
**Target**: November 15, 2025  
**Status**: 10% complete (architecture in place)  
**Blockers**: All sensors working, telemetry tested

---

## 🔄 How to Update This File

### Template for "What Was Done Last"

```markdown
**Session Date**: YYYY-MM-DD

### Completed Tasks
1. ✅ **Task Name**
   - What was accomplished
   - Files changed
   - Results/output

### Issues Encountered
1. **Issue Description**
   - What went wrong
   - Root cause
   - Solution/workaround
   - Where documented
```

### Template for "Next Actions"

```markdown
#### Task Number. Task Name
**Status**: Not started / In progress / Blocked  
**Time**: Estimated hours  
**Depends on**: Other task numbers  
**Assigned**: Team member (if applicable)

**Steps**:
- [ ] Step 1
- [ ] Step 2

**Success criteria**:
- Specific, testable outcome
```

### Commit Message Format

When updating this file:
```
docs: update NEXT_STEPS after [what you did]

- Completed: [list tasks]
- Added: [new tasks]
- Blocked on: [blockers]
```

---

## 📞 Questions or Issues?

- **Build problems**: See [README.md](../README.md) Troubleshooting
- **Hardware questions**: See [PIN_CONNECTIONS.md](PIN_CONNECTIONS.md)
- **Architecture questions**: See [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md)
- **Debugging help**: See [DEBUG_GUIDE.md](DEBUG_GUIDE.md)

---

**Remember**: Update this file after EVERY work session! 🔄
