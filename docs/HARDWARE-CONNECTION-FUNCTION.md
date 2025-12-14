# OpenFC2040 Pin Connections

**Complete GPIO Mapping and Hardware Connections**

> **Hardware Design by**: [@vxj9800](https://github.com/vxj9800/openFC2040)  

---

## Table of Contents

- [Quick Reference](#quick-reference)
- [Detailed Pin Mapping and Functionality](#detailed-pin-mapping-and-functionality)
- [Power Rails](#power-rails)
- [Component Connections](#component-connections)
- [Connector Pinouts](#connector-pinouts)
- [Notes and Warnings](#notes-and-warnings)

---

## Quick Reference

### GPIO Pin Summary Table

| GPIO | Function | Peripheral | Component | Pin Type | Notes |
|------|----------|------------|-----------|----------|-------|
| GPIO0 | UART0_TX | UART0 | Telemetry | Output | Console TX |
| GPIO1 | UART0_RX | UART0 | Telemetry | Input | Console RX |
| GPIO2 | I2C1_SDA | I2C1 | Telemetry | Bidir | CTS (optional) |
| GPIO3 | I2C1_SCL | I2C1 | Telemetry | Bidir | RTS (optional) |
| GPIO4 | UART1_TX | UART1 | GPS | Output | GPS TX |
| GPIO5 | UART1_RX | UART1 | GPS | Input | GPS RX |
| GPIO6 | I2C1_SDA | I2C1 | GPS/Mag | Bidir | Magnetometer SDA |
| GPIO7 | I2C1_SCL | I2C1 | GPS/Mag | Bidir | Magnetometer SCL |
| GPIO8 | SPI1_RX | SPI1 | IMU/Baro | Input | MISO (SDO) |
| GPIO9 | SPI1_CSn | SPI1 | IMU | Output | IMU Chip Select |
| GPIO10 | SPI1_SCK | SPI1 | IMU/Baro | Output | Shared SPI Clock |
| GPIO11 | SPI1_TX | SPI1 | IMU/Baro | Output | MOSI (SDA) |
| GPIO12 | GPIO | - | Barometer | Output | Baro Chip Select |
| GPIO13 | PWM6_B | PWM | LED Red | Output | Active LOW |
| GPIO14 | PWM7_A | PWM | LED Green | Output | Active LOW |
| GPIO15 | PWM7_B | PWM | LED Blue | Output | Active LOW |
| GPIO16 | SPI0_RX | SPI0 | SD Card | Input | SD MISO |
| GPIO17 | SPI0_CSn | SPI0 | SD Card | Output | SD Chip Select |
| GPIO18 | SPI0_SCK | SPI0 | SD Card | Output | SD Clock |
| GPIO19 | SPI0_TX | SPI0 | SD Card | Output | SD MOSI |
| GPIO20 | PWM2_A | PWM | ESC0 | Output | Motor 1 |
| GPIO21 | PWM2_B | PWM | ESC1 | Output | Motor 2 |
| GPIO22 | PWM3_A | PWM | ESC2 | Output | Motor 3 |
| GPIO23 | PWM3_B | PWM | ESC3 | Output | Motor 4 |
| GPIO24 | GPIO | - | RC Input | Input | PPM/SBUS |
| GPIO25 | PWM4_B | PWM | Buzzer | Output | Active HIGH |
| GPIO26 | ADC0 | ADC | ARM Button | Input | Arming button |
| GPIO27 | ADC1 | ADC | Battery Voltage | Input | Voltage divider |
| GPIO28 | ADC2 | ADC | Battery Current | Input | Current sensor |
| GPIO29 | ADC3 | ADC | USB Detect | Input | VBUS detection |

### QSPI Flash (W25Q128JV)

| QSPI Pin | Function | Component | Notes |
|----------|----------|-----------|-------|
| QSPI_CS | Chip Select | Flash | 16MB SPI Flash |
| QSPI_SD0 | Data 0 (DI) | Flash | MOSI |
| QSPI_SD1 | Data 1 (DO) | Flash | MISO |
| QSPI_SD2 | Data 2 (IO2) | Flash | Quad SPI |
| QSPI_SD3 | Data 3 (IO3) | Flash | Quad SPI |
| QSPI_SCLK | Clock | Flash | SPI Clock |

### USB

| USB Pin | Function | Notes |
|---------|----------|-------|
| USB_DM | USB D- | Data Negative |
| USB_DP | USB D+ | Data Positive |
| VBUS | USB Power | 5V from USB |

### Other

| Pin | Function | Notes |
|-----|----------|-------|
| RUN | Reset | Active LOW reset |
| SWCLK | SWD Clock | Debugging |
| SWDIO | SWD Data | Debugging |
| XIN | Crystal In | 12 MHz crystal |
| XOUT | Crystal Out | 12 MHz crystal |

---

## Detailed Pin Mapping and Functionality

### Communication Interfaces

#### UART0 - Telemetry/Console

**Purpose**: MAVLink telemetry OR NuttX Shell console

| Pin | GPIO | Direction | Signal | Voltage | Notes |
|-----|------|-----------|--------|---------|-------|
| 1 | GND | - | Ground | 0V | Common ground |
| 2 | GPIO0 | Output | TX | 3.3V | Transmit data |
| 3 | GPIO1 | Input | RX | 3.3V | Receive data |
| 4 | GPIO2 | Bidir | SDA/CTS | 3.3V | Optional flow control |
| 5 | GPIO3 | Bidir | SCL/RTS | 3.3V | Optional flow control |
| 6 | +5V | - | Power | 5V | From VBUS/Battery |

**Connector**: 6-pin JST-GH (SM06B-GHS-TB)

**Current Use**: UART console (115200 baud, 8N1)

**Wiring for USB-Serial Adapter**:
```
Adapter TX  →  GPIO1 (RX)
Adapter RX  →  GPIO0 (TX)
Adapter GND →  GND
```

#### UART1 - GPS_MAG

**Purpose**: External GPS, Magnetometer module communication

| Pin | GPIO | Direction | Signal | Voltage | Notes |
|-----|------|-----------|--------|---------|-------|
| 1 | GND | - | Ground | 0V | Common ground |
| 2 | GPIO4 | Output | TX | 3.3V | GPS RX |
| 3 | GPIO5 | Input | RX | 3.3V | GPS TX |
| 4 | GPIO6 | Bidir | SDA | 3.3V | Magnetometer I2C |
| 5 | GPIO7 | Bidir | SCL | 3.3V | Magnetometer I2C |
| 6 | +5V | - | Power | 5V | GPS module power |

**Connector**: 6-pin JST-GH (SM06B-GHS-TB)

**Compatible GPS**: Ublox NEO-M8N, M9N with HMC5883L/QMC5883L magnetometer

---

### Sensors

#### LSM6DS3TR-C IMU (6-Axis Gyro + Accel)

**Interface**: SPI1

| Signal | GPIO | RP2040 Pin | Function |
|--------|------|------------|----------|
| CS | GPIO9 | Pin 12 | Chip Select (active LOW) |
| SCK | GPIO10 | Pin 14 | SPI Clock |
| MOSI | GPIO11 | Pin 15 | Master Out Slave In (SDA) |
| MISO | GPIO8 | Pin 11 | Master In Slave Out (SDO) |
| INT1 | - | - | Not connected |
| INT2 | - | - | Not connected |

**Package**: LGA-14 (3.0mm x 2.5mm)

**Specifications**:
- Gyro range: ±125, ±250, ±500, ±1000, ±2000 dps
- Accel range: ±2, ±4, ±8, ±16 g
- Max ODR: 6.66 kHz
- Power: 0.9 mA (combo mode)

**SPI Settings**:
- Mode: Mode 0 or Mode 3
- Max frequency: 10 MHz
- CS: Active LOW

#### DPS310XTSA1 Barometer

**Interface**: SPI1 (BUS shared with IMU)

| Signal | GPIO | RP2040 Pin | Function |
|--------|------|------------|----------|
| CS | GPIO12 | Pin 16 | Chip Select (active LOW) |
| SCK | GPIO10 | Pin 14 | SPI Clock (shared) |
| MOSI | GPIO11 | Pin 15 | Master Out (shared) |
| MISO | GPIO8 | Pin 11 | Master In (shared) |

**Package**: LGA-8

**Specifications**:
- Pressure range: 300-1200 hPa
- Accuracy: ±0.002 hPa (±0.02m)
- Temperature: -40°C to +85°C
- Power: 1.7 µA (standby)

**SPI Settings**:
- Mode: Mode 0 or Mode 3
- Max frequency: 10 MHz
- CS: Active LOW

---

### Outputs

#### PWM Outputs (ESC0-ESC3)

| ESC | GPIO | PWM Slice | PWM Channel | Connector Pin | Motor |
|-----|------|-----------|-------------|---------------|-------|
| ESC0 | GPIO20 | PWM2 | A | 1 | Motor 1 (Front Right) |
| ESC1 | GPIO21 | PWM2 | B | 2 | Motor 2 (Rear Right) |
| ESC2 | GPIO22 | PWM3 | A | 3 | Motor 3 (Rear Left) |
| ESC3 | GPIO23 | PWM3 | B | 4 | Motor 4 (Front Left) |

**Connector per ESC**: 3-pin JST-GH
- Pin 1: GND
- Pin 2: +5V
- Pin 3: Signal (GPIO)

**PWM Signal**:
- Frequency: 50 Hz (20ms period) for standard PWM
- Or 400 Hz (2.5ms period) for Oneshot125
- Pulse width: 1000-2000 µs (1ms-2ms)
- Logic level: 3.3V

**⚠️ WARNING**: NEVER connect propellers during testing! Use current-limited power supply.

#### RGB LED (FM-B2020RGBA-HG)

| Color | GPIO | PWM Slice | PWM Channel | Active |
|-------|------|-----------|-------------|--------|
| Red | GPIO13 | PWM6 | B | LOW |
| Green | GPIO14 | PWM7 | A | LOW |
| Blue | GPIO15 | PWM7 | B | LOW |

**Package**: SMD LED (2.0mm x 2.0mm)

**Drive**: Active LOW (0V = ON, 3.3V = OFF)

**Resistors**:
- Red: 160Ω (GPIO13 → LED)
- Green: 60.4Ω (GPIO14 → LED)
- Blue: 100Ω (GPIO15 → LED)

**Control**:
```c
// Turn on RED LED
gpio_write(GPIO13, 0);  // Active LOW!

// Turn off RED LED
gpio_write(GPIO13, 1);
```

#### Buzzer (QMB-09B-03)

| Signal | GPIO | PWM Slice | PWM Channel | Active |
|--------|------|-----------|-------------|--------|
| Buzzer | GPIO25 | PWM4 | B | HIGH |

**Type**: Passive buzzer (requires PWM tone)

**Drive**: Active HIGH

**Frequency Range**: 2700±300 HZ

**Typical tones**:
- Arming: 1 beep (short)
- Armed: Continuous tone
- Low battery: 3 rapid beeps

---

### Inputs

#### RC Input

| Signal | GPIO | Function | Input Type |
|--------|------|----------|------------|
| RC_IN | GPIO24 | Digital input | PPM/SBUS |

**Connector**: 3-pin JST-GH
- Pin 1: GND
- Pin 2: +5V (to power receiver)
- Pin 3: GPIO24 (signal)

**Supported Protocols**:
- PPM (Pulse Position Modulation)
- SBUS (Futaba, inverted serial)
- DSM/DSM2/DSMX (Spektrum)

**Signal Level**: 3.3V tolerant

**⚠️ Note**: SBUS is inverted serial (normally HIGH). May need level shifter or inverter circuit.

#### Arming Button

| Signal | GPIO | ADC Channel | Function |
|--------|------|-------------|----------|
| ARM_PB | GPIO26 | ADC0 | Push button |

**Circuit**: Button to ground, internal pull-up

**Reading**:
- Released: 3.3V (ADC: 4095)
- Pressed: 0V (ADC: 0)

---

### Storage

#### MicroSD Card Slot (503398-1892)

**Interface**: SPI0

| Signal | GPIO | RP2040 Pin | SD Pin | Function |
|--------|------|------------|--------|----------|
| CS | GPIO17 | Pin 22 | DAT3/CS | Chip Select |
| MOSI | GPIO19 | Pin 25 | CMD/DI | Data In |
| MISO | GPIO16 | Pin 21 | DAT0/DO | Data Out |
| SCK | GPIO18 | Pin 24 | CLK | Clock |
| CD | - | - | CD | Card Detect (not used) |

**SPI Settings**:
- Mode: Mode 0
- Init frequency: 400 kHz
- Max frequency: 25 MHz
- Voltage: 3.3V

**Filesystem**: FAT32 (for compatibility)

**⚠️ Important**: SD card must be formatted as FAT32, not exFAT.

#### SPI Flash (W25Q128JVSIQ)

**Interface**: QSPI

| Signal | QSPI Pin | Flash Pin | Function |
|--------|----------|-----------|----------|
| CS | QSPI_CS | /CS | Chip Select |
| DI | QSPI_SD0 | DI | Data In (MOSI) |
| DO | QSPI_SD1 | DO | Data Out (MISO) |
| IO2 | QSPI_SD2 | /WP | Write Protect |
| IO3 | QSPI_SD3 | /HOLD | Hold |
| CLK | QSPI_SCLK | CLK | Clock |

**Capacity**: 128 Mbit (16 MB)

**Speed**:
- Standard SPI: up to 104 MHz
- Dual SPI: up to 104 MHz
- Quad SPI: up to 104 MHz

**Usage**: Firmware storage, parameter storage, logs

---

### Analog Inputs (ADC)

#### Battery Monitoring

| Signal | GPIO | ADC Channel | Function | Voltage Range |
|--------|------|-------------|----------|---------------|
| BATT_V | GPIO27 | ADC1 | Battery voltage | 0-3.3V (after divider) |
| BATT_A | GPIO28 | ADC2 | Battery current | 0-3.3V (from sensor) |

**ADC Specifications**:
- Resolution: 12-bit (0-4095)
- Reference: 3.3V
- Input range: 0-3.3V

**Voltage Divider (BATT_V)**:

⚠️ **CRITICAL**: Voltage divider values need verification from schematic!

From BOM:
- R3, R4: 27.4Ω resistors

Current firmware config assumes:
```c
#define BOARD_BATTERY_V_DIV   (1.74f)  // May be incorrect!
```

For 4S LiPo (16.8V max), we need higher divider ratio!

**Action Required**: Review schematic to determine actual R_top and R_bottom values.

Calculation:
```
V_div = R_bottom / (R_top + R_bottom)
ADC_voltage = Battery_voltage × V_div
```

For 16.8V battery:
```
ADC_voltage must be < 3.3V
Min divider ratio: 16.8V / 3.3V = 5.09
```

**Current Sensor (BATT_A)**:

Typically uses ACS712 or similar Hall-effect sensor.

Output: 2.5V @ 0A, ±0.066V/A or ±0.185V/A (depends on sensor)

**⚠️ TODO**: Verify current sensor IC from schematic.

---

## Power Rails

### Voltage Rails

| Rail | Voltage | Source | Current | Consumers |
|------|---------|--------|---------|-----------|
| VBUS | 5.0V | USB or Battery | 500mA+ | Voltage regulator, peripherals |
| +3.3V | 3.3V | SSP1117-3.3V LDO | 1A max | RP2040, sensors, logic |
| +1.1V | 1.1V | Internal VREG | - | RP2040 core |
| IOVDD | 3.3V | +3.3V rail | - | RP2040 I/O |
| DVDD | 1.1V | Internal VREG | - | RP2040 digital core |
| ADC_AVDD | 3.3V | +3.3V rail | - | ADC reference |
| USB_VDD | 3.3V | +3.3V rail | - | USB PHY |

### Power Tree

```
Battery (7.4V-16.8V)
    │
    ├─→ 5V Regulator
    │       │
    │       ├─→ ESC connectors (+5V pin)
    │       ├─→ GPS connector (+5V pin)
    │       └─→ RC Input connector (+5V pin)
    │
    └─→ USB VBUS (5V)
            │
            └─→ SSP1117-3.3V LDO
                    │
                    ├─→ RP2040 IOVDD (all 10 pins)
                    ├─→ RP2040 ADC_AVDD
                    ├─→ RP2040 USB_VDD
                    ├─→ LSM6DS3 (IMU)
                    ├─→ DPS310 (Barometer)
                    ├─→ W25Q128 (Flash)
                    └─→ Peripheral pull-ups

RP2040 Internal:
+3.3V → Internal 1.1V VREG → DVDD (core voltage)
```

### Decoupling Capacitors

From BOM:

| Value | Quantity | Locations |
|-------|----------|-----------|
| 100nF | 15 | C1-C9, C12, C17, C19-C22 (bypass caps) |
| 1µF | 2 | C10, C11 (bulk caps) |
| 10µF | 3 | C13, C14, C18 (power rail bulk) |

**Placement**: Decoupling caps placed close to each IC power pin.

---

## Component Connections

### All Onboard Components

| Component | Part Number | Interface | GPIOs Used | Function |
|-----------|-------------|-----------|------------|----------|
| MCU | RP2040 | - | All 30 GPIOs | Main processor |
| IMU | LSM6DS3TR-C | SPI1 | 8, 9, 10, 11 | Gyro + Accel |
| Barometer | DPS310XTSA1 | SPI1 | 8, 10, 11, 12 | Pressure sensor |
| Flash | W25Q128JVSIQ | QSPI | QSPI pins | Firmware storage |
| LED | FM-B2020RGBA-HG | PWM | 13, 14, 15 | RGB status LED |
| Buzzer | QMB-09B-03 | PWM | 25 | Audio feedback |
| USB | MICRO5.9MMUSB | USB | USB_DP, USB_DM | Communication |
| SD Slot | 503398-1892 | SPI0 | 16, 17, 18, 19 | Data logging |
| Crystal | 12MHz | OSC | XIN, XOUT | Clock source |
| LDO | SSP1117-3.3V | - | - | 3.3V regulator |

---

## Connector Pinouts

### Telemetry (6-pin JST-GH)

| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | GND | Ground |
| 2 | TX (GPIO0) | Transmit data |
| 3 | RX (GPIO1) | Receive data |
| 4 | SDA (GPIO2) | I2C Data / CTS |
| 5 | SCL (GPIO3) | I2C Clock / RTS |
| 6 | +5V | Power output |

### GPS/Mag (6-pin JST-GH)

| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | GND | Ground |
| 2 | TX (GPIO4) | GPS transmit |
| 3 | RX (GPIO5) | GPS receive |
| 4 | SDA (GPIO6) | Magnetometer I2C data |
| 5 | SCL (GPIO7) | Magnetometer I2C clock |
| 6 | +5V | GPS module power |

### ESC0-ESC3 (3-pin JST-GH each)

| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | GND | Ground |
| 2 | +5V | ESC power (BEC) |
| 3 | Signal | PWM signal (GPIO20-23) |

### RC Input (3-pin JST-GH)

| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | GND | Ground |
| 2 | +5V | Receiver power |
| 3 | Signal (GPIO24) | RC input signal |

### Debug (3-pin header)

| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | GND | Ground |
| 2 | SWCLK | SWD clock |
| 3 | SWDIO | SWD data |

---

## Notes and Warnings

### ⚠️ Critical Warnings

1. **Battery Voltage Divider**: Current firmware config may be WRONG! Verify from schematic before connecting 4S LiPo.

2. **LED Polarity**: RGB LED is **ACTIVE LOW**. Writing 0 turns LED ON!

3. **SBUS Inversion**: SBUS protocol is inverted. May need hardware inverter.

4. **ESC Testing**: NEVER connect propellers during testing! Use current-limited power supply.

5. **SD Card Format**: Must be FAT32, not exFAT.

### 📝 Important Notes

1. **I2C Pull-ups**: 47kΩ resistor arrays (RN1, RN2) provide pull-ups for I2C lines.

2. **SPI Sharing**: IMU and Barometer share SPI1 bus. Only CS lines differ.

3. **PWM Frequency**: Configurable in firmware. Standard PWM (50 Hz) or Oneshot125 (400 Hz).

4. **ADC Reference**: RP2040 ADC uses internal 3.3V reference (no external VREF).

5. **USB Detection**: GPIO29 (ADC3) monitors VBUS for USB connection status.

### 🔧 Configuration in Firmware

Pin assignments defined in:
```
firmware/openfc2040/rsp_2040/board/board_config.h
```

Example:
```c
// LEDs
#define GPIO_nLED_RED    13  // Active LOW
#define GPIO_nLED_GREEN  14  // Active LOW
#define GPIO_nLED_BLUE   15  // Active LOW

// IMU
#define GPIO_SPI1_CS_IMU    9
#define GPIO_SPI1_SCK       10
#define GPIO_SPI1_MOSI      11
#define GPIO_SPI1_MISO      8

// Battery monitoring
#define ADC_BATTERY_VOLTAGE_CHANNEL  1  // GPIO27
#define ADC_BATTERY_CURRENT_CHANNEL  2  // GPIO28
```

---

## Schematic Reference

**Official Hardware Repository**: https://github.com/vxj9800/openFC2040

Files:
- `openFC2040.json` - EasyEDA schematic
- `BOM_PCB_openFC2040_2022-05-01.csv` - Bill of Materials
- `Gerber_PCB_openFC2040.zip` - PCB fabrication files

**Local copy**: `hardware/openfc2040-official/`

---

**Verified By**: Schematic analysis (openFC2040.json)

For questions about pin configuration, see `firmware/openfc2040/rsp_2040/board/board_config.h`.
