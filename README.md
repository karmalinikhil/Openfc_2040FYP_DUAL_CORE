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

## Building the Firmware

### Prerequisites (Linux)

1. **PX4 Development Environment**
   ```bash
   git clone https://github.com/PX4/PX4-Autopilot.git
   cd PX4-Autopilot
   git submodule update --init --recursive
   ```

2. **ARM Toolchain Installation**
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

1. **Setup the build environment**
   ```bash
   # Set PX4 path (adjust to your location)
   export PX4_ROOT=/path/to/PX4-Autopilot
   
   # Copy the rsp_2040 directory to PX4-Autopilot
   cp -r rsp_2040 $PX4_ROOT/boards/
   
   # Copy the LSM6DS3 driver to PX4 drivers
   mkdir -p $PX4_ROOT/src/drivers/imu/st/lsm6ds3
   cp -r rsp_2040/drivers/imu/st/lsm6ds3/* $PX4_ROOT/src/drivers/imu/st/lsm6ds3/
   ```

2. **Build the firmware**
   ```bash
   cd $PX4_ROOT
   
   # Clean build (optional, for first build or after major changes)
   make distclean
   
   # Build the firmware (uses all CPU cores by default)
   make rsp_2040_default
   
   # Or specify number of parallel jobs
   make rsp_2040_default -j4
   ```

3. **Generate UF2 file for easy flashing**
   ```bash
   # Convert binary to UF2 format
   uf2conv build/rsp_2040_default/rsp_2040_default.bin \
           --base 0x10000000 \
           --family RP2040 \
           --output build/rsp_2040_default/rsp_2040_default.uf2
   ```

4. **Output files**
   ```bash
   # Check generated files
   ls -lh build/rsp_2040_default/
   
   # Key files:
   # - rsp_2040_default.bin  : Raw binary firmware
   # - rsp_2040_default.uf2  : UF2 format for drag-and-drop
   # - rsp_2040_default.elf  : Debug symbols included
   ```

5. **Using the build script (automated)**
   ```bash
   # Make the script executable
   chmod +x rsp_2040/build.sh
   
   # Run complete build
   ./rsp_2040/build.sh all
   
   # Or individual steps
   ./rsp_2040/build.sh setup  # Copy files to PX4
   ./rsp_2040/build.sh build  # Build firmware
   ./rsp_2040/build.sh flash  # Flash to board
   ```

## Flashing the Firmware (Linux)

### Method 1: UF2 Bootloader (Recommended)

1. **Enter bootloader mode**
   ```bash
   # Hold BOOTSEL button while connecting USB
   # The device will appear as a mass storage device
   
   # Check if device is detected
   lsusb | grep "2e8a:0003"  # Should show Raspberry Pi RP2 Boot
   
   # Find the mount point
   df -h | grep RPI-RP2
   # Usually mounted at /media/$USER/RPI-RP2
   ```

2. **Flash the firmware**
   ```bash
   # Copy UF2 file to the RP2040
   cp build/rsp_2040_default/rsp_2040_default.uf2 /media/$USER/RPI-RP2/
   
   # The board will automatically reboot with new firmware
   # Check serial connection
   ls /dev/ttyACM*  # Should show /dev/ttyACM0 or similar
   ```

3. **Verify firmware**
   ```bash
   # Connect to console
   screen /dev/ttyACM0 115200
   # Or use minicom
   minicom -D /dev/ttyACM0 -b 115200
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

### Method 4: Automated Script

```bash
# Use the provided build script
./rsp_2040/build.sh flash

# The script will:
# 1. Check for RP2040 in bootloader mode
# 2. Copy UF2 file automatically
# 3. Verify the flash was successful
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
PWM_MIN = 1000         # Minimum PWM value (μs)
PWM_MAX = 2000         # Maximum PWM value (μs)

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
   pwm test -c 1 -p 1500  # Channel 1, 1500μs
   pwm test -c 2 -p 1500  # Channel 2, 1500μs
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

### Code Structure

```
rsp_2040/
├── src/                  # Board-specific source files
│   ├── board_config.h    # Hardware configuration
│   ├── init.c           # Board initialization
│   ├── spi.cpp          # SPI bus configuration
│   ├── i2c.cpp          # I2C bus configuration
│   ├── timer_config.cpp # PWM timer configuration
│   ├── led.c            # RGB LED driver
│   ├── usb.c            # USB configuration
│   └── CMakeLists.txt   # Build configuration
├── init/                # Startup scripts
│   ├── rc.board_defaults
│   ├── rc.board_sensors
│   └── rc.board_mavlink
├── nuttx-config/        # NuttX OS configuration
│   ├── include/
│   │   └── board.h
│   ├── nsh/
│   │   └── defconfig
│   └── Kconfig
├── drivers/             # Custom drivers
│   └── imu/st/lsm6ds3/ # LSM6DS3 IMU driver
├── default.px4board     # PX4 build configuration
├── firmware.prototype   # Firmware metadata
└── README.md           # This file
```

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

⚠️ **WARNING**: This is flight control software. Improper configuration or bugs can cause crashes.

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

1. Fork the repository
2. Create a feature branch
3. Test thoroughly
4. Submit pull request with:
   - Clear description
   - Test results
   - Documentation updates

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
