# Build Commands Reference

Quick reference for building OpenFC2040 firmware on Linux.

## Prerequisites Installed

Assuming you have already installed:
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- CMake, Make, Python3
- Git with submodule support

## Quick Build (Most Common)

```bash
# Navigate to project
cd /path/to/rsp_2040

# Build everything
./scripts/build.sh
```

## Step-by-Step Commands

### 1. First Time Setup

```bash
# Clone repository with submodules
git clone --recursive https://github.com/yourteam/rsp_2040.git
cd rsp_2040

# Initialize PX4 submodule and copy board files
./scripts/setup.sh
```

### 2. Build Firmware

```bash
# Complete build (setup + compile + generate UF2)
./scripts/build.sh all

# Or just compile (if already set up)
./scripts/build.sh build

# Build with specific thread count
./scripts/build.sh build -j 8

# Clean build
./scripts/build.sh clean
```

### 3. Generate UF2 File

UF2 is automatically generated during build, but if you need to regenerate:

```bash
cd px4-autopilot

python3 Tools/uf2conv.py \
    -b 0x10000000 \
    -f 0xe48bff56 \
    build/rsp_2040_default/rsp_2040_default.bin \
    -o build/rsp_2040_default/rsp_2040_default.uf2
```

**Parameters explained:**
- `-b 0x10000000` - Base address for RP2040 flash memory
- `-f 0xe48bff56` - Family ID for RP2040 chips
- Input: `.bin` file
- Output: `.uf2` file

### 4. Flash to Board

```bash
# Automated flashing
./scripts/flash.sh

# Or manual copy (if board is in bootloader mode)
cp px4-autopilot/build/rsp_2040_default/rsp_2040_default.uf2 /media/$USER/RPI-RP2/
```

## Manual Build Commands (Without Scripts)

If you prefer to run commands manually:

```bash
# 1. Copy board files to PX4
mkdir -p px4-autopilot/boards/rsp_2040
cp -r board/rsp_2040/* px4-autopilot/boards/rsp_2040/

# 2. Copy custom drivers
mkdir -p px4-autopilot/src/drivers/imu/st/lsm6ds3
cp -r drivers/imu/st/lsm6ds3/* px4-autopilot/src/drivers/imu/st/lsm6ds3/

# 3. Build
cd px4-autopilot
make rsp_2040_default -j$(nproc)

# 4. Generate UF2
python3 Tools/uf2conv.py \
    -b 0x10000000 \
    -f 0xe48bff56 \
    build/rsp_2040_default/rsp_2040_default.bin \
    -o build/rsp_2040_default/rsp_2040_default.uf2
```

## Output Files Location

```bash
px4-autopilot/build/rsp_2040_default/
├── rsp_2040_default.bin    # Raw binary (for uf2conv)
├── rsp_2040_default.uf2    # UF2 file (for flashing)
├── rsp_2040_default.elf    # ELF with debug symbols
└── rsp_2040_default.hex    # Intel HEX format
```

## Cleaning Build Artifacts

```bash
# Clean everything
./scripts/clean.sh

# Deep clean (includes PX4 distclean)
./scripts/clean.sh --deep

# Manual clean
cd px4-autopilot
make distclean
```

## Common Build Options

```bash
# Build with verbose output
./scripts/build.sh build VERBOSE=1

# Build specific target
cd px4-autopilot
make rsp_2040_default -j$(nproc)

# List all available targets
make list_config_targets
```

## Troubleshooting

### Build fails with "arm-none-eabi-gcc not found"
```bash
# Install ARM toolchain
sudo apt-get install gcc-arm-none-eabi
```

### Submodule not initialized
```bash
git submodule update --init --recursive
```

### Permission denied on scripts
```bash
chmod +x scripts/*.sh
```

### Out of memory during build
```bash
# Reduce parallel jobs
./scripts/build.sh build -j 2
```

## Development Workflow

```bash
# 1. Make changes to your code
vim board/rsp_2040/src/init.c

# 2. Rebuild
./scripts/build.sh build

# 3. Flash
./scripts/flash.sh

# 4. Monitor serial output
screen /dev/ttyACM0 115200
```

## CI/CD Integration

For automated builds:

```bash
#!/bin/bash
set -e

# Setup
git clone --recursive https://github.com/yourteam/rsp_2040.git
cd rsp_2040
./scripts/setup.sh

# Build
./scripts/build.sh all

# Artifacts are in:
# px4-autopilot/build/rsp_2040_default/rsp_2040_default.uf2
```
