# OpenFC2040 Flight Controller (RP2040 + PX4)

An open-source flight controller based on the RP2040 microcontroller running PX4 Autopilot firmware. This repository contains the PX4-Autopilot source with board-specific configurations for the OpenFC2040 hardware.

## Project Status

**Working Features:**
- ✅ PX4 firmware builds and runs on RP2040
- ✅ UART console (GPIO0/GPIO1) at 115200 baud
- ✅ RGB LED control via `led_control` command (GPIO13/14/15, active-low)
- ✅ MAVLink communication ready

**In Progress:**
- 🔄 DPS310 Baro and ICM42688 IMU integration
- 🔄 QGroundControl connection testing

## Repository Structure

```
OpenFC2040_FYP/
├── PX4-Autopilot/           # PX4 firmware source (submodule)
│   └── boards/raspberrypi/pico/  # RP2040 board config
├── docs/                    # Documentation
│   ├── DPS310_SPI_FIX_REPORT.md  # Barometer SPI fix details
│   ├── PIN_CONNECTIONS.md        # Hardware pin mapping
│   ├── DEBUG_GUIDE.md            # SWD debugging guide
│   └── ...
├── peripheral_testing/      # Standalone sensor test code
└── CONTRIBUTING.md
```

## Quick Start

### 1. Install Prerequisites (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi gdb-multiarch cmake ninja-build git python3 python3-pip
sudo apt install picocom   # optional: serial console
```

### 2. Clone and Initialize

```bash
git clone --recursive https://github.com/Rispats/OpenFC2040_FYP.git
cd OpenFC2040_FYP
```

### 3. Build PX4 Firmware

```bash
cd PX4-Autopilot
make raspberrypi_pico_default
```

### 4. Flash the Board

```bash
# Convert .bin to .uf2 (if needed)
python3 /tmp/bin2uf2.py build/raspberrypi_pico_default/raspberrypi_pico_default.bin \
                         build/raspberrypi_pico_default/raspberrypi_pico_default.uf2

# Hold BOOTSEL while connecting USB, then:
cp build/raspberrypi_pico_default/raspberrypi_pico_default.uf2 /media/$USER/RPI-RP2/
```

### 5. Access the Console

```bash
# UART console via USB-to-Serial adapter on GPIO0 (TX) / GPIO1 (RX)
picocom -b 115200 /dev/ttyUSB0
```

## Hardware Connections

| Function | RP2040 GPIO | Notes |
|----------|-------------|-------|
| UART TX  | GPIO 0      | Console output |
| UART RX  | GPIO 1      | Console input |
| SPI SCK  | GPIO 2      | Sensors |
| SPI MOSI | GPIO 3      | Sensors |
| SPI MISO | GPIO 4      | Sensors |
| BARO CS  | GPIO 5      | DPS310 |
| IMU CS   | GPIO 9      | ICM42688 |
| LED RED  | GPIO 13     | Active-low |
| LED GREEN| GPIO 14     | Active-low |
| LED BLUE | GPIO 15     | Active-low |

See `docs/PIN_CONNECTIONS.md` for complete pinout.

## NSH Commands

# Control LEDs
nsh> led_control on -c red
nsh> led_control on -c green
nsh> led_control on -c blue
nsh> led_control on -c white
nsh> led_control test

# System info
nsh> ver all
nsh> top
```

## Documentation

- `docs/PIN_CONNECTIONS.md` - Complete hardware pin mapping
- `docs/DEBUG_GUIDE.md` - SWD debugging with Picoprobe/OpenOCD
- `docs/PROJECT_CONTEXT.md` - Build system and architecture overview
- `docs/GIT_QUICK_REFERENCE.md` - Git workflow tips

## Notes

- The RP2040 board uses ~6.5% of flash (~1.1MB firmware)
- UART console is the primary debug interface (USB CDC not yet working)
- All RGB LEDs are active-low (write LOW to turn ON)

## License and Credits

- **PX4 Autopilot**: BSD-3-Clause
- **NuttX RTOS**: Apache-2.0
- **Hardware Design**: OpenFC2040 by Vatsal Joshi (@vxj9800)
