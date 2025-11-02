# OpenFC2040 Flight Controller Development# OpenFC2040 Flight Controller Development - Final Year Project



**Open-source PX4 Autopilot on RP2040 MCU****Project**: Custom PX4-based Flight Controller for RP2040 Microcontroller  

**Board**: OpenFC2040 (Designed by Vatsal Joshi)  

> 🎓 **Final Year Project**: Custom flight controller firmware development  **Status**: ✅ Firmware builds successfully, UART console configured  

> 🔧 **Hardware**: OpenFC2040 by [@vxj9800](https://github.com/vxj9800/openFC2040)  **Last Updated**: 2024-11-01

> 🚁 **Firmware**: PX4 Autopilot v1.17.0-alpha1 on NuttX RTOS

---

---

## 🎯 Project Overview

## 📖 Table of Contents

This repository contains the complete development workspace for creating PX4 Autopilot firmware for the OpenFC2040 flight controller - a custom RP2040-based board designed for autonomous drone applications.

- [Project Overview](#project-overview)

- [Quick Navigation](#quick-navigation)**What is this?** (For beginners)

- [Getting Started](#getting-started)- **OpenFC2040** = Custom circuit board with RP2040 chip + sensors for flying drones

- [Hardware Setup](#hardware-setup)- **PX4 Firmware** = Software that runs on the board to control flight

- [Configuration](#configuration)- **This Project** = Building custom PX4 firmware that works with OpenFC2040 hardware

- [Troubleshooting](#troubleshooting)

- [Contributing](#contributing)---

- [Project Status](#project-status)

## 📁 Repository Structure

---

```

## 🎯 Project OverviewFYP/

├── README.md                          ← You are here

This project brings **PX4 Autopilot** to the **Raspberry Pi RP2040** microcontroller using the **OpenFC2040** custom flight controller board.├── QUICK_START.md                     ← Start here for setup

├── CONTRIBUTING.md                    ← Team collaboration guide

### Key Features│

├── firmware/                          📱 Firmware Development

- ✅ **Sensors**: LSM6DS3 IMU (6-axis), DPS310 Barometer│   ├── openfc2040/                   ← Main PX4 firmware project

- ✅ **Storage**: MicroSD card, 16MB SPI Flash (W25Q128JV)│   │   ├── README.md                 ← Firmware-specific documentation

- ✅ **Communication**: USB, UART telemetry, GPS port│   │   ├── px4-autopilot/            ← PX4 source code

- ✅ **Outputs**: 4x PWM ESC, RC input, RGB LED│   │   ├── board/                    ← OpenFC2040 board configuration

- ✅ **Debugging**: SWD interface│   │   ├── drivers/                  ← Custom drivers

- ✅ **Power**: Battery voltage/current monitoring│   │   ├── scripts/                  ← Build/flash automation

│   │   └── docs/                     ← Technical documentation

---│   │

│   ├── test-firmware/                ← Peripheral testing firmware

## 🗂️ Quick Navigation│   │   └── peripherals_testing/      ← Pico SDK test code

│   │

### Directory Structure│   └── builds/                       ← Compiled firmware (.uf2 files)

│

```├── hardware/                          🔧 Hardware Documentation

FYP/│   ├── openfc2040-design/            ← Original board design files

├── README.md                      ← You are here│   ├── schematics/                   ← Circuit schematics

├── CONTRIBUTING.md                ← Git workflow & collaboration│   ├── datasheets/                   ← Component datasheets

├── firmware/│   ├── bom/                          ← Bill of materials

│   ├── openfc2040/               ← Main PX4 firmware│   └── openfc2040_pinout.txt         ← Pin assignments

│   └── test-firmware/            ← Peripheral tests│

├── hardware/├── tools/                             🛠️ Development Tools

│   ├── openfc2040-official/      ← Hardware repo (vxj9800)│   ├── debuggers/                    ← SWD debugger tools

│   └── openfc2040-design/        ← Local hardware files│   │   ├── debugprobe/               ← Picoprobe firmware

├── tools/│   │   ├── picoprobe/                ← Alternative debugger

│   ├── debuggers/                ← Picoprobe, OpenOCD│   │   └── openocd_configs/          ← OpenOCD configurations

│   └── utilities/                ← Picotool│   │

├── docs/                          ← Start here!│   └── utilities/                    ← Helper utilities

│   ├── PROJECT_CONTEXT.md        ← Architecture & build explained│       └── picotool/                 ← RP2040 flash utility

│   ├── PIN_CONNECTIONS.md        ← Complete GPIO mapping│

│   ├── NEXT_STEPS.md             ← Current status├── testing/                           🧪 Testing & Validation

│   ├── DEBUG_GUIDE.md            ← SWD debugging│   ├── bench-tests/                  ← Hardware bench tests

│   └── CONTRIBUTING.md           ← How to collaborate│   └── results/                      ← Test results & logs

├── testing/                       ← Test procedures│

└── scripts/                       ← Automation├── docs/                              📚 Documentation

```│   ├── README.md                     ← Documentation index

│   ├── PROJECT_STATUS.md             ← Current project state

### Essential Reading (In Order)│   ├── ARCHITECTURE.md               ← System architecture

│   │

1. **This README** - Project overview and quick start│   ├── getting-started/              ← New user guides

2. [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) - **Read this to understand the build process!**│   ├── hardware/                     ← Hardware documentation

3. [`docs/PIN_CONNECTIONS.md`](docs/PIN_CONNECTIONS.md) - GPIO mappings│   ├── development/                  ← Developer guides

4. [`docs/DEBUG_GUIDE.md`](docs/DEBUG_GUIDE.md) - Debugging setup│   ├── testing/                      ← Testing procedures

5. [`docs/NEXT_STEPS.md`](docs/NEXT_STEPS.md) - What to do next│   └── reference/                    ← Technical references

6. [`CONTRIBUTING.md`](CONTRIBUTING.md) - Git workflow│

└── scripts/                           ⚙️ Automation Scripts

---    ├── setup_workspace.sh            ← First-time setup

    ├── build_all.sh                  ← Build all projects

## 🚀 Getting Started    └── clean_all.sh                  ← Clean build artifacts

```

### Prerequisites (Linux)

---

Install ARM toolchain and build tools:

## 🚀 Quick Start

```bash

# ARM cross-compiler### For New Users (First Time Setup)

sudo apt update

sudo apt install gcc-arm-none-eabi**Prerequisites**: Linux (Ubuntu 20.04+ recommended)



# Build tools```bash

sudo apt install cmake ninja-build git python3 python3-pip# 1. Navigate to FYP directory

cd /home/rishi-patil/Desktop/FYP

# PX4 dependencies

pip3 install --user pyros-genmsg setuptools empy toml numpy packaging jinja2 jsonschema# 2. Read the quick start guide

sudo apt install genromfs kconfig-frontendscat QUICK_START.md



# Debugging tools (optional)# 3. Run workspace setup

sudo apt install openocd gdb-multiarch./scripts/setup_workspace.sh



# Serial terminal# 4. Build firmware

sudo apt install picocomcd firmware/openfc2040

sudo usermod -aG dialout $USER  # Log out and back in./scripts/build.sh

```

# 5. Flash to board

**What each tool does**: See [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md)./scripts/flash.sh

```

### First-Time Setup

**Estimated time**: 30 minutes (first time)

```bash

cd ~/Desktop/FYP/firmware/openfc2040/rsp_2040### For Returning Users



# Initialize PX4 submodules (takes 5-10 minutes)```bash

git submodule update --init --recursivecd /home/rishi-patil/Desktop/FYP/firmware/openfc2040

./scripts/build.sh           # Build firmware

# Verify setup./scripts/flash.sh           # Flash to board

ls px4-autopilot/  # Should see boards/, src/, etc.```

```

---

### Building Firmware

## 📖 Documentation

```bash

cd firmware/openfc2040/rsp_2040### Essential Reading (Start Here)



# Quick build (recommended)1. **[QUICK_START.md](QUICK_START.md)** - Get up and running in 30 minutes

./scripts/build.sh all2. **[docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md)** - Current state, what works, what doesn't

3. **[firmware/openfc2040/README.md](firmware/openfc2040/README.md)** - Firmware-specific guide

# Output: px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.uf24. **[CONTRIBUTING.md](CONTRIBUTING.md)** - How to collaborate effectively

```

### By Topic

**First build**: 5-15 minutes  

**Incremental builds**: 30 seconds**Getting Started:**

- [Dependencies Guide](docs/getting-started/DEPENDENCIES.md) - What each tool does and why

**⚠️ Important**: There is NO "single file that compiles everything"!  - [First Build](docs/getting-started/FIRST_BUILD.md) - Step-by-step build guide

The build process uses CMake + Ninja to compile ~500 files.  - [Troubleshooting](docs/getting-started/TROUBLESHOOTING.md) - Common issues

See [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) for detailed explanation.

**Hardware:**

### Flashing to Board- [GPIO Pin Mapping](docs/hardware/GPIO_PIN_MAPPING.md) - Complete pin assignments

- [UART Console Setup](docs/hardware/UART_CONSOLE_SETUP.md) - Console access

```bash- [Component Details](docs/hardware/COMPONENT_VERIFICATION.md) - Hardware verification

# 1. Hold BOOTSEL button on board

# 2. Connect USB cable**Development:**

# 3. Release BOOTSEL (board appears as RPI-RP2 drive)- [Architecture Overview](docs/ARCHITECTURE.md) - How PX4/NuttX/RP2040 fit together

- [Build System Explained](docs/BUILD_SYSTEM_EXPLAINED.md) - Understanding compilation

# 4. Flash firmware- [Debugging Guide](docs/development/DEBUGGING.md) - SWD/OpenOCD/GDB setup

cp px4-autopilot/build/raspberrypi_pico_minimal/raspberrypi_pico_minimal.uf2 /media/$USER/RPI-RP2/- [Next Steps](docs/development/NEXT_STEPS.md) - **⚠️ Update after every commit!**



# Or use helper script**Testing:**

./scripts/flash.sh- [Test Procedures](docs/testing/TEST_PROCEDURES.md) - Systematic testing checklist

```

---

---

## 🎯 Project Status

## 🔌 Hardware Setup

**Last Updated**: 2024-11-01

### UART Console (Required)

| Component | Status | Notes |

**Why**: USB console on RP2040/NuttX is unreliable (garbled output).|-----------|--------|-------|

| **Firmware Build** | ✅ Working | Minimal firmware compiles successfully |

**You need**:| **Console Access** | ✅ Working | UART on GPIO0/1 (USB console unreliable) |

- USB-to-Serial adapter (FTDI FT232RL, CP2102, or CH340G)| **Board Config** | ✅ Complete | All GPIO pins mapped |

- 3x jumper wires| **RGB LEDs** | ✅ Fixed | Active-LOW polarity corrected |

| **IMU Driver** | ⚠️ In Progress | LSM6DS3 needs porting (30%) |

**Wiring**:| **Barometer** | ✅ Available | DPS310 driver ready |

| Adapter | OpenFC2040 || **PWM Outputs** | ✅ Configured | Needs bench testing |

|---------|------------|| **SD Card** | ⚠️ Untested | Configuration exists |

| TX      | GPIO1 (RX) || **Battery Monitor** | ⚠️ Needs Verify | Voltage divider may be incorrect |

| RX      | GPIO0 (TX) |

| GND     | GND        |**Current Blocker**: USB-to-Serial adapter needed for console testing (~$10, 1-day delivery)



**Connect**:**See**: [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md) for detailed status

```bash

picocom -b 115200 /dev/ttyUSB0 --imap lfcrlf---



# You should see:## 🔧 Hardware Specifications

# nsh>

```### OpenFC2040 Board



**Test**:**Microcontroller**: Raspberry Pi RP2040

```bash- Dual-core ARM Cortex-M0+ @ 133MHz

nsh> ver all    # Show version- 264KB SRAM

nsh> free       # Memory usage- 2MB external flash (W25Q128JV)

nsh> dmesg      # Boot log

```**Sensors**:

- IMU: LSM6DS3TR-C (6-axis accelerometer/gyroscope)

See [`firmware/openfc2040/rsp_2040/UART_CONNECTION_GUIDE.md`](firmware/openfc2040/rsp_2040/UART_CONNECTION_GUIDE.md)- Barometer: DPS310 (pressure sensor)



### SWD Debugging**Interfaces**:

- 4x PWM outputs (ESC control)

See [`docs/DEBUG_GUIDE.md`](docs/DEBUG_GUIDE.md) for complete setup.- 2x UART (Telemetry, GPS)

- 2x I2C (GPS I2C, Telemetry I2C)

---- 1x SPI (Sensors)

- 1x RC Input (PPM/SBUS)

## ⚙️ Configuration- USB Full-Speed

- RGB LED

### Changing Build Configuration- Passive Buzzer

- Battery voltage/current sensing

```bash

# Edit defconfig**See**: [hardware/openfc2040_pinout.txt](hardware/openfc2040_pinout.txt) for complete pin mapping

nano boards/raspberrypi/pico/nuttx-config/nsh/defconfig

---

# Rebuild

./scripts/build.sh clean## 🛠️ Development Tools

./scripts/build.sh all

```**Required**:

- arm-none-eabi-gcc (ARM cross-compiler)

### Pin Assignments- cmake & ninja-build (Build system)

- python3 (Build scripts)

See [`docs/PIN_CONNECTIONS.md`](docs/PIN_CONNECTIONS.md) for complete GPIO mapping.- git (Version control)



To modify: Edit `board/board_config.h`**Optional**:

- Picoprobe/Debugprobe (SWD debugging)

---- OpenOCD (Debug interface)

- gdb-multiarch (Debugger)

## 🐛 Troubleshooting- USB-to-Serial adapter (Console access)



### Build Fails**Installation**: See [docs/getting-started/DEPENDENCIES.md](docs/getting-started/DEPENDENCIES.md)



**"arm-none-eabi-gcc not found"**:---

```bash

sudo apt install gcc-arm-none-eabi## 🧪 Testing

```

**Current Test Status**:

**"CMake version too old"**:- ✅ Build system verified

```bash- ✅ Firmware flashing works

cmake --version  # Need 3.16+- ⏳ Console access (waiting for adapter)

sudo snap install cmake --classic- ⏳ Peripheral testing (pending console)

```- ⏳ Flight testing (future)



**"Ninja build failed"**:**Test Procedures**: [docs/testing/TEST_PROCEDURES.md](docs/testing/TEST_PROCEDURES.md)

```bash

./scripts/build.sh clean---

./scripts/build.sh all

```## 🤝 Contributing



### Flash Fails**This is a team project!** Please follow these guidelines:



- Hold BOOTSEL button BEFORE connecting USB1. **Read first**: [CONTRIBUTING.md](CONTRIBUTING.md)

- Try different USB cable2. **Update always**: [docs/development/NEXT_STEPS.md](docs/development/NEXT_STEPS.md) after every commit

- Verify file is `.uf2` format3. **Follow workflow**: Create branch → Make changes → Update docs → Commit → Push

4. **Test before commit**: Ensure firmware builds successfully

### Console Issues

**Git Workflow**:

**No output**:```bash

- Verify TX→RX, RX→TX crossovergit pull origin main                    # Get latest

- Check baud rate: 115200git checkout -b feature/your-feature    # Create branch

- Test adapter with loopback# Make changes

./scripts/build.sh                      # Test build

**Permission denied**:# Update docs/development/NEXT_STEPS.md

```bashgit add .

sudo usermod -aG dialout $USERgit commit -m "feat: Your feature"

# Log out and back ingit push origin feature/your-feature

``````



### More Help---



See detailed troubleshooting in [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md)## 📚 Learning Resources



---**New to flight controllers?**

- [PX4 Documentation](https://docs.px4.io/)

## 🤝 Contributing- [Understanding Flight Controllers](https://oscarliang.com/flight-controller/)



**Read [`CONTRIBUTING.md`](CONTRIBUTING.md) first!****New to RP2040?**

- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)

Quick workflow:- [Getting Started with Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)



1. Create branch: `git checkout -b feature/your-feature`**New to embedded development?**

2. Make changes and test on hardware- [Embedded C Programming](https://www.embedded.com/electronics-blogs/beginners-corner/)

3. **Update `docs/NEXT_STEPS.md`** ⚠️ CRITICAL!- [ARM Cortex-M Guide](https://interrupt.memfault.com/blog/arm-cortex-m-rtos-context-switching)

4. Commit: `git commit -m "feat: descriptive message"`

5. Push: `git push origin feature/your-feature`---



### Commit Message Format## 🐛 Troubleshooting



```**Common Issues**:

feat: add LSM6DS3 IMU driver

### Build Errors

- Implement SPI communication- **Missing dependencies**: Run `./scripts/setup_workspace.sh`

- Add calibration routine- **"region 'flash' overflowed"**: Firmware too large, disable modules in .px4board

- Verified on hardware- **Git submodule errors**: Run `git submodule update --init --recursive`



Next: Add magnetometer support### Flashing Issues

```- **Board not detected**: Hold BOOTSEL while connecting USB

- **Flash fails**: Ensure `/media/RPI-RP2` is mounted

---- **Permission denied**: Add user to dialout group



## 📊 Project Status### Console Issues

- **Garbled USB output**: Use UART console (GPIO0/GPIO1) instead

**Last Updated**: November 1, 2025- **No /dev/ttyUSB0**: USB-to-Serial adapter not connected or no drivers

- **Permission denied**: `sudo usermod -a -G dialout $USER`, then logout/login

### ✅ Completed

- PX4 builds for RP2040**More help**: [docs/getting-started/TROUBLESHOOTING.md](docs/getting-started/TROUBLESHOOTING.md)

- UART console working

- SWD debugging setup---

- Minimal firmware (1.1MB, 52% flash)

## 📞 Support & Contact

### 🚧 In Progress

- Essential documentation**For issues**:

- Hardware review1. Check [TROUBLESHOOTING.md](docs/getting-started/TROUBLESHOOTING.md)

2. Review [docs/development/NEXT_STEPS.md](docs/development/NEXT_STEPS.md) for known issues

### ⏳ Blocked3. Check investigation docs in [docs/development/archived/](docs/development/archived/)

- **UART testing** - Need USB-to-Serial adapter4. Ask team members



### 📝 Next**For contributing**:

- Port LSM6DS3 driver- See [CONTRIBUTING.md](CONTRIBUTING.md)

- Verify battery voltage divider- Follow Git workflow

- Test peripherals (LEDs, PWM, RC)- Update NEXT_STEPS.md



**See [`docs/NEXT_STEPS.md`](docs/NEXT_STEPS.md) for details**---



---## 📄 License



## 📚 Resources[Add license information]



### Hardware---

- **OpenFC2040 Repository**: https://github.com/vxj9800/openFC2040

- **RP2040 Datasheet**: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf## 🙏 Credits



### Firmware- **Board Design**: Vatsal Joshi (OpenFC2040)

- **PX4 Autopilot**: https://px4.io/- **PX4 Autopilot**: [PX4 Development Team](https://px4.io/)

- **PX4 Dev Guide**: https://docs.px4.io/main/en/- **RP2040**: [Raspberry Pi Foundation](https://www.raspberrypi.com/)

- **NuttX RTOS**: https://nuttx.apache.org/

---

### Tools

- **Picoprobe**: https://github.com/raspberrypi/picoprobe## 📊 Project Timeline

- **OpenOCD**: https://openocd.org/

- **Oct 2024**: Project started, hardware analysis

---- **Oct 2024**: Build system configured, initial firmware built

- **Nov 2024**: USB console investigation, switched to UART

## 📄 License- **Nov 2024**: Documentation reorganization, workspace refinement

- **Future**: IMU driver, peripheral testing, flight testing

- **PX4 Autopilot**: BSD 3-Clause

- **NuttX RTOS**: Apache 2.0---

- **OpenFC2040 Hardware**: See hardware repo

- **Custom firmware**: MIT License**Ready to contribute?** Start with [QUICK_START.md](QUICK_START.md) and [CONTRIBUTING.md](CONTRIBUTING.md)! 🚀


---

**Happy Flying!** 🚁

**Next Steps**: Read [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) to understand how the build system works!
