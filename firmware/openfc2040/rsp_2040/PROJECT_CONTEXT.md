# OpenFC2040 Flight Controller - Complete Project Context

**Date**: October 30, 2024  
**Project**: PX4-based flight controller firmware for custom OpenFC2040 board  
**Status**: ✅ **BUILD SUCCESSFUL** - Firmware compiled and flashed to board!  
**Location**: `firmware/openfc2040/rsp_2040` (from the repository root)

---

## 🎯 Project Objective

Build custom PX4 Autopilot firmware for the OpenFC2040 flight controller board based on Raspberry Pi RP2040 microcontroller. The goal is to create a fully functional flight controller with proper hardware integration for all onboard components.

---

## 🏗️ Project Structure

```
firmware/openfc2040/rsp_2040/
├── board/                          # OpenFC2040 board configuration
│   ├── src/
│   │   ├── board_config.h         # ✅ Main hardware configuration
│   │   ├── init.c                  # Board initialization (not used - conflicts)
│   │   ├── spi.cpp                 # SPI config (not used - conflicts)
│   │   ├── i2c.cpp                 # I2C config (not used - conflicts)
│   │   └── ...                     # Other board files
│   ├── default.px4board            # ✅ PX4 build configuration
│   ├── nuttx-config/               # NuttX RTOS configuration
│   └── board.cmake                 # CMake board definition (attempted, not working)
│
├── drivers/                         # Custom drivers
│   └── imu/st/lsm6ds3/             # ❌ Incomplete LSM6DS3 driver (missing .cpp)
│
├── docs/                            # Documentation
│   ├── HARDWARE_ANALYSIS.md        # Complete component analysis
│   ├── GPIO_PIN_MAPPING.md         # Detailed GPIO assignments
│   ├── BUZZER_CONFIGURATION.md     # Buzzer specifications
│   └── ...
│
├── scripts/                         # Build automation
│   ├── setup.sh                    # ✅ First-time setup
│   ├── build.sh                    # ✅ Build firmware
│   ├── flash.sh                    # Flash to board
│   └── clean.sh                    # Clean build
│
├── px4-autopilot/                   # PX4 source (cloned by setup.sh)
│   └── boards/raspberrypi/pico/    # ✅ Using this as base board
│
├── HARDWARE_REVIEW_SUMMARY.md       # Hardware review results
├── QUICK_START_AFTER_REVIEW.md      # Quick start guide
└── PROJECT_CONTEXT.md               # ← This file

```

---

## 🔧 Hardware Specifications

### Board: OpenFC2040
- **MCU**: Raspberry Pi RP2040
  - Dual Cortex-M0+ @ 133MHz
  - 264KB SRAM
  - No onboard flash (uses external)
  
### Components Identified (from BOM analysis)

| Component | Part Number | Function | GPIO | Status |
|-----------|-------------|----------|------|--------|
| **External Flash** | W25Q128JVSIQTR | 16MB QSPI NOR | QSPI pins | ✅ Configured |
| **IMU** | LSM6DS3TR-C | 6-axis gyro/accel | CS=GPIO9 | ⚠️ Driver incomplete |
| **Barometer** | DPS310XTSA1 | Pressure sensor | CS=GPIO12 | ✅ Driver available |
| **MicroSD** | 503398-1892 | SD card slot | GPIO16-19 | ✅ Configured |
| **RGB LED** | FM-B2020RGBA-HG | Common-anode LED | GPIO13/14/15 | ✅ Fixed polarity |
| **Buzzer** | QMB-09B-03 | Passive 2700Hz | GPIO25 | ⚠️ No driver (removed) |

### Complete GPIO Pin Mapping

| GPIO | Function | Peripheral | Interface | Verified |
|------|----------|------------|-----------|----------|
| 0-1 | UART0 TX/RX | Telemetry | UART | ✅ |
| 2-3 | I2C1 SDA/SCL | Telemetry I2C | I2C | ✅ |
| 4-5 | UART1 TX/RX | GPS | UART | ✅ |
| 6-7 | I2C0 SDA/SCL | GPS/Magnetometer | I2C | ✅ |
| 8 | SPI0 MISO | IMU/Baro | SPI | ✅ |
| 9 | SPI0 CS | IMU (LSM6DS3) | SPI | ✅ |
| 10 | SPI0 SCK | IMU/Baro | SPI | ✅ |
| 11 | SPI0 MOSI | IMU/Baro | SPI | ✅ |
| 12 | SPI0 CS | Barometer (DPS310) | SPI | ✅ |
| 13 | PWM | RGB LED Red | GPIO | ✅ Active LOW |
| 14 | PWM | RGB LED Green | GPIO | ✅ Active LOW |
| 15 | PWM | RGB LED Blue | GPIO | ✅ Active LOW |
| 16 | SPI1 CS/MISO | SD Card | SPI | ⚠️ Assumed |
| 17 | GPIO | SD Card Detect | GPIO | ⚠️ Assumed |
| 18 | SPI1 SCK | SD Card | SPI | ⚠️ Assumed |
| 19 | SPI1 MOSI | SD Card | SPI | ⚠️ Assumed |
| 20 | PWM | ESC 1 (Motor 1) | PWM | ✅ |
| 21 | PWM | ESC 2 (Motor 2) | PWM | ✅ |
| 22 | PWM | ESC 3 (Motor 3) | PWM | ✅ |
| 23 | PWM | ESC 4 (Motor 4) | PWM | ✅ |
| 24 | GPIO/PWM | RC Input (PPM) | PWM | ✅ |
| 25 | PWM | Buzzer | PWM | ⚠️ No driver |
| 26 | ADC0 | Reserved/Debug | ADC | - |
| 27 | ADC1 | Battery Voltage | ADC | ⚠️ Needs calibration |
| 28 | ADC2 | Battery Current | ADC | ⚠️ Needs calibration |
| 29 | ADC3 | USB VBUS Detect | ADC | ✅ |

---

## 🚨 Critical Issues & Solutions

### 1. **RGB LED Polarity** - ✅ FIXED
**Problem**: FM-B2020RGBA-HG is common-anode (active LOW), was configured as active HIGH  
**Solution**: Changed to `PX4_MAKE_GPIO_OUTPUT_SET` in board_config.h  
**Status**: ✅ Fixed

### 2. **MicroSD Card Driver** - ✅ ADDED
**Problem**: No SD card driver configured  
**Solution**: Added SD card configuration to default.px4board and board_config.h  
**Status**: ✅ Configured, needs testing

### 3. **LSM6DS3 IMU Driver** - ❌ INCOMPLETE
**Problem**: Custom driver missing LSM6DS3.cpp implementation file  
**Solution**: Temporarily disabled in build  
**Status**: ⚠️ Need to find/create complete driver OR use alternative IMU  
**Action Required**: Either implement LSM6DS3.cpp or use a different IMU driver

### 4. **Tone Alarm / Buzzer** - ⚠️ REMOVED
**Problem**: PX4's tone_alarm driver requires arch-specific support not available for RP2040  
**Solution**: Removed from configuration  
**Status**: ⚠️ Buzzer control must be done via direct GPIO PWM (not through PX4 driver)  
**Workaround**: Can control buzzer directly in application code via PWM at 2700Hz

### 5. **Battery Voltage Divider** - ⚠️ NEEDS VERIFICATION
**Problem**: BOM shows divider ratio of 1.74:1 (max 5.74V), but 4S LiPo = 16.8V max  
**Status**: ⚠️ CRITICAL - Must verify actual resistor network from schematic  
**Action Required**: Check schematic, measure actual divider, update `BOARD_BATTERY_V_DIV`

### 6. **Custom Board Files Conflict** - ✅ RESOLVED
**Problem**: Custom init.c, spi.cpp, i2c.cpp incompatible with PX4 Pico board code  
**Solution**: Only copy board_config.h, use Pico's original source files  
**Status**: ✅ Fixed in latest build script

---

## 🛠️ Build System Status

### Current Approach
Using PX4's **raspberrypi_pico** board as base, customizing only:
1. `board_config.h` - Hardware pin definitions
2. `default.px4board` - Driver and module selection

### Why This Approach?
- ✅ Pico board has complete NuttX configuration for RP2040
- ✅ Avoids NuttX path and configuration issues
- ✅ Leverages tested RP2040 support in PX4
- ✅ Only need to customize GPIO pins and drivers

### Build Commands

```bash
# Navigate to project (from repo root)
cd firmware/openfc2040/rsp_2040

# First time setup (downloads PX4, takes 5-10 min)
./scripts/setup.sh

# Build firmware (10-20 min first time)
./scripts/build.sh all

# Flash to board (hold BOOTSEL, connect USB)
./scripts/flash.sh
```

### Build Target
- **Target**: `raspberrypi_pico_default`
- **Output**: `px4-autopilot/build/raspberrypi_pico_default/raspberrypi_pico_default.uf2`
- **Platform**: NuttX RTOS on RP2040

---

## 📝 Configuration Files Status

### ✅ Correctly Configured

**`board/src/board_config.h`**:
- ✅ RGB LED pins (GPIO13/14/15) - Active LOW
- ✅ ADC channels (GPIO27/28/29)
- ✅ PWM outputs (GPIO20-23) for ESCs
- ✅ RC input (GPIO24)
- ✅ SPI0 bus (IMU + Baro)
- ✅ SD card SPI1 bus (GPIO16-19)
- ✅ External flash config (W25Q128)
- ✅ UART/I2C assignments
- ⚠️ Battery voltage divider (needs verification)

**`board/default.px4board`**:
- ✅ DPS310 barometer driver enabled
- ✅ GPS driver enabled
- ✅ PWM output enabled
- ✅ RC input enabled
- ✅ RGB LED enabled
- ✅ SD card driver enabled
- ✅ External flash driver enabled
- ❌ LSM6DS3 IMU driver disabled (incomplete)
- ❌ Tone alarm disabled (not supported)

### ⚠️ Needs Attention

**LSM6DS3 Driver** (`drivers/imu/st/lsm6ds3/`):
- ✅ Has: LSM6DS3.hpp, lsm6ds3_main.cpp, ST_LSM6DS3_Registers.hpp
- ❌ Missing: LSM6DS3.cpp (main implementation)
- **Status**: Cannot build with this driver enabled

---

## 🔄 Build Process Flow

### Setup Phase (setup.sh)
1. Check dependencies (git, cmake, gcc-arm, python3, empy, yaml)
2. Clone PX4-Autopilot from GitHub
3. Initialize all PX4 submodules
4. Copy board configuration files
5. Ready to build

### Build Phase (build.sh)
1. Copy board_config.h to `px4-autopilot/boards/raspberrypi/pico/src/`
2. Copy default.px4board to `px4-autopilot/boards/raspberrypi/pico/`
3. Run CMake configuration
4. Build with ninja (uses 22 threads)
5. Generate UF2 firmware file

### Current Build Status
- ✅ Setup complete
- ✅ CMake configuration succeeds
- ✅ **Build completed successfully** (Oct 30, 2024 17:42)
- ✅ **UF2 file generated** (2.1MB)
- ✅ **Firmware automatically flashed to board** (detected in bootloader mode)

### Build Output Files
```
px4-autopilot/build/raspberrypi_pico_default/
├── raspberrypi_pico_default.bin    # 1.1MB - Raw binary
├── raspberrypi_pico_default.elf    # 24MB - Debug symbols
├── raspberrypi_pico_default.map    # 4.4MB - Memory map
├── raspberrypi_pico_default.px4    # 1.1MB - PX4 format
└── raspberrypi_pico_default.uf2    # 2.1MB - ✅ Flash format for RP2040
```

**Firmware Size**: 1073KB / 2048KB (52% used) - Good headroom for features!

---

## 🐛 Recent Issues & Resolutions

### Issue 1: Python empy Module
**Error**: `ModuleNotFoundError: No module named 'empy'`  
**Root Cause**: Package is `python3-empy` but module imports as `em`  
**Fix**: Changed setup.sh to check for `import em` instead of `import empy`  
**Status**: ✅ Resolved

### Issue 2: Board Path Issues
**Error**: `cannot stat '/board/rsp_2040/*'`  
**Root Cause**: Incorrect directory structure in scripts  
**Fix**: Updated paths to `/board/*` and destination to `boards/rsp_2040/rsp_2040/`  
**Status**: ✅ Resolved

### Issue 3: Build Target Not Found
**Error**: `Make target rsp_2040_default not found`  
**Root Cause**: PX4 expects `vendor_model_label` format  
**Fix**: Changed to use existing `raspberrypi_pico_default` target  
**Status**: ✅ Resolved

### Issue 4: NuttX Configuration Errors
**Error**: `NuttX custom board directory isn't in board directory`  
**Root Cause**: Trying to create custom board instead of using Pico  
**Fix**: Switched to customizing existing Pico board  
**Status**: ✅ Resolved

### Issue 5: LSM6DS3 Driver Missing Files
**Error**: `Cannot find source file: LSM6DS3.cpp`  
**Root Cause**: Incomplete driver - only header files present  
**Fix**: Disabled LSM6DS3 driver in default.px4board  
**Status**: ⚠️ Temporary workaround, need complete driver

### Issue 6: Compilation Errors in Board Files
**Error**: Various errors in init.c, spi.cpp, i2c.cpp  
**Root Cause**: Custom board files incompatible with Pico board code  
**Fix**: Only copy board_config.h, use Pico's source files  
**Status**: ✅ Resolved

### Issue 7: UF2 Generation Failed
**Error**: "Binary file not found, cannot generate UF2"  
**Root Cause**: Script using wrong variable names (old BOARD_NAME instead of BOARD_TARGET)  
**Fix**: Updated build script to use BOARD_TARGET, downloaded uf2conv.py tool  
**Solution**: Generated UF2 manually using uf2conv.py from Microsoft UF2 repo  
**Status**: ✅ Resolved - UF2 generated successfully and auto-flashed to board!

### Issue 8: Serial Console Shows Corrupted Data
**Error**: Garbled characters on serial console at all baud rates (115200, 57600, 9600, 230400)  
**Observation**: 
- USB device detected as "Linux-USB Serial Gadget (CDC ACM mode)" 
- /dev/ttyACM0 exists and is accessible
- Data appears to be transmitting but corrupted/wrong format
**Root Cause**: **INVESTIGATING** - Possible causes:
1. Firmware crashing/rebooting continuously
2. Hardware incompatibility with our customized board_config.h
3. Clock/PLL configuration issue
4. Missing critical hardware initialization

**Troubleshooting Steps Taken**:
1. ✅ Tried multiple baud rates - all show corruption
2. ✅ Backed up custom files to `.openfc2040` extension
3. ✅ Restored original Pico board configuration
4. ✅ Built clean original PX4 Pico firmware (1113KB, 53% flash)
5. ⏳ **NEXT**: Flash original firmware to test if hardware works

**Files Backed Up**:
- `boards/raspberrypi/pico/default.px4board.openfc2040` - Our custom config
- `boards/raspberrypi/pico/src/board_config.h.openfc2040` - Our custom header

**Test Firmware Created**:
- Location: `/rsp_2040/px4-autopilot/build/raspberrypi_pico_default/raspberrypi_pico_default_ORIGINAL.uf2`
- Size: 2.2MB
- Config: Original PX4 Pico (GPIO25 LED, default pins)
- Purpose: Verify hardware functionality

**Status**: ⚠️ **TESTING IN PROGRESS** - Need to flash original firmware

### Issue 9: ✅ **BREAKTHROUGH** - Hardware Confirmed Working!
**Discovery**: User has working test firmware (`/peripherals_testing/`) that successfully:
- ✅ Communicates via USB serial at 115200 baud (same as PX4 attempts)
- ✅ All peripherals work (IMU, Baro, LEDs, Buzzer, ESCs)
- ✅ Uses exact same GPIO pins
- ✅ Clean console output with commands

**Proven Working Configuration** (from peripherals_testing):
```c
// LEDs: GPIO13/14/15 (active LOW) - ✅ WORKS
// IMU CS: GPIO9, Baro CS: GPIO12 - ✅ WORKS  
// SPI: SCK=GPIO10, MOSI=GPIO11, MISO=GPIO8 - ✅ WORKS
// ESCs: GPIO20-23 - ✅ WORKS
// RC: GPIO24, Buzzer: GPIO25 - ✅ WORKS
// USB Serial: 115200 baud stdio_usb - ✅ WORKS PERFECTLY
```

**Conclusion**: **Hardware is 100% functional**. The issue is PX4/NuttX-specific.

**Root Cause Analysis**:
- Working firmware uses: Pico SDK `stdio_usb` (simple, direct USB CDC)
- PX4 uses: NuttX `CONFIG_NSH_USBCONSOLE` + `CONFIG_CDCACM` (complex USB stack)
- Possible issues:
  1. NuttX CDCACM driver initialization order
  2. Console buffer corruption (`BOARD_CONSOLE_BUFFER_SIZE`)
  3. NuttX trying to output before USB is ready
  4. Mismatch in stdout/stdin device assignment

**Next Action**: Compare working Pico SDK USB setup vs PX4 NuttX USB configuration

---

## 🎯 Next Steps (Prioritized)

### ✅ Build Phase - COMPLETED!
1. ✅ **Stop copying incompatible source files** - DONE
2. ✅ **Run build** - DONE
3. ✅ **Resolve compile errors** - DONE
4. ✅ **Generate UF2 firmware** - DONE
5. ✅ **Flash to board** - DONE (auto-flashed)

### 🔄 Current Phase: Debugging USB Console - Hardware Confirmed Working!

**BREAKTHROUGH**: User's test firmware (`/peripherals_testing/`) proves hardware is 100% functional with USB serial at 115200 baud!

**Root Cause**: Issue is PX4/NuttX USB console configuration, NOT hardware.

**IMMEDIATE ACTIONS**:

1. ⏳ **Test Original PX4 Pico Firmware**
   - Flash: `/rsp_2040/px4-autopilot/build/.../raspberrypi_pico_default_ORIGINAL.uf2`
   - Determine if issue is our custom config or general PX4 RP2040 problem

2. ⏳ **Alternative: Try UART Console** (if USB fails)
   - PX4 can use UART0 (GPIO0/1) instead of USB
   - Connect USB-to-Serial adapter to GPIO0 (TX) and GPIO1 (RX)
   - May be more reliable than USB CDC-ACM

**Suspected Issues**:
- NuttX `CONFIG_DEV_CONSOLE` is disabled but USB console enabled
- Console buffer (3KB) may be corrupting output
- USB initialization timing vs boot messages

**Original Plan (Resume After Fix)**:
1. Connect to serial console - Test NSH shell access
2. Verify boot sequence - Check for errors in dmesg
3. Test basic commands - ver, free, top, uorb top
4. Test RGB LEDs - led_control test (verify active LOW works)
5. Check sensor detection - Look for DPS310 barometer

### 🆕 Session Summary — November 1, 2025
- Built Picoprobe debug firmware for RP2350 (Pico 2) and confirmed USB VID/PID `2e8a:000c`.
- Resolved OpenOCD connection by specifying CMSIS-DAP VID/PID and lowering SWD clock; wiring fix (add VTREF) enabled stable SWD link.
- Attached GDB via `gdb-multiarch`; captured register/memory dumps but cores remained in hard-fault state when halted too early.
- Determined debugger must break after firmware runs (`continue` → manual interrupt) to avoid lockup vector (`pc=0xfffffffe`).
- Identified need for minimal USB test build (pico_debug) but build blocked by missing board CMake integration; next session should finish that or diff PX4 USB init against working `peripherals_testing` firmware.

### 🔜 Next Work Session Checklist
1. **SWD Bring-up** *(Recommended first)*
   - Use board's SWD/SWCLK header with Picoprobe/J-Link
   - Capture boot logs and inspect USB CDC initialization path
2. **USB Console Investigation**
   - Review NuttX `rp2040_usb.c` init timing around CDC ACM
   - Identify where garbled data originates (buffer vs driver)
3. **Optional UART Console Re-Test**
   - If deeper debug needed, reapply UART console config (commands logged in `BUILD_LOG.md`)
   - Connect USB-to-UART adapter to GPIO0/1 (only when telemetry disconnected)
4. **After Console Fixed**
   - Resume original testing plan (LEDs, sensors, SD)
   - Start migrating working drivers from `/peripherals_testing/`

### Short Term (Hardware Bring-Up)
1. ⚠️ **Verify battery voltage divider** from schematic (CRITICAL)
2. ⚠️ **Confirm SD card GPIO pins** (16-19) from schematic  
3. ⚠️ **Test SD card** - sd_bench command
4. **Test PWM outputs** - pwm test (NO PROPELLERS!)
5. **Test RC input** - Connect receiver, check input_rc
6. **Verify UART communication** - GPS and telemetry ports
7. **Test DPS310 barometer** - dps310 start, dps310 status

### Medium Term (Driver Integration)
1. ⚠️ **Find/implement complete LSM6DS3 driver** OR switch to different IMU
2. **Implement buzzer control** - Direct PWM at 2700Hz (not via tone_alarm)
3. **Full sensor calibration** - Accel, gyro, mag (if added)
4. **Test all communication interfaces** - UART, I2C, SPI
5. **Battery monitoring calibration** - After verifying divider

### Long Term (Full Integration)
1. Add IMU driver (LSM6DS3 or alternative)
2. Test barometer (DPS310)
3. Implement buzzer control (direct PWM, not tone_alarm)
4. Full sensor calibration
5. Test flight modes
6. Configure failsafes
7. Mission planning and autonomous flight

---

## 📚 Key Documentation

### Created Documents
1. **HARDWARE_ANALYSIS.md** - Complete BOM analysis, component specs
2. **GPIO_PIN_MAPPING.md** - Every GPIO pin documented with connectors
3. **BUZZER_CONFIGURATION.md** - Passive buzzer specifications (2700Hz)
4. **HARDWARE_REVIEW_SUMMARY.md** - Summary of hardware review
5. **QUICK_START_AFTER_REVIEW.md** - Quick build/flash guide
6. **PROJECT_CONTEXT.md** - This file (complete project context)

### External References
- PX4 Documentation: https://docs.px4.io/
- RP2040 Datasheet: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf
- PX4 GitHub: https://github.com/PX4/PX4-Autopilot
- Raspberry Pi Pico Board in PX4: `/px4-autopilot/boards/raspberrypi/pico/`

---

## 💻 Development Environment

### System
- **OS**: Ubuntu 24.04 (Linux)
- **Python**: 3.12.3
- **GCC ARM**: arm-none-eabi-gcc 13.2.1
- **CMake**: 3.28.3
- **Make**: GNU Make 4.3
- **Cores**: 22 (used for parallel builds)

### Dependencies Installed
```bash
sudo apt install git cmake make arm-none-eabi-gcc python3 python3-empy python3-yaml
```

### PX4 Version
- **Branch**: main
- **Version**: v1.17.0-alpha1-52-gf9595319b8
- **Cloned from**: https://github.com/PX4/PX4-Autopilot.git

---

## 🔍 Debugging Tips

### Check Build Status
```bash
cd px4-autopilot
make list_config_targets | grep pico
# Should show: raspberrypi_pico[_default]
```

### Clean Build
```bash
cd firmware/openfc2040/rsp_2040
./scripts/clean.sh
./scripts/build.sh all
```

### View Build Log
```bash
cd px4-autopilot
make raspberrypi_pico_default 2>&1 | tee build.log
```

### Check Board Configuration
```bash
cat px4-autopilot/boards/raspberrypi/pico/default.px4board
cat px4-autopilot/boards/raspberrypi/pico/src/board_config.h
```

---

## ⚙️ Configuration Parameters

### Build Configuration
- **Platform**: nuttx
- **Toolchain**: arm-none-eabi
- **Architecture**: cortex-m0plus
- **Flash**: Constrained (Yes)
- **Memory**: Constrained (Yes)
- **PWM Frequency**: 1MHz
- **Serial GPS**: /dev/ttyS1
- **Serial Telemetry**: /dev/ttyS0
- **Root Path**: /fs/microsd
- **Param File**: /fs/mtd_params

### Enabled Drivers
- ✅ ADC (board_adc)
- ✅ Barometer (DPS310)
- ✅ GPS
- ❌ IMU (LSM6DS3) - Disabled, incomplete driver
- ✅ PWM Output (4 channels)
- ✅ RC Input
- ✅ RGB LED
- ✅ SD Card
- ✅ External Flash (W25)
- ❌ Tone Alarm - Disabled, not supported

### Enabled Modules
- EKF2 (Extended Kalman Filter)
- Commander (main state machine)
- Navigator (mission handling)
- MC (Multicopter) controllers
- Battery status
- MAVLink
- Logger
- Sensors
- And more...

---

## 🎓 Learning & Notes

### Key Insights
1. **PX4 board structure**: vendor/model/label format
2. **NuttX integration**: Complex, better to use existing board as base
3. **RP2040 support**: Community-driven, Pico board is the reference
4. **Driver compatibility**: Can't just copy files between boards
5. **Build system**: CMake + Ninja, multi-stage process

### Mistakes Made & Lessons
1. ❌ Tried creating custom board from scratch → Should have used Pico as base from start
2. ❌ Copied incompatible source files → Only customize config files
3. ❌ Missing driver implementation → Verify all dependencies before enabling
4. ✅ Used existing community board → Much smoother process

### Best Practices
1. Always use existing similar board as base
2. Only override what's necessary (config files)
3. Test builds incrementally (disable features if needed)
4. Document everything (GPIOs, changes, issues)
5. Keep original files backed up

---

## 🚀 Quick Resume Guide

**If you need to continue this project later or hand it off:**

1. **Current State**: Build script updated, ready to compile
2. **Next Command**: `cd firmware/openfc2040/rsp_2040 && ./scripts/build.sh all`
3. **Expected**: Should compile successfully now (source file conflicts resolved)
4. **If Errors**: Check this document's "Recent Issues & Resolutions" section
5. **After Build**: Follow "Next Steps" section above

**Critical Files to Preserve**:
- `/board/src/board_config.h` - Hardware configuration
- `/board/default.px4board` - Build configuration  
- `/scripts/*.sh` - Build automation
- `/docs/*.md` - All documentation
- This file (`PROJECT_CONTEXT.md`)

**Don't Lose**:
- PCB design files in `/openFC2040-main/`
- Hardware analysis in `/docs/`
- All memory/notes about battery divider issue

---

## 📞 Support & Resources

### If Stuck
1. Check PX4 Slack: https://px4.io/slack
2. PX4 Forum: https://discuss.px4.io/
3. GitHub Issues: https://github.com/PX4/PX4-Autopilot/issues
4. RP2040 specific: Look at Pico board discussions

### Useful Commands
```bash
# Show all available boards
make list_config_targets

# Build specific board
make raspberrypi_pico_default

# Clean everything
make distclean

# Run specific test
make tests

# Flash via USB (UF2)
# Just copy .uf2 file to RPI-RP2 drive
```

---

---

## 🧪 IMMEDIATE NEXT STEPS - Testing Commands

### Step 1: Connect to Serial Console
```bash
# Install screen if needed
sudo apt install screen

# Connect via USB serial (board should be running now)
screen /dev/ttyACM0 115200

# Alternative
sudo minicom -D /dev/ttyACM0 -b 115200
```

**Expected**: NuttShell prompt `nsh>`

### Step 2: Basic System Check
```bash
# In NSH console:
ver all              # Show firmware version
free                 # Memory usage
top                  # CPU usage
dmesg                # Boot messages - check for errors!
uorb top             # Message bus activity
```

### Step 3: Test RGB LED (Verify Active LOW Fix)
```bash
led_control test     # Should cycle R→G→B
led_control on       # Turn on
led_control off      # Turn off
led_control blink    # Blink pattern
```

**Expected**: LEDs work correctly (not inverted)

### Step 4: Test Barometer
```bash
dps310 start -s      # Start on SPI bus
dps310 status        # Check if detected
listener sensor_baro # Read pressure data
```

### Step 5: Test PWM (⚠️ NO PROPELLERS!)
```bash
pwm test -c 1 -p 1100  # Motor 1 min throttle
pwm test -c 2 -p 1100  # Motor 2
pwm test -c 3 -p 1100  # Motor 3  
pwm test -c 4 -p 1100  # Motor 4
pwm idle               # Return to idle
```

### Step 6: Check SD Card
```bash
mount                # Check if SD mounted
sd_bench             # Test read/write speed
ls -la /fs/microsd   # List SD card files
```

### Step 7: Check Sensors
```bash
sensors status       # Show all detected sensors
```

**Expected Output**:
- ✅ Baro0: DPS310 (SPI) - OK
- ⚠️ No IMU (disabled)
- ⚠️ No magnetometer (not connected)

---

**Last Updated**: October 30, 2024, 10:50 PM IST  
**Last Milestone**: ✅ **HARDWARE CONFIRMED WORKING** - Test firmware works perfectly!  
**Current Action**: Diagnosing PX4/NuttX USB console configuration issue  
**Project Phase**: Root Cause Analysis & Fix Implementation  
**Completion**: ~40% (Build successful, hardware verified, fixing console config)

**Key Documents**:
- `PROJECT_CONTEXT.md` - Complete project state (this file)
- `MIGRATION_PLAN.md` - **NEW** - Strategy to port working config to PX4
- `DIAGNOSIS_SUMMARY.md` - Detailed USB console issue analysis
- `TESTING_STATUS.md` - Testing procedures
- `/peripherals_testing/` - ✅ **Working test firmware** (reference implementation)
- `tools/debuggers/picoprobe/` - Picoprobe (debug firmware sources and builds)

**New Strategy**: Incrementally adapt PX4 to match the proven-working peripherals_testing configuration (using dedicated SWD header for low-level debugging)

**Debug Interface Update**:
- Board exposes dedicated **SWD/SWCLK** header wired directly to RP2040 SWD pins.
- Use this header with a SWD debugger (Picoprobe/J-Link/etc.) for low-level access.
- **UART0 pins (GPIO0/1) stay reserved for telemetry** as originally planned.

---

## ⚡ IMMEDIATE ACTION REQUIRED

### Flash Original Firmware Test

**File to flash**: `px4-autopilot/build/raspberrypi_pico_default/raspberrypi_pico_default_ORIGINAL.uf2`

**Steps**:
1. Hold BOOTSEL button on board
2. Connect USB cable
3. Board appears as `RPI-RP2` drive
4. Copy `raspberrypi_pico_default_ORIGINAL.uf2` to the drive
5. Board will reboot automatically
6. Connect serial: `picocom -b 115200 /dev/ttyACM0`

**Expected Results**:
- ✅ If console works → Problem is in our custom configuration
- ❌ If still corrupted → Possible hardware issue or deeper problem

**Key Differences in Original vs Custom**:
- Original: LED on GPIO25 (onboard Pico LED)
- Custom: RGB LEDs on GPIO13/14/15
- Original: RC input on GPIO16
- Custom: RC input on GPIO24
- Original: No SD card/flash configuration
- Custom: Added SD card, DPS310, battery monitoring

---

*This document contains all context needed to resume this project. Read it thoroughly before making changes.*
