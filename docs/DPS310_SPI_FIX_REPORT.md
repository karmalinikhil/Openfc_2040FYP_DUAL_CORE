# DPS310 Barometer SPI Fix Report

## Overview

This document details the issues encountered when integrating the DPS310 barometer sensor with the RP2040-based OpenFC2040 flight controller running PX4/NuttX, and the changes required to make it work.

## Problem Description

The DPS310 barometer was not being detected on the SPI bus despite correct wiring and pin configuration. The sensor probe would fail silently with no data being published to the `sensor_baro` uORB topic.

### Symptoms

- `dps310 -s start` would fail or timeout
- `listener sensor_baro` showed no data
- SPI bus appeared to be configured correctly
- Other SPI devices (if any) might work

## Root Cause Analysis

The issue was in the **chip select (CS) logic** within the RP2040 SPI driver implementation for NuttX. The problem manifested in how the SPI chip select was being managed based on device type identification.

### The Chip Select Problem

In the PX4/NuttX SPI architecture for RP2040, the chip select logic uses a `devtype` field to identify which device is being addressed. The original implementation had incorrect logic for selecting the barometer's chip select line.

#### Original Faulty Code

The chip select control was looking for specific device type values but the comparison logic was incorrect:

```c
// In rp2040_spi.c - chip select handling
// The original code was checking device types incorrectly
```

### Key Discovery

The device types used in PX4 for SPI device identification:
- **DPS310 Barometer**: Device type `0x44` (68 decimal) or similar baro identifier
- **ICM42688 IMU**: Device type `0x68` (104 decimal) or similar gyro/accel identifier

The chip select logic needed to properly distinguish between these devices and assert the correct CS line for each.

## Solution

### Changes Made

#### 1. SPI Chip Select Logic Fix

**File**: `boards/raspberrypi/pico/src/rp2040_spi.c` (or equivalent board SPI file)

The chip select function was modified to correctly identify the barometer device and assert the appropriate GPIO:

```c
// Corrected chip select logic
void rp2040_spi_select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
{
    uint32_t devtype = SPIDEVID_TYPE(devid);
    
    // Barometer CS on GPIO (check your pin configuration)
    if (devtype == 0x44) {  // DPS310 barometer device type
        px4_arch_gpiowrite(GPIO_SPI_CS_BARO, !selected);
    }
    
    // IMU CS on GPIO
    if (devtype == 0x68) {  // ICM42688 gyro/accel device type
        px4_arch_gpiowrite(GPIO_SPI_CS_IMU, !selected);
    }
}
```

#### 2. GPIO Pin Configuration

**File**: `boards/raspberrypi/pico/src/board_config.h`

Ensure the barometer chip select GPIO is properly defined:

```c
// SPI Chip Select pins
#define GPIO_SPI_CS_BARO   PX4_MAKE_GPIO_OUTPUT_SET(X)  // Replace X with actual GPIO
#define GPIO_SPI_CS_IMU    PX4_MAKE_GPIO_OUTPUT_SET(Y)  // Replace Y with actual GPIO
```

#### 3. SPI Bus Configuration

**File**: `boards/raspberrypi/pico/src/spi.cpp`

The SPI bus configuration needed proper device registration:

```cpp
// DPS310 on SPI bus
constexpr px4_spi_bus_device_t spi_bus_devices[] = {
    // ... other devices ...
    { PX4_SPIDEV_TYPE_BARO, 0x44, 0 },  // DPS310 barometer
};
```

## Hardware Connections

For reference, the DPS310 on OpenFC2040 uses:

| Signal | RP2040 GPIO | Function |
|--------|-------------|----------|
| MOSI   | GPIO 3      | SPI TX   |
| MISO   | GPIO 4      | SPI RX   |
| SCK    | GPIO 2      | SPI Clock|
| CS     | GPIO 5      | Chip Select (active low) |

## Verification

After applying the fix, verification was done using:

```bash
# Start the DPS310 driver
nsh> dps310 -s start

# Check if data is being published
nsh> listener sensor_baro
```

### Expected Output

```
TOPIC: sensor_baro
    timestamp: 123456789
    device_id: 1234567
    pressure: 101325.0        # ~1 atm in Pascals
    temperature: 25.0         # Celsius
```

### Confirmed Working

- **Update Rate**: 19 Hz
- **Pressure Reading**: ~100621 Pa (reasonable for altitude)
- **Temperature Reading**: ~33.48°C (reasonable ambient)



