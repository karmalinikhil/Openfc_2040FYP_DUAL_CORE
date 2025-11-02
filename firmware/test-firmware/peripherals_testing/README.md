# OpenFC2040 Peripheral Testing Firmware

Simple firmware to test all peripherals on the OpenFC2040 flight controller board: IMU, Barometer, GPS, LED, and Buzzer.

## Hardware Overview

The OpenFC2040 features:
- **MCU**: Raspberry Pi RP2040 (dual-core ARM Cortex-M0+)
- **IMU**: LSM6DS3TR-C (6-axis gyro/accelerometer)
- **Barometer**: DPS310XTSA1 (pressure sensor)
- **RGB LED**: Status indication
- **Buzzer**: Audio feedback
- **ESC Outputs**: 4x PWM outputs for motor control
- **RC Input**: RC receiver connection
- **External Flash**: W25Q128 (128Mbit)

## Pin Mapping

| Component | Pin | GPIO | Notes |
|-----------|-----|------|-------|
| **LEDs** | | | |
| Red LED | | GPIO13 | |
| Green LED | | GPIO14 | |
| Blue LED | | GPIO15 | |
| **SPI Sensors** | | | |
| SCK (Clock) | | GPIO10 | Shared by IMU & Barometer |
| MOSI (SDA) | | GPIO11 | IMU data line |
| MISO (SDO) | | GPIO8 | Shared by IMU & Barometer |
| IMU CS | | GPIO9 | LSM6DS3TR-C chip select |
| Baro CS | | GPIO12 | DPS310 chip select |
| **ESC Outputs** | | | |
| ESC1 | | GPIO20 | PWM output |
| ESC2 | | GPIO21 | PWM output |
| ESC3 | | GPIO22 | PWM output |
| ESC4 | | GPIO23 | PWM output |
| **Other** | | | |
| RC Input | | GPIO24 | RC receiver |
| Buzzer | | GPIO25 | Audio output |

## Features

- **Software SPI**: Bit-banging SPI for sensor communication
- **Interactive Commands**: Serial console with debugging commands
- **LED Control**: RGB LED with manual and automatic modes
- **Sensor Reading**: Real-time IMU and barometer data
- **PWM ESC Control**: 400Hz PWM for motor control
- **Buzzer Control**: Audio feedback and startup sequence

## Quick Start

### Prerequisites

- CMake (3.13 or later)
- GCC ARM toolchain
- Git

### Build and Flash

1. **Clone and setup**:
   ```bash
   git clone <this-repo>
   cd OPENFC2040
   git submodule update --init --recursive
   ```

2. **Build firmware**:
   ```bash
   ./scripts/build.sh
   ```

3. **Flash to board**:
   ```bash
   # Put board in bootloader mode (hold BOOTSEL + connect USB)
   ./scripts/flash.sh
   ```

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Manual Flash

```bash
# Copy UF2 file to RP2040 in bootloader mode
cp build/openfc2040_firmware.uf2 /media/$USER/RPI-RP2/
```

## Serial Console Commands

Connect to the board's USB serial port (115200 baud) and use these commands:

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `imu` | Read IMU data (accelerometer + gyroscope) |
| `baro` | Read barometer data (pressure + temperature) |
| `gps` | Read GPS data from UART |
| `sensors` | Read sensor IDs (IMU: 0x6A, Baro: 0x10) |
| `red` | Turn on red LED |
| `green` | Turn on green LED |
| `blue` | Turn on blue LED |
| `off` | Turn off all LEDs |
| `beep` | Test buzzer |
| `live` | Start live sensor data stream (press any key to stop) |

## Development

### Directory Structure

```
OPENFC2040/
├── src/                    # Source code
│   └── main.c             # Main firmware file
├── include/               # Header files
│   └── openfc2040_board.h # Hardware pin definitions
├── scripts/               # Build and flash scripts
│   ├── build.sh          # Build script
│   └── flash.sh          # Flash script
├── build/                 # Build output (generated)
├── pico-sdk/             # Pico SDK (submodule)
├── CMakeLists.txt        # CMake configuration
└── README.md             # This file
```

### Adding Features

1. Edit `src/main.c` for firmware logic
2. Update `include/openfc2040_board.h` for pin definitions
3. Rebuild with `./scripts/build.sh`
4. Flash with `./scripts/flash.sh`

### Debugging

- Use `sensors` command to verify sensor communication
- Use `barotest` for detailed barometer debugging
- Use `spitest` to verify SPI communication
- Monitor LED patterns for system status

## Troubleshooting

### Build Issues

- Ensure Pico SDK submodule is initialized: `git submodule update --init --recursive`
- Check CMake version: `cmake --version` (need 3.13+)
- Verify ARM toolchain: `arm-none-eabi-gcc --version`

### Sensor Issues

- IMU not detected: Check SPI wiring and CS pin (GPIO9)
- Barometer not detected: Check CS pin (GPIO12) and run `barotest`
- No sensor response: Verify SCK (GPIO10) and MISO (GPIO8) connections

### Flash Issues

- Board not detected: Hold BOOTSEL while connecting USB
- Permission denied: Check mount point permissions
- Copy fails: Ensure RPI-RP2 drive is mounted

## License

This firmware is provided as-is for educational and development purposes.

## Contributing

Feel free to submit issues and pull requests to improve the firmware.
