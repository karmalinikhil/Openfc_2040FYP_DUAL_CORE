# OpenFC2040 PX4 Firmware Port

## Overview

This directory contains a complete PX4 firmware port for the OpenFC2040 flight controller, a custom RP2040-based board designed for drone applications. The OpenFC2040 combines the Raspberry Pi RP2040 microcontroller with essential flight control sensors and interfaces.

## Hardware Specifications

### Microcontroller
- **MCU**: Raspberry Pi RP2040 (Dual-core ARM Cortex-M0+ @ 133MHz)
- **RAM**: 264KB SRAM
- **Flash**: 2MB external (W25Q128JV)

### Sensors
- **IMU**: LSM6DS3TR-C (6-axis accelerometer/gyroscope)
- **Barometer**: DPS310 (precision pressure sensor)

### Interfaces
- **PWM Outputs**: 4 channels for ESC control (GPIO20-23)
- **UART Ports**: 
  - Telemetry (GPIO0/1)
  - GPS (GPIO4/5)
- **I2C Buses**: 
  - GPS I2C (GPIO6/7)
  - Telemetry I2C (GPIO2/3)
- **RC Input**: GPIO24
- **USB**: Full-speed USB with VBUS detection
- **RGB LED**: GPIO13-15
- **Buzzer**: GPIO25
- **Battery Monitoring**: Voltage (GPIO27) and Current (GPIO28) sensing

## Quick Start

### Build and flash from this workspace

Run these commands from the repository root (`OpenFC2040_FYP`):

```bash
./scripts/setup_workspace.sh       # one-time dependency check and submodule sync
cd firmware/openfc2040
./scripts/build.sh                 # build PX4 firmware
./scripts/flash.sh                 # flash UF2 to the board (BOOTSEL mode)
```

### Incremental rebuild only

```bash
cd firmware/openfc2040
./scripts/build.sh
```

## Building the Firmware

### Prerequisites (Linux)

1. **ARM Toolchain Installation**
   ```bash
   # Ubuntu/Debian (20.04 or later)
   sudo apt-get update
   sudo apt-get install -y gcc-arm-none-eabi gdb-multiarch
   
   # Fedora/RHEL/CentOS
   sudo dnf install arm-none-eabi-gcc arm-none-eabi-newlib
   
   # Arch Linux
   sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib
   ```

3. **Build Dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install -y \
       cmake ninja-build python3-pip \
       python3-dev libxml2-utils \
       git make build-essential
   
   # Python packages
   pip3 install --user pyserial empy toml numpy pandas \
       jinja2 pyyaml pyros-genmsg packaging
   ```

4. **RP2040 Specific Tools**
   ```bash
   # Install picotool for flashing
   git clone https://github.com/raspberrypi/picotool.git
   cd picotool
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   
   # Install uf2 utilities
   pip3 install --user uf2conv
   ```

### Build Instructions (Linux)

1. **First-time setup**
   ```bash
   cd OpenFC2040_FYP
   ./scripts/setup_workspace.sh
   cd firmware/openfc2040
   ```

2. **Build the firmware**
   ```bash
   # Complete build (setup + build + generate UF2)
   ./scripts/build.sh all
   
   # Or just build (if already set up)
   ./scripts/build.sh build
   
   # Build with specific thread count
   ./scripts/build.sh build -j 4
   
   # Clean build
   ./scripts/build.sh clean
   ```

3. **Output files**
   ```bash
   # Firmware is built in:
   px4-autopilot/build/rsp_2040_default/
   
   # Key files:
   # - rsp_2040_default.bin  : Raw binary firmware
   # - rsp_2040_default.uf2  : UF2 format for drag-and-drop
   # - rsp_2040_default.elf  : Debug symbols included
   ```

## Flashing the Firmware (Linux)

### Method 1: UF2 Bootloader (Recommended - Automated)

```bash
# Flash using the script
./scripts/flash.sh

# The script will:
# 1. Check for RP2040 in bootloader mode
# 2. Copy UF2 file automatically
# 3. Verify the flash was successful
```

### Method 2: Manual UF2 Flash

1. **Enter bootloader mode**
   - Hold BOOTSEL button while connecting USB
   - Device appears as USB drive "RPI-RP2"

2. **Flash the firmware**
   ```bash
   # Copy UF2 file to the RP2040
   cp px4-autopilot/build/rsp_2040_default/rsp_2040_default.uf2 /media/$USER/RPI-RP2/
   
   # Board will automatically reboot
   ```

3. **Verify firmware**
   ```bash
   # Connect to console
   screen /dev/ttyACM0 115200
   ```

### Method 2: Picotool (Command Line)

1. **Install picotool** (if not already installed)
   ```bash
   # Dependencies
   sudo apt install libusb-1.0-0-dev
   
   # Build picotool
   git clone https://github.com/raspberrypi/picotool.git
   cd picotool
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```

2. **Flash using picotool**
   ```bash
   # Put device in BOOTSEL mode (hold button while connecting)
   
   # Load firmware
   sudo picotool load build/rsp_2040_default/rsp_2040_default.uf2
   
   # Or load and reboot
   sudo picotool load -r build/rsp_2040_default/rsp_2040_default.uf2
   
   # Get device info
   sudo picotool info
   ```

### Method 3: OpenOCD with SWD

1. **Setup OpenOCD**
   ```bash
   # Install OpenOCD with RP2040 support
   sudo apt install openocd
   
   # Or build from source for latest RP2040 support
   git clone https://github.com/raspberrypi/openocd.git
   cd openocd
   ./bootstrap
   ./configure --enable-picoprobe
   make
   sudo make install
   ```

2. **Connect SWD programmer** (e.g., Raspberry Pi Pico as picoprobe)
   ```
   Picoprobe     ->  OpenFC2040
   SWDIO (GP2)   ->  SWDIO
   SWCLK (GP3)   ->  SWCLK
   GND           ->  GND
   ```

3. **Flash using OpenOCD**
   ```bash
   # Flash the firmware
   openocd -f interface/picoprobe.cfg -f target/rp2040.cfg \
           -c "adapter speed 5000" \
           -c "program build/rsp_2040_default/rsp_2040_default.elf verify reset exit"
   
   # Or for debugging
   openocd -f interface/picoprobe.cfg -f target/rp2040.cfg
   # Then in another terminal:
   gdb-multiarch build/rsp_2040_default/rsp_2040_default.elf
   (gdb) target remote localhost:3333
   (gdb) load
   (gdb) monitor reset init
   (gdb) continue
   ```


### Troubleshooting Flash Issues

1. **Device not detected**
   ```bash
   # Check USB devices
   lsusb
   dmesg | tail -20  # Check kernel messages
   
   # Add udev rules for RP2040
   echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", MODE="0666"' | \
       sudo tee /etc/udev/rules.d/99-rp2040.rules
   sudo udevadm control --reload-rules
   ```

2. **Permission issues**
   ```bash
   # Add user to dialout group
   sudo usermod -a -G dialout $USER
   # Logout and login again
   ```

3. **Mount point issues**
   ```bash
   # Manually mount if needed
   sudo mkdir -p /mnt/rp2040
   sudo mount /dev/sdX1 /mnt/rp2040  # Replace sdX1 with actual device
   sudo cp rsp_2040_default.uf2 /mnt/rp2040/
   sudo umount /mnt/rp2040
   ```

## Configuration and Calibration

### Initial Setup

1. **Connect to QGroundControl**
   - Connect via USB or telemetry port
   - Baud rate: 57600 (telemetry) or 115200 (USB)

2. **Sensor Calibration**
   ```
   - Accelerometer calibration
   - Gyroscope calibration
   - Compass calibration (if external magnetometer connected)
   - Level horizon calibration
   ```

3. **Radio Setup**
   - Configure RC input on GPIO24
   - Calibrate radio channels
   - Set flight modes

4. **ESC Calibration**
   - Connect ESCs to GPIO20-23
   - Run ESC calibration procedure
   - Set PWM min/max values

### Parameter Configuration

Key parameters for OpenFC2040:

```
# Battery monitoring
BAT1_V_DIV = 11.0      # Voltage divider ratio
BAT1_A_PER_V = 40.0    # Current sensor scaling

# PWM Configuration
PWM_RATE = 400         # PWM frequency in Hz
PWM_MIN = 1000         # Minimum PWM value (us)
PWM_MAX = 2000         # Maximum PWM value (us)

# Sensor orientation
SENS_BOARD_ROT = 0     # Board rotation (if needed)

# System identification
SYS_AUTOSTART = 4001   # Generic Quadcopter
```

## Testing and Validation

### Hardware Tests

1. **LED Test**
   ```bash
   # In NuttX shell
   led_control test
   ```

2. **Sensor Test**
   ```bash
   # Check IMU
   lsm6ds3 status
   
   # Check barometer
   dps310 status
   
   # List all sensors
   sensors status
   ```

3. **PWM Output Test**
   ```bash
   # Test individual channels
      pwm test -c 1 -p 1500  # Channel 1, 1500 us
      pwm test -c 2 -p 1500  # Channel 2, 1500 us
   ```

### SITL (Software In The Loop)

1. **Build SITL version**
   ```bash
   make px4_sitl_default gazebo
   ```

2. **Run simulation**
   ```bash
   ./build/px4_sitl_default/bin/px4 -s etc/init.d-posix/rcS
   ```

3. **Connect QGroundControl** to UDP port 14550

### Hardware-in-the-Loop (HIL)

1. **Connect OpenFC2040 to computer**
2. **Configure HIL mode in QGroundControl**
3. **Run simulation with hardware sensors**

## Troubleshooting

### Common Issues

1. **Board not detected via USB**
   - Check USB cable (must be data cable, not charge-only)
   - Verify VBUS detection on GPIO29
   - Check USB descriptors in `usb.c`

2. **Sensors not detected**
   - Verify SPI connections (GPIO8-12)
   - Check chip select pins (GPIO9 for IMU, GPIO12 for baro)
   - Run `i2cdetect` for I2C devices

3. **No PWM output**
   - Verify timer configuration in `timer_config.cpp`
   - Check GPIO20-23 connections
   - Ensure ESCs are powered

4. **GPS not working**
   - Check UART1 configuration (GPIO4/5)
   - Verify baud rate (typically 9600 or 115200)
   - Ensure GPS has clear sky view

### Debug Commands

```bash
# System information
ver all

# Check work queues
work_queue status

# Performance counters
perf

# Top (CPU usage)
top

# Memory usage
free

# Parameter list
param show

# Reboot
reboot
```

## Development Notes

### Repository Structure

```
rsp_2040/                           # Main repository (self-sufficient)
├── .gitmodules                     # Git submodule configuration
├── .gitignore                      # Git ignore rules
├── README.md                       # This file
├── CONTRIBUTING.md                 # Collaboration guidelines
│
├── px4-autopilot/                  # PX4-Autopilot (Git submodule)
│   └── [Full PX4 repository]      # Auto-managed, don't edit directly
│
├── board/                          # Board configuration (YOUR CODE)
│   └── rsp_2040/                  # OpenFC2040 board files
│       ├── src/                   # Board-specific source
│       │   ├── board_config.h     # Hardware configuration
│       │   ├── init.c            # Board initialization
│       │   ├── spi.cpp           # SPI bus configuration
│       │   ├── i2c.cpp           # I2C bus configuration
│       │   ├── timer_config.cpp  # PWM timer configuration
│       │   ├── led.c             # RGB LED driver
│       │   ├── usb.c             # USB configuration
│       │   └── CMakeLists.txt    # Build configuration
│       ├── init/                  # Startup scripts
│       │   ├── rc.board_defaults
│       │   ├── rc.board_sensors
│       │   └── rc.board_mavlink
│       ├── nuttx-config/          # NuttX OS configuration
│       │   ├── include/board.h
│       │   ├── nsh/defconfig
│       │   └── Kconfig
│       ├── default.px4board       # PX4 build configuration
│       └── firmware.prototype     # Firmware metadata
│
├── drivers/                        # Custom drivers (YOUR CODE)
│   └── imu/st/lsm6ds3/           # LSM6DS3 IMU driver
│       ├── LSM6DS3.hpp
│       ├── LSM6DS3.cpp
│       └── CMakeLists.txt
│
├── scripts/                        # Build & utility scripts
│   ├── setup.sh                   # First-time setup
│   ├── build.sh                   # Build firmware
│   ├── flash.sh                   # Flash to board
│   └── clean.sh                   # Clean build artifacts
│
├── docs/                          # Documentation
│   ├── METHOD.md                  # Development methodology
│   ├── test_checklist.md          # Testing procedures
│   └── hardware/
│       └── pinout.txt             # Hardware pinout
│
├── configs/                       # Configuration files
│   └── params_openfc2040.txt     # Default parameters
│
└── tests/                         # Test scripts and data
```

### What to Edit

**Recommended files to edit (your code):**
- `board/rsp_2040/` - All board-specific files
- `drivers/` - Custom drivers
- `scripts/` - Build scripts
- `docs/` - Documentation
- `configs/` - Configuration files

**Avoid editing these (auto-managed):**
- `px4-autopilot/` - Managed as Git submodule
- `build/` - Auto-generated build artifacts

### Adding Features

1. **New Sensor Support**
   - Add driver in `drivers/` directory
   - Update `default.px4board` configuration
   - Add initialization in `rc.board_sensors`

2. **Additional PWM Channels**
   - Modify `timer_config.cpp`
   - Update `board_config.h` definitions
   - Adjust `DIRECT_PWM_OUTPUT_CHANNELS`

3. **Custom Parameters**
   - Add to `rc.board_defaults`
   - Document in this README

## Safety Considerations

**Warning**: this is flight control software. Improper configuration or bugs can cause crashes.

- Always test with propellers removed
- Use a current-limited power supply during development
- Implement proper failsafe configurations
- Test thoroughly before flight
- Keep safety switch accessible
- Have a plan for emergency landing

## Support and Contributing

### Reporting Issues

When reporting issues, please include:
- Firmware version (`ver all` output)
- Hardware connections diagram
- Console output/logs
- Steps to reproduce

### Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

Quick workflow:
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Make changes in `board/` or `drivers/`
4. Test thoroughly: `./scripts/build.sh && ./scripts/flash.sh`
5. Commit: `git commit -m "Add feature"`
6. Push: `git push origin feature/your-feature`
7. Create Pull Request

## License

This firmware port is released under the BSD 3-Clause License, consistent with PX4 Autopilot.

## Acknowledgments

- PX4 Development Team for the autopilot framework
- Raspberry Pi Foundation for the RP2040 platform
- OpenFC2040 hardware designers
- Community contributors

## Contact

For questions and support:
- PX4 Forum: https://discuss.px4.io/
- PX4 Discord: https://discord.gg/dronecode
- GitHub Issues: [Create issue in repository]

---

**Version**: 1.0.0  
**Last Updated**: 2024  
**Status**: Production-ready with testing recommended
