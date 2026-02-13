## Overview

This document details the issues encountered when integrating the DPS310 barometer sensor with the RP2040-based OpenFC2040 flight controller running PX4/NuttX, and the changes required to make it work.

---

## Problem Description

The DPS310 barometer was not being detected on the SPI bus despite correct wiring and pin configuration. The sensor probe would fail silently with no data being published to the `sensor_baro` uORB topic in the NuttX shell(nsh).

  
### Symptoms

- `dps310 -s start` command in NuttX shell would give the output message "no instance started (no device on bus?)"

- `listener sensor_baro` showed no data

---

## Root Cause Analysis

The issue was in the **chip select (CS) logic** within the RP2040 SPI driver implementation for NuttX. The problem manifested in how the SPI chip select was being managed based on device type identification.

  

### The Chip Select Problem

In the PX4/NuttX SPI architecture for RP2040, the chip select logic uses a `devtype` field to identify which device is being addressed. The original implementation had incorrect logic for selecting the barometer's chip select line.

  

#### Original Faulty Code

The chip select control was looking for specific device type values but the comparison logic was incorrect:

In `rp2040_spi.c` - the chipselect handling, the original source code was checking device types incorrectly


  

### Key Discovery

The device types used in PX4 for SPI device identification:

- **DPS310 Barometer**: Device type `0x44` (68 decimal) or similar baro identifier


The chip select logic needed to properly distinguish between these devices and assert the correct CS line for each.

---  

## Solution

  

### Changes Made

  

#### 1. SPI Chip Select Logic Fix

  

**File**: `boards/raspberrypi/pico/src/init.c` 

The chip select function was modified to correctly identify the barometer device and assert the appropriate GPIO according to the chip ID:


```c

// Corrected chip select logic

void rp2040_spi_select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)

{

uint32_t devtype = SPIDEVID_TYPE(devid);

// Barometer CS on GPIO (check your pin configuration)

if (devtype == 0x44) { // DPS310 barometer device type

px4_arch_gpiowrite(GPIO_SPI_CS_BARO, !selected);

}


}

```


#### 2. GPIO Pin Configuration

**File**: `boards/raspberrypi/pico/src/board_config.h`

Ensure the barometer chip select GPIO is properly defined:

  

```c

// SPI Chip Select pins

#define GPIO_SPI_CS_BARO PX4_MAKE_GPIO_OUTPUT_SET(X) // Replace X with actual GPIO

#define GPIO_SPI_CS_IMU PX4_MAKE_GPIO_OUTPUT_SET(Y) // Replace Y with actual GPIO

```

  

#### 3. SPI Bus Configuration

  

**File**: `boards/raspberrypi/pico/src/spi.cpp`

  

The SPI bus configuration needed proper device registration:

  

```cpp

// DPS310 on SPI bus

constexpr px4_spi_bus_device_t spi_bus_devices[] = {

// ... other devices ...

{ PX4_SPIDEV_TYPE_BARO, 0x44, 0 }, // DPS310 barometer

};

```

  #### 4. Initializing internal SPI instead of external on start up

**File**: `/boards/raspberrypi/pico/init/rc.board_sensors`

```bash
#!/bin/sh
#
# board specific sensors init
#------------------------------------------------------------------------------
# 1. Start ADC
# Necessary for Battery Voltage (GPIO27) and Current (GPIO28) monitoring
board_adc start

# 2. Start IMU: Using LSM9DS1 driver as placeholder
# The LSM6DS3TR-C is similar but has no native PX4 driver
# -s : Internal SPI bus
# -R : Rotation (0 = ROTATION_NONE)
# NOTE: This may not work correctly - LSM9DS1 and LSM6DS3 have different register maps!
# A proper LSM6DS3 driver needs to be ported for production use
lsm9ds1 start -s -R 0 # Was -S, which made it search for external SPI buses

# 3. Start Barometer: DPS310
# -s : Internal SPI bus
dps310 start -s # Was -S, which made it search for external SPI buses
```

---

## Verification

After applying the fix, verification was done using:


```bash

# Start the DPS310 driver
nsh> dps310 -s start # Might give "device not found on bus" or similar type of error because the device(dps310) automatically starts when the board is powered ON 


nsh> dps310 status # Outputs given shows that it can detect baro on SPI bus 


# Check if data is being published
nsh> listener sensor_baro

```

---

## Key Takeaways


1. **Device Type Matters**: PX4's SPI subsystem relies on device type identifiers (devtype) to route chip select signals. These must match between the driver and board configuration.

  

2. **Active Low CS**: SPI chip selects are typically active-low. The `!selected` logic ensures the CS line goes LOW when the device is selected.

  

3. **Check Both Ends**: When SPI doesn't work, verify:

- Physical connections (MOSI, MISO, SCK, CS)

- GPIO configuration in board_config.h

- Chip select logic in the SPI driver

- Device type matching in driver vs board config

  

4. **Debug Approach**: Use the following to debug SPI issues:

- Check if CS line toggles (oscilloscope or logic analyzer)

- Verify SPI clock frequency is within sensor specs

- Check for pull-up/pull-down resistor requirements

---

## Related Files


- `boards/raspberrypi/pico/src/board_config.h` - GPIO definitions

- `boards/raspberrypi/pico/src/spi.cpp` - SPI bus device configuration

- `boards/raspberrypi/pico/src/rp2040_spi.c` - Low-level SPI with CS control

- `src/drivers/barometer/dps310/` - DPS310 driver source

- `/boards/raspberrypi/pico/init/rc.board_sensors` - Changing internal and external SPI buses on initialization

  


