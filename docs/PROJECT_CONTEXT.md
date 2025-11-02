# OpenFC2040 Project Context

**Understanding the Architecture and Build Process**

> Critical reading: many collaborators assume a single file in VS Code builds the entire firmware. That is incorrect - this document explains the real build system.

---

## Table of Contents

- [The Build Process Myth](#the-build-process-myth)
- [How Firmware Compilation Actually Works](#how-firmware-compilation-actually-works)
- [What Each Dependency Does](#what-each-dependency-does)
- [Which Files Are Involved](#which-files-are-involved)
- [Current Project Status](#current-project-status)
- [Architecture Overview](#architecture-overview)
- [Next Milestones](#next-milestones)

---

## The Build Process Myth

### Common Misconception

"There's a single file you run in VSCode that compiles everything."

### Reality

Building PX4 firmware involves:
1. **CMake** reads hundreds of `CMakeLists.txt` files
2. **Ninja** compiles ~500 source files in parallel
3. **Linker** combines compiled objects into one `.elf` file
4. **Post-processing** converts `.elf` to `.uf2` format

**There is NO single magical file!**

---

## How Firmware Compilation Actually Works

### Step-by-Step Build Process

#### 1. Configuration Phase (CMake)

```bash
cd px4-autopilot
make raspberrypi_pico_minimal
```

**What happens**:
- CMake reads `platforms/nuttx/CMakeLists.txt`
- CMake reads `boards/raspberrypi/pico/CMakeLists.txt`
- CMake finds all source files (`src/**/*.cpp`, `src/**/*.c`)
- CMake generates `build.ninja` file with compilation rules

**Output**: `build/raspberrypi_pico_minimal/build.ninja` (20,000+ lines!)

#### 2. Compilation Phase (Ninja)

```bash
# Ninja is invoked automatically by CMake
ninja -C build/raspberrypi_pico_minimal
```

**What happens**:
- Ninja compiles each `.cpp`/`.c` file to `.o` object file
- Uses `arm-none-eabi-gcc` compiler
- Compiles in parallel (uses all CPU cores)
- ~500 files compiled

**Example of ONE compilation command**:
```bash
arm-none-eabi-gcc \
  -mcpu=cortex-m0plus \
  -mthumb \
  -O2 \
  -g \
  -I./platforms/nuttx/NuttX/nuttx/include \
  -I./platforms/common/include \
  -D__PX4_NUTTX \
  -c src/drivers/imu/st/lsm6ds3/LSM6DS3.cpp \
  -o build/raspberrypi_pico_minimal/src/drivers/imu/st/lsm6ds3/LSM6DS3.cpp.o
```

This happens **500+ times** for different files!

#### 3. Linking Phase

```bash
arm-none-eabi-ld -o build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.elf \
  build/**/*.o \
  -L./platforms/nuttx/NuttX/nuttx/libs \
  -lnuttx -lm -lgcc
```

**What happens**:
- Combines all `.o` files into single `.elf` executable
- Links against NuttX RTOS library
- Resolves all function calls between files
- **Output**: `raspberrypi_pico_minimal.elf` (~5 MB with debug symbols)

#### 4. Binary Generation

```bash
# Convert ELF to binary
arm-none-eabi-objcopy -O binary \
  raspberrypi_pico_minimal.elf \
  raspberrypi_pico_minimal.bin

# Convert binary to UF2 (RP2040 bootloader format)
python3 ../../Tools/uf2conv.py \
  raspberrypi_pico_minimal.bin \
  --base 0x10000000 \
  --family RP2040 \
  --output raspberrypi_pico_minimal.uf2
```

**Output**: `raspberrypi_pico_minimal.uf2` (~1.1 MB)

### Summary: Build Pipeline

```
Source Files (.cpp, .c)
         ↓
    [CMake reads CMakeLists.txt]
         ↓
    build.ninja generated
         ↓
    [Ninja invokes gcc ~500 times]
         ↓
    Object Files (.o)
         ↓
    [Linker combines all .o files]
         ↓
    ELF Executable (.elf)
         ↓
    [objcopy converts to binary]
         ↓
    Binary (.bin)
         ↓
    [uf2conv.py adds RP2040 header]
         ↓
    Flashable Firmware (.uf2)
```

**Total time**: 5-15 minutes first build, 30 seconds incremental

---

## What Each Dependency Does

### Build Toolchain

| Tool | Purpose | Why We Need It |
|------|---------|----------------|
| **arm-none-eabi-gcc** | Cross-compiler | Compiles C/C++ for ARM Cortex-M0+ (not x86 Linux) |
| **arm-none-eabi-ld** | Linker | Combines compiled objects into executable |
| **arm-none-eabi-objcopy** | Binary converter | Converts ELF to raw binary |
| **CMake** | Build system generator | Reads high-level build rules, generates Ninja files |
| **Ninja** | Build tool | Executes compilation commands in parallel |
| **Python 3** | Scripting | Runs build scripts, UF2 conversion, code generation |

### PX4-Specific Tools

| Tool | Purpose | Why We Need It |
|------|---------|----------------|
| **pyros-genmsg** | Message code generator | Generates C code from `.msg` message definitions |
| **empy** | Template engine | Processes code templates (generates MAVLink bindings) |
| **genromfs** | ROM filesystem builder | Creates read-only filesystem for startup scripts |
| **kconfig-frontends** | Configuration menu | Interactive configuration (like Linux kernel menuconfig) |

### Debugging Tools

| Tool | Purpose | Why We Need It |
|------|---------|----------------|
| **openocd** | SWD debugger interface | Connects to Picoprobe hardware debugger |
| **gdb-multiarch** | Debugger | Sets breakpoints, inspects variables, steps through code |
| **picocom** | Serial terminal | Connects to UART console for NSH shell |

---

## Which Files Are Involved

### Key Build Configuration Files

```
px4-autopilot/
├── CMakeLists.txt                           ← Top-level build config
├── boards/raspberrypi/pico/
│   ├── CMakeLists.txt                       ← Board-specific build rules
│   ├── default.px4board                     ← Board definition
│   ├── nuttx-config/
│   │   └── nsh/defconfig                    ← NuttX configuration
│   └── src/
│       ├── board_config.h                   ← Pin definitions, hardware config
│       ├── init.c                           ← Board initialization
│       └── CMakeLists.txt                   ← Board source build rules
├── platforms/nuttx/CMakeLists.txt           ← NuttX platform build rules
├── src/
│   ├── drivers/                             ← Driver source code
│   ├── modules/                             ← PX4 modules (estimator, navigator, etc.)
│   ├── systemcmds/                          ← NSH commands
│   └── lib/                                 ← Libraries (math, conversion, etc.)
└── Tools/
    └── uf2conv.py                           ← UF2 converter script
```

### Source Code Organization

**Drivers** (compiled to driver libraries):
```
src/drivers/
├── imu/st/lsm6ds3/                          ← IMU driver
│   ├── LSM6DS3.cpp
│   ├── LSM6DS3.hpp
│   └── CMakeLists.txt
├── barometer/dps310/                        ← Barometer driver
│   ├── DPS310.cpp
│   └── CMakeLists.txt
├── gps/                                     ← GPS drivers
└── rc/                                      ← RC input driver
```

**Modules** (PX4 autopilot logic):
```
src/modules/
├── sensors/                                 ← Sensor fusion
├── navigator/                               ← Flight path planning
├── mc_pos_control/                          ← Multicopter position control
├── mc_att_control/                          ← Attitude control
└── mavlink/                                 ← MAVLink telemetry
```

**Platform Code** (NuttX integration):
```
platforms/nuttx/
├── src/
│   ├── px4/                                 ← PX4-NuttX interface
│   ├── drivers/                             ← Platform drivers (GPIO, PWM)
│   └── px4_layer/                           ← Abstraction layer
└── NuttX/                                   ← NuttX RTOS (submodule)
    └── nuttx/
        ├── arch/arm/src/rp2040/             ← RP2040 architecture code
        ├── boards/arm/rp2040/               ← Board support
        └── drivers/                         ← NuttX drivers
```

### Configuration Flow

```
boards/raspberrypi/pico/nuttx-config/nsh/defconfig
                ↓
        [NuttX Kconfig]
                ↓
        Generated Config
                ↓
        Compiled into NuttX
```

**Key config options**:
```ini
# Console
CONFIG_UART0_SERIAL_CONSOLE=y              # Use UART for console
CONFIG_UART0_BAUD=115200                   # Baud rate

# Drivers
CONFIG_SENSORS_LSM6DS3=y                   # Enable IMU
CONFIG_SENSORS_DPS310=y                    # Enable barometer

# Filesystems
CONFIG_FS_FAT=y                            # Enable FAT for SD card
CONFIG_FS_ROMFS=y                          # Enable ROMFS for startup scripts
```

---

## Current Project Status

### What Works

1. **Build System**:
   - PX4 compiles successfully for RP2040
   - Generates valid `.uf2` firmware
   - Size: 1.1 MB (52% flash, 6.4% SRAM)

2. **Console**:
   - UART console configured (GPIO0/GPIO1, 115200 baud)
   - NSH shell available
   - Commands work: `help`, `ver all`, `free`, `dmesg`

3. **Debugging**:
   - SWD interface configured
   - OpenOCD connects via Picoprobe
   - GDB can halt/resume/step firmware

4. **Basic Drivers**:
   - GPIO configured for all peripherals
   - PWM outputs mapped (ESC0-3)
   - SPI bus configured (IMU, Barometer)
   - UART ports mapped (GPS, Telemetry)

### What Doesn't Work

1. **USB Console**:
   - Garbled output on `/dev/ttyACM0`
   - Root cause: NuttX USB CDC-ACM driver unreliable on RP2040
   - **Solution**: Use UART console (industry standard)

2. **Hardware Testing**:
   - **Blocked**: Need USB-to-Serial adapter for UART console
   - Cannot test sensors without console access
   - Cannot verify peripherals (LEDs, PWM, RC)

### Known Blockers

1. **UART Testing**:
   - Need: USB-to-Serial adapter ($3-10)
   - Recommended: FTDI FT232RL, CP2102, or CH340G
   - Wire: TX→GPIO1, RX→GPIO0, GND→GND

2. **Battery Voltage Divider**:
   - **CRITICAL**: Current config shows 1.74:1 divider (max 5.74V)
   - 4S LiPo max voltage: 16.8V
   - **Risk**: ADC will saturate or damage
   - **Action**: Review hardware schematic for actual divider ratio

3. **Missing Drivers**:
   - LSM6DS3 IMU: Needs porting from test firmware
   - SD card: Driver exists but untested
   - RC input: Mapped but untested

---

## Architecture Overview

### Hardware Layers

```
┌─────────────────────────────────────────────┐
│           PX4 Autopilot Application         │
│  (Navigator, Controllers, Estimators)       │
├─────────────────────────────────────────────┤
│           PX4 Middleware (uORB)             │
│  (Inter-module communication via topics)    │
├─────────────────────────────────────────────┤
│           PX4 Drivers                       │
│  (IMU, Baro, GPS, PWM, RC, SD)              │
├─────────────────────────────────────────────┤
│           NuttX Platform Layer              │
│  (SPI, UART, GPIO, PWM abstraction)         │
├─────────────────────────────────────────────┤
│           NuttX RTOS Kernel                 │
│  (Scheduler, Threads, Semaphores, IPC)      │
├─────────────────────────────────────────────┤
│           RP2040 HAL (Hardware Access)      │
│  (Register access, clocks, DMA)             │
├─────────────────────────────────────────────┤
│           RP2040 Hardware                   │
│  (Cortex-M0+ cores, peripherals, GPIO)      │
└─────────────────────────────────────────────┘
```

### PX4 → NuttX → RP2040 Mapping

| PX4 Component | NuttX Driver | RP2040 Peripheral | GPIO Pins |
|---------------|--------------|-------------------|-----------|
| IMU (LSM6DS3) | SPI1 driver | SPI1 | MOSI:11, MISO:8, SCK:10, CS:9 |
| Barometer (DPS310) | SPI1 driver | SPI1 (shared) | CS:12 |
| GPS | UART1 driver | UART1 | TX:4, RX:5 |
| Telemetry | UART0 driver | UART0 | TX:0, RX:1 |
| ESC0-3 (PWM) | PWM driver | PWM slices 2-5 | GPIO 20-23 |
| RC Input | GPIO + Timer | GPIO24 | GPIO24 |
| RGB LED | GPIO | GPIO 13-15 | R:13, G:14, B:15 |
| SD Card | SPI0 driver | SPI0 | TX:19, SCK:18, CS:17, RX:16 |
| Flash | QSPI driver | QSPI | QSPI_CS, QSPI_SD0-3, QSPI_SCLK |

### Boot Sequence

1. **RP2040 Bootloader** (ROM):
   - Loads firmware from flash to SRAM
   - Jumps to reset vector

2. **NuttX Startup**:
   - Initialize clocks and PLLs
   - Setup memory regions (SRAM, Flash)
   - Initialize hardware peripherals
   - Start scheduler

3. **PX4 Initialization** (`/etc/init.d/rcS`):
   - Mount filesystems (ROMFS, SD card)
   - Load board-specific config
   - Start system daemons

4. **Driver Initialization**:
   - Probe and initialize sensors
   - Start uORB topic publication
   - Initialize communication (MAVLink)

5. **NSH Shell Ready**:
   - User can interact via UART console
   - Can start/stop modules
   - Can test peripherals

---

## Next Milestones

### Milestone 1: Console Access (Priority 1 - BLOCKING)

**Goal**: Get UART console working for testing

**Tasks**:
- [ ] Acquire USB-to-Serial adapter
- [ ] Wire adapter to GPIO0 (TX), GPIO1 (RX), GND
- [ ] Connect via picocom
- [ ] Verify NSH prompt and commands

**Success Criteria**:
- Clean console output (no garbled characters)
- Can run `ver all`, `free`, `dmesg`
- Can start/stop modules

**Estimated Time**: 1 hour (once adapter arrives)

---

### Milestone 2: Hardware Peripheral Verification (Priority 1)

**Goal**: Verify all hardware peripherals work

**Tasks**:
- [ ] Test RGB LED: `led_control test`
- [ ] Test PWM outputs: `pwm test` (NO PROPS!)
- [ ] Test RC input: `input_rc status`
- [ ] Test IMU: `lsm6ds3 status` (if driver ported)
- [ ] Test Barometer: `dps310 status`
- [ ] Test SD card: `sd_bench`

**Success Criteria**:
- All peripherals respond correctly
- No hardware errors in dmesg
- Sensors publish valid data

**Estimated Time**: 4-6 hours

---

### Milestone 3: Battery Voltage Divider Fix (Priority 1 - CRITICAL)

**Goal**: Correctly measure battery voltage for 4S LiPo

**Tasks**:
- [ ] Review hardware schematic from openFC2040 repo
- [ ] Identify resistor values (R_top, R_bottom)
- [ ] Calculate actual divider ratio: V_div = (R_bottom) / (R_top + R_bottom)
- [ ] Calculate ADC scale factor for 16.8V max (4S LiPo)
- [ ] Update `BOARD_BATTERY_V_DIV` in `board_config.h`
- [ ] Test with multimeter and known voltage

**Success Criteria**:
- ADC reading matches multimeter (±0.1V)
- 16.8V LiPo reads correctly
- No ADC saturation

**Estimated Time**: 2-3 hours

---

### Milestone 4: LSM6DS3 IMU Driver Port (Priority 2)

**Goal**: Port working LSM6DS3 driver from test firmware

**Tasks**:
- [ ] Copy driver files from `firmware/test-firmware/peripherals_testing/src/main.c`
- [ ] Create PX4 driver structure (`src/drivers/imu/st/lsm6ds3/`)
- [ ] Implement uORB topic publication
- [ ] Add calibration constants
- [ ] Test gyro and accelerometer data
- [ ] Verify sample rates (ODR)

**Success Criteria**:
- Driver compiles and links
- Sensor detected on `lsm6ds3 status`
- Publishes to `sensor_accel` and `sensor_gyro` topics
- Data looks reasonable (gravity ~9.8 m/s², gyro at rest ~0)

**Estimated Time**: 6-8 hours

---

### Milestone 5: Full System Integration (Priority 3)

**Goal**: All systems operational for flight testing

**Tasks**:
- [ ] Verify all sensors publish data
- [ ] Test MAVLink telemetry
- [ ] Test SD card logging
- [ ] Calibrate sensors (accel, gyro, mag, baro)
- [ ] Test RC input and PWM outputs
- [ ] Bench test with QGroundControl
- [ ] Verify safety features (arming, disarming)

**Success Criteria**:
- QGroundControl connects via telemetry
- All sensors show green in QGC
- Can arm/disarm via RC
- Logs saved to SD card
- Ready for test stand (NO FLIGHT YET)

**Estimated Time**: 10-15 hours

---

## FAQs

### "Why can't I just click 'Build' in VSCode?"

You can! VSCode uses CMake extension which calls the same build process described above. But you need to understand what's happening under the hood to debug issues.

### "How do I add a new source file?"

1. Create `.cpp` file in appropriate directory (e.g., `src/drivers/my_driver/`)
2. Add to `CMakeLists.txt` in that directory:
   ```cmake
   px4_add_module(
     MODULE drivers__my_driver
     MAIN my_driver
     SRCS
       my_driver.cpp
   )
   ```
3. Rebuild: `./scripts/build.sh`

### "Build takes 15 minutes! Can I speed it up?"

First build is slow (compiles everything). Subsequent builds are fast (only recompiles changed files).

Speed up:
- Use all CPU cores: `./scripts/build.sh all -j $(nproc)`
- Use SSD (not HDD)
- Disable some PX4 modules in defconfig

### "What's the difference between .elf, .bin, and .uf2?"

- `.elf`: Executable with debug symbols (used by GDB)
- `.bin`: Raw binary data (machine code only)
- `.uf2`: Binary with RP2040 bootloader header (for UF2 flashing)

All three contain the same code, just different formats!

---

**Need More Help?**

- Build issues: See main [README.md](../README.md) Troubleshooting section
- Pin mappings: See [PIN_CONNECTIONS.md](PIN_CONNECTIONS.md)
- Debugging: See [DEBUG_GUIDE.md](DEBUG_GUIDE.md)
- Next actions: See [NEXT_STEPS.md](NEXT_STEPS.md)
