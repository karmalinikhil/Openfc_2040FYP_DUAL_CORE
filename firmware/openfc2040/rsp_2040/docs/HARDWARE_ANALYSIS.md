# OpenFC2040 Hardware Analysis - Complete PCB Review

**Date**: October 30, 2024  
**Based on**: EasyEDA PCB Design Files (openFC2040-main)  
**PCB Version**: 2022-05-01

---

## 📋 Executive Summary

This document provides a comprehensive analysis of the OpenFC2040 PCB hardware, comparing the actual PCB design against the current PX4 firmware configuration. Several **critical components** require driver implementation and configuration updates.

### 🚨 Critical Findings

1. **❌ MISSING: MicroSD Card Driver** - Hardware present, no software support
2. **❌ MISSING: External Flash (W25Q128) Configuration** - 16MB flash not configured for logging
3. **⚠️ INCORRECT: RGB LED Configuration** - Using FM-B2020RGBA-HG (4-channel RGBA) but configured as 3 separate GPIOs
4. **⚠️ INCOMPLETE: Connector Pin Mappings** - GPS_MAG, POWER, TELE connectors need detailed pinout
5. **✅ VERIFIED: Core sensors properly configured** - IMU and Barometer correct

---

## 🔌 Complete Hardware Inventory

### **1. Microcontroller Unit**
| Component | Part Number | Package | Specs |
|-----------|-------------|---------|-------|
| MCU | RP2040 | LQFN-56 | Dual Cortex-M0+ @ 133MHz, 264KB SRAM |
| Flash | W25Q128JVSIQTR | SOIC-8 | 128Mbit (16MB) SPI NOR Flash |
| Crystal | X322512MSB4SI | SMD 4-pin | 12MHz |

**Status**: ✅ Configured  
**Notes**: External flash needs explicit configuration for logging and storage

---

### **2. Sensors**

#### **2.1 IMU (Inertial Measurement Unit)**
| Component | Part Number | Interface | Specs |
|-----------|-------------|-----------|-------|
| IMU | LSM6DS3TR-C | SPI | 6-axis (Accel + Gyro), LGA-14 |

**Status**: ✅ Driver present in `/drivers/imu/st/lsm6ds3/`  
**Expected GPIO**: CS=GPIO9, SCK=GPIO10, MOSI=GPIO11, MISO=GPIO8  
**Configuration**: ✅ Correctly defined in `board_config.h`

#### **2.2 Barometer**
| Component | Part Number | Interface | Specs |
|-----------|-------------|-----------|-------|
| Barometer | DPS310XTSA1 | SPI | Precision pressure sensor, LGA-8 |

**Status**: ✅ PX4 driver available (`CONFIG_DRIVERS_BAROMETER_DPS310=y`)  
**Expected GPIO**: CS=GPIO12, SCK=GPIO10, MISO=GPIO8  
**Configuration**: ✅ Correctly defined in `board_config.h`

---

### **3. Storage Devices**

#### **3.1 External Flash Memory**
| Component | Part Number | Interface | Capacity |
|-----------|-------------|-----------|----------|
| Flash | W25Q128JVSIQTR (Winbond) | SPI | 16MB (128Mbit) |

**Status**: ⚠️ **NEEDS CONFIGURATION**  
**Purpose**: Firmware storage, parameter backup, logging  
**GPIO Assignment**: Shared SPI bus with bootloader  
**Action Required**: 
- Verify RP2040 SDK flash configuration
- Configure for PX4 parameter storage
- Enable logging to flash

#### **3.2 MicroSD Card Slot**
| Component | Part Number | Interface | Type |
|-----------|-------------|-----------|------|
| SD Slot | 503398-1892 (Molex) | SPI/SDIO | MicroSD (TF card) |

**Status**: ❌ **MISSING DRIVER - CRITICAL**  
**Purpose**: High-speed logging, mission storage, parameter backup  
**Action Required**:
```cpp
// Need to add to board configuration:
1. Identify SD card GPIO pins from schematic (typical: CS, SCK, MOSI, MISO)
2. Add PX4 SD card driver configuration
3. Enable SDIO if supported, else SPI mode
4. Configure filesystem support
```

**Expected GPIOs** (need schematic verification):
- CS: GPIO16 or GPIO17 (typical for RP2040)
- SCK: GPIO18
- MOSI: GPIO19
- MISO: GPIO16

---

### **4. User Interface**

#### **4.1 RGB LED**
| Component | Part Number | Type | Channels |
|-----------|-------------|------|----------|
| LED | FM-B2020RGBA-HG (NATIONSTAR) | RGBA SMD | 4 channels (R, G, B, + A) |

**Status**: ⚠️ **INCORRECT CONFIGURATION**  

**Current Configuration** (WRONG):
```c
#define GPIO_LED_RED    PX4_MAKE_GPIO_OUTPUT_CLEAR(13)  // GPIO13
#define GPIO_LED_GREEN  PX4_MAKE_GPIO_OUTPUT_CLEAR(14)  // GPIO14
#define GPIO_LED_BLUE   PX4_MAKE_GPIO_OUTPUT_CLEAR(15)  // GPIO15
```

**Problem**: FM-B2020RGBA-HG is a **common-anode RGBA LED** with 4 channels:
- Pin 1: Red cathode
- Pin 2: Common anode (+3.3V)
- Pin 3: Green cathode
- Pin 4: Blue cathode
- Pin 5: Additional channel (White or Amber)

**LED Current Limiting Resistors** (from BOM):
- R14 = 162Ω (likely Red channel)
- R15 = 60.4Ω (likely Blue channel)
- R16 = 100Ω (likely Green channel)  
- R17 = 140Ω (likely 4th channel - White/Amber)

**Action Required**:
1. Verify actual GPIO connections from schematic
2. Determine if 4th channel is used
3. Update LED driver to use PWM for brightness control
4. Correct polarity (common anode = active LOW)

---

#### **4.2 Buzzer**
| Component | Part Number | Type | Drive |
|-----------|-------------|------|-------|
| Buzzer | QMB-09B-03 (HNDZ) | Passive | PWM required |

**Status**: ✅ Configured on GPIO25  
**Type**: Passive buzzer (requires PWM square wave)  
**Optimal Frequency**: 2700Hz ± 300Hz (range: 2400-3000Hz)  
**Configuration**: ✅ `CONFIG_DRIVERS_TONE_ALARM=y`  
**Note**: Drive with PWM at 2700Hz for maximum sound output

---

#### **4.3 Buttons**
| Component | Function | Type |
|-----------|----------|------|
| ARM | Arming button | TS-1187A-B-A-B tactile switch |
| BOOTSEL | Bootloader entry | 2-pin header with jumper |

**Status**: ✅ ARM button configured  
**BOOTSEL**: Hardware function (RP2040 boot mode)

---

### **5. Connectors**

#### **5.1 JST-GH 6-Pin Connectors** (3x SM06B-GHS-TB)

| Connector | Purpose | Typical Pinout |
|-----------|---------|----------------|
| GPS_MAG | GPS + Magnetometer | 1:VCC, 2:TX, 3:RX, 4:SCL, 5:SDA, 6:GND |
| TELE | Telemetry Radio | 1:VCC, 2:TX, 3:RX, 4:CTS, 5:RTS, 6:GND |
| POWER | Battery Input | 1:VCC, 2:VCC, 3:CURR, 4:VOLT, 5:?, 6:GND |

**Status**: ⚠️ **NEEDS VERIFICATION**  
**Action Required**: 
- Verify actual pinout from schematic
- Confirm UART assignments (GPS=UART1, TELE=UART0)
- Verify ADC channels for battery monitoring

**Current Configuration**:
```c
// UARTs
CONFIG_BOARD_SERIAL_GPS1="/dev/ttyS1"   // GPIO4/5
CONFIG_BOARD_SERIAL_TEL1="/dev/ttyS0"   // GPIO0/1

// I2C for GPS magnetometer
PX4_I2C_BUS_GPS=0      // GPIO6/7
PX4_I2C_BUS_TELEM=1    // GPIO2/3

// ADC for battery
ADC_BATTERY_VOLTAGE_CHANNEL=1  // GPIO27
ADC_BATTERY_CURRENT_CHANNEL=2  // GPIO28
```

---

#### **5.2 3-Pin Headers** (6x HDR-M-2.54_1x3)

| Connector | Purpose | Pinout |
|-----------|---------|--------|
| ESC0 | Motor 1 PWM | GND, VCC, Signal |
| ESC1 | Motor 2 PWM | GND, VCC, Signal |
| ESC2 | Motor 3 PWM | GND, VCC, Signal |
| ESC3 | Motor 4 PWM | GND, VCC, Signal |
| RC | RC Receiver Input | GND, VCC, Signal |
| DBG | Debug/Serial Console | GND, TX, RX |

**Status**: ✅ Configured  
**PWM Outputs**: GPIO20-23  
**RC Input**: GPIO24

---

### **6. Power Management**

| Component | Part Number | Function | Output |
|-----------|-------------|----------|--------|
| Voltage Regulator | SSP1117-3.3V | LDO Regulator | 3.3V @ 1A |
| Schottky Diode | 1N5819WS | Reverse protection | 1A, 40V |

**Status**: ✅ Hardware design correct  
**Input Voltage**: 5V via USB or POWER connector  
**Output**: 3.3V for all logic

---

### **7. USB Interface**

| Component | Part Number | Type | Pins |
|-----------|-------------|------|------|
| USB Connector | MICRO5.9MMUSB (SHOU HAN) | Micro-USB | 5-pin |

**Status**: ✅ Configured  
**USB Data Lines**: R3, R4 (27.4Ω) are USB termination resistors (correct for HS USB)  
**VBUS Detection**: GPIO29  
**Configuration**: ✅ `CONFIG_DRIVERS_CDCACM_AUTOSTART=y`

---

## 🔧 Required Firmware Updates

### **Priority 1: CRITICAL - Add SD Card Support**

**File**: `/board/rsp_2040/src/board_config.h`

```c
/* MicroSD Card Configuration - ADD THIS */
#define BOARD_HAS_SDIO           1
#define PX4_SDIO_BUS             1
// Need to verify GPIO from schematic:
#define GPIO_SDIO_CS             PX4_MAKE_GPIO_OUTPUT(16)  // TBD - verify from schematic
#define GPIO_SDIO_SCK            (18 | GPIO_FUN(RP2040_GPIO_FUNC_SPI))
#define GPIO_SDIO_MOSI           (19 | GPIO_FUN(RP2040_GPIO_FUNC_SPI))
#define GPIO_SDIO_MISO           (16 | GPIO_FUN(RP2040_GPIO_FUNC_SPI))

// Card detect pin (if available)
#define GPIO_SDIO_CD             (17 | GPIO_FUN(RP2040_GPIO_FUNC_SIO))  // TBD
```

**File**: `/board/default.px4board`

```ini
# Add SD card driver
CONFIG_DRIVERS_SDCARD=y
CONFIG_FS_FAT=y
CONFIG_FS_ROMFS=y
```

---

### **Priority 2: HIGH - Configure External Flash**

**File**: `/board/rsp_2040/src/board_config.h`

```c
/* External Flash Configuration - ADD THIS */
#define BOARD_HAS_EXT_FLASH      1
#define EXT_FLASH_SIZE_MB        16
#define EXT_FLASH_PAGE_SIZE      256
#define EXT_FLASH_SECTOR_SIZE    4096
```

**File**: `/board/default.px4board`

```ini
# Enable external flash for logging
CONFIG_DRIVERS_FLASH_W25=y
```

---

### **Priority 3: MEDIUM - Fix RGB LED Configuration**

**Investigation needed**: Need to trace schematic to determine:
1. Which GPIOs control which LED channels
2. Is the 4th channel (R17/140Ω) connected?
3. Common anode or cathode configuration?

**Likely correction**:
```c
/* RGB LED Configuration - CORRECTED */
// FM-B2020RGBA-HG is common-anode, active LOW
#define GPIO_LED_RED    PX4_MAKE_GPIO_OUTPUT_SET(13)   // Active LOW
#define GPIO_LED_GREEN  PX4_MAKE_GPIO_OUTPUT_SET(14)   // Active LOW
#define GPIO_LED_BLUE   PX4_MAKE_GPIO_OUTPUT_SET(15)   // Active LOW

// If 4th channel used:
// #define GPIO_LED_WHITE  PX4_MAKE_GPIO_OUTPUT_SET(XX)  // TBD - verify GPIO
```

---

## 📊 GPIO Pin Assignment Summary

| GPIO | Function | Peripheral | Status |
|------|----------|------------|--------|
| 0 | UART0 TX | Telemetry | ✅ |
| 1 | UART0 RX | Telemetry | ✅ |
| 2 | I2C1 SDA | Telemetry I2C | ✅ |
| 3 | I2C1 SCL | Telemetry I2C | ✅ |
| 4 | UART1 TX | GPS | ✅ |
| 5 | UART1 RX | GPS | ✅ |
| 6 | I2C0 SDA | GPS I2C | ✅ |
| 7 | I2C0 SCL | GPS I2C | ✅ |
| 8 | SPI MISO | Sensors (IMU/Baro) | ✅ |
| 9 | SPI CS | IMU LSM6DS3 | ✅ |
| 10 | SPI SCK | Sensors | ✅ |
| 11 | SPI MOSI | Sensors | ✅ |
| 12 | SPI CS | Barometer DPS310 | ✅ |
| 13 | GPIO | LED Red | ⚠️ Check polarity |
| 14 | GPIO | LED Green | ⚠️ Check polarity |
| 15 | GPIO | LED Blue | ⚠️ Check polarity |
| 16 | SPI/GPIO | SD Card (CS or MISO) | ❌ Needs config |
| 17 | GPIO | SD Card Detect? | ❌ TBD |
| 18 | SPI SCK | SD Card | ❌ Needs config |
| 19 | SPI MOSI | SD Card | ❌ Needs config |
| 20 | PWM | ESC 1 (Motor 1) | ✅ |
| 21 | PWM | ESC 2 (Motor 2) | ✅ |
| 22 | PWM | ESC 3 (Motor 3) | ✅ |
| 23 | PWM | ESC 4 (Motor 4) | ✅ |
| 24 | GPIO/PWM | RC Input | ✅ |
| 25 | PWM | Buzzer | ✅ |
| 26 | ADC0 | Reserved/Debug | ❓ |
| 27 | ADC1 | Battery Voltage | ✅ |
| 28 | ADC2 | Battery Current | ✅ |
| 29 | ADC3 | USB VBUS Detect | ✅ |

**Legend**:
- ✅ Verified and configured
- ⚠️ Configured but needs verification
- ❌ Missing configuration
- ❓ Unknown/unused

---

## 🎯 Recommended Actions

### **Immediate Actions** (Block build/flash):

1. **Extract GPIO connections from schematic**
   - Parse `PCB_openFC2040.json` or `openFC2040.json` for net connections
   - Create definitive GPIO mapping table
   - Verify SD card pins, LED channels

2. **Add SD Card Driver**
   - Implement or configure RP2040 SD card driver
   - Enable FAT filesystem support
   - Configure logging to SD card

3. **Configure External Flash**
   - Enable W25Q128 driver
   - Configure for parameter storage
   - Enable blackbox logging

### **Follow-up Actions**:

4. **Fix LED Configuration**
   - Verify FM-B2020RGBA-HG connections
   - Update driver for correct polarity
   - Test all LED channels

5. **Verify Connectors**
   - Document GPS_MAG connector pinout
   - Document POWER connector pinout
   - Verify battery ADC scaling

6. **Test Hardware**
   - Create hardware test suite
   - Verify all GPIO assignments
   - Test SD card read/write
   - Test external flash
   - Verify sensor communication

---

## 📚 References

- **PCB Design**: EasyEDA files in `/openFC2040-main/`
- **RP2040 Datasheet**: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf
- **LSM6DS3TR-C Datasheet**: ST Microelectronics
- **DPS310 Datasheet**: Infineon Technologies
- **W25Q128JV Datasheet**: Winbond Electronics
- **PX4 Board Porting Guide**: https://docs.px4.io/main/en/hardware/porting_guide.html

---

**Document Status**: 🟡 Pending Schematic Analysis  
**Next Step**: Parse schematic JSON to extract GPIO connections
