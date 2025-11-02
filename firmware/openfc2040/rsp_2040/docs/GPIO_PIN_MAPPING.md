# OpenFC2040 Complete GPIO Pin Mapping

**Board**: OpenFC2040 Flight Controller  
**MCU**: Raspberry Pi RP2040  
**Based on**: PCB Design v2022-05-01 + User Confirmation

---

## 📌 Verified GPIO Pin Assignments

| GPIO | Function | Peripheral | Component | Interface | Status |
|------|----------|------------|-----------|-----------|--------|
| **GPIO0** | UART0 TX | Telemetry UART | TELE connector | UART | ✅ Verified |
| **GPIO1** | UART0 RX | Telemetry UART | TELE connector | UART | ✅ Verified |
| **GPIO2** | I2C1 SDA | Telemetry I2C | TELE connector | I2C | ✅ Verified |
| **GPIO3** | I2C1 SCL | Telemetry I2C | TELE connector | I2C | ✅ Verified |
| **GPIO4** | UART1 TX | GPS UART | GPS_MAG connector | UART | ✅ Verified |
| **GPIO5** | UART1 RX | GPS UART | GPS_MAG connector | UART | ✅ Verified |
| **GPIO6** | I2C0 SDA | GPS I2C | GPS_MAG connector | I2C | ✅ Verified |
| **GPIO7** | I2C0 SCL | GPS I2C | GPS_MAG connector | I2C | ✅ Verified |
| **GPIO8** | SPI0 MISO (RX) | Sensor SPI | IMU + Barometer | SPI | ✅ Verified |
| **GPIO9** | SPI0 CS | IMU Chip Select | LSM6DS3TR-C | SPI | ✅ Verified |
| **GPIO10** | SPI0 SCK | Sensor SPI Clock | IMU + Barometer | SPI | ✅ Verified |
| **GPIO11** | SPI0 MOSI (TX) | Sensor SPI | IMU + Barometer | SPI | ✅ Verified |
| **GPIO12** | SPI0 CS | Baro Chip Select | DPS310XTSA1 | SPI | ✅ Verified |
| **GPIO13** | GPIO/PWM | RGB LED Red | FM-B2020RGBA-HG | PWM | ✅ Verified |
| **GPIO14** | GPIO/PWM | RGB LED Green | FM-B2020RGBA-HG | PWM | ✅ Verified |
| **GPIO15** | GPIO/PWM | RGB LED Blue | FM-B2020RGBA-HG | PWM | ✅ Verified |
| **GPIO16** | SPI1 MISO | SD Card Data In | MicroSD slot | SPI/SDIO | ⚠️ Needs Config |
| **GPIO17** | GPIO | SD Card Detect | MicroSD slot | GPIO | ⚠️ Needs Config |
| **GPIO18** | SPI1 SCK | SD Card Clock | MicroSD slot | SPI/SDIO | ⚠️ Needs Config |
| **GPIO19** | SPI1 MOSI | SD Card Data Out | MicroSD slot | SPI/SDIO | ⚠️ Needs Config |
| **GPIO20** | PWM | ESC 1 / Motor 1 | ESC0 connector | PWM | ✅ Verified |
| **GPIO21** | PWM | ESC 2 / Motor 2 | ESC1 connector | PWM | ✅ Verified |
| **GPIO22** | PWM | ESC 3 / Motor 3 | ESC2 connector | PWM | ✅ Verified |
| **GPIO23** | PWM | ESC 4 / Motor 4 | ESC3 connector | PWM | ✅ Verified |
| **GPIO24** | GPIO/PWM | RC Input | RC connector | PPM/PWM | ✅ Verified |
| **GPIO25** | PWM | Buzzer | Passive buzzer | PWM | ✅ Verified |
| **GPIO26** | ADC0 | Reserved/Debug | - | ADC | ⚠️ Unused |
| **GPIO27** | ADC1 | Battery Voltage | POWER connector | ADC | ✅ Verified |
| **GPIO28** | ADC2 | Battery Current | POWER connector | ADC | ✅ Verified |
| **GPIO29** | ADC3 | USB VBUS Detect | USB connector | ADC | ✅ Verified |

### Special RP2040 Pins (QSPI - Flash Interface)
| Signal | Function | Component |
|--------|----------|-----------|
| QSPI_SS | Flash Chip Select | W25Q128JVSIQTR (16MB) |
| QSPI_SCLK | Flash Clock | W25Q128JVSIQTR |
| QSPI_SD0 | Flash Data 0 | W25Q128JVSIQTR |
| QSPI_SD1 | Flash Data 1 | W25Q128JVSIQTR |
| QSPI_SD2 | Flash Data 2 | W25Q128JVSIQTR |
| QSPI_SD3 | Flash Data 3 | W25Q128JVSIQTR |

**Note**: QSPI pins are dedicated to external flash and not available as GPIOs.

---

## 🔌 Connector Pinouts

### GPS_MAG Connector (6-pin JST-GH)
| Pin | Signal | Function | RP2040 GPIO |
|-----|--------|----------|-------------|
| 1 | VCC | 5V Power | - |
| 2 | TX | GPS UART TX | GPIO4 |
| 3 | RX | GPS UART RX | GPIO5 |
| 4 | SCL | I2C Clock (Mag) | GPIO7 |
| 5 | SDA | I2C Data (Mag) | GPIO6 |
| 6 | GND | Ground | - |

### TELE Connector (6-pin JST-GH)
| Pin | Signal | Function | RP2040 GPIO |
|-----|--------|----------|-------------|
| 1 | VCC | 5V Power | - |
| 2 | TX | Telemetry TX | GPIO0 |
| 3 | RX | Telemetry RX | GPIO1 |
| 4 | SCL | I2C Clock | GPIO3 |
| 5 | SDA | I2C Data | GPIO2 |
| 6 | GND | Ground | - |

### POWER Connector (6-pin JST-GH)
| Pin | Signal | Function | RP2040 GPIO |
|-----|--------|----------|-------------|
| 1 | VCC | Battery + | - |
| 2 | VCC | Battery + | - |
| 3 | CURR_SENSE | Current Sensor | GPIO28 (ADC2) |
| 4 | VOLT_SENSE | Voltage Sensor | GPIO27 (ADC1) |
| 5 | - | Reserved | - |
| 6 | GND | Ground | - |

### ESC Connectors (4x 3-pin headers)
| Connector | Pin 1 | Pin 2 | Pin 3 | GPIO |
|-----------|-------|-------|-------|------|
| ESC0 | GND | VCC (5V) | Signal | GPIO20 |
| ESC1 | GND | VCC (5V) | Signal | GPIO21 |
| ESC2 | GND | VCC (5V) | Signal | GPIO22 |
| ESC3 | GND | VCC (5V) | Signal | GPIO23 |

### RC Connector (3-pin header)
| Pin | Signal | Function | GPIO |
|-----|--------|----------|------|
| 1 | GND | Ground | - |
| 2 | VCC | 5V Power | - |
| 3 | Signal | RC PPM/PWM | GPIO24 |

### DBG Connector (3-pin header)
| Pin | Signal | Function | GPIO |
|-----|--------|----------|------|
| 1 | GND | Ground | - |
| 2 | TX | Debug UART TX | TBD |
| 3 | RX | Debug UART RX | TBD |

---

## 🎨 RGB LED Configuration

**Component**: FM-B2020RGBA-HG (NATIONSTAR)  
**Type**: 4-channel Common-Anode RGBA LED  
**Package**: SMD 2020 (2.0mm x 2.0mm)

### Pin Configuration
| LED Pin | Channel | RP2040 GPIO | Current Limit Resistor |
|---------|---------|-------------|------------------------|
| 1 | Red Cathode | GPIO13 | R14 (162Ω) |
| 2 | Common Anode | +3.3V | - |
| 3 | Green Cathode | GPIO14 | R16 (100Ω) |
| 4 | Blue Cathode | GPIO15 | R15 (60.4Ω) |
| 5 | White/Amber* | GPIO?? | R17 (140Ω) |

**Note**: Pin 5 (4th channel) may be White or Amber. Need to verify if connected.

### Drive Configuration
- **Type**: Common Anode (active LOW)
- **Logic**: LOW = LED ON, HIGH = LED OFF
- **Current**: ~10-15mA per channel (depends on resistor)
- **PWM**: Capable for brightness control

### Expected Forward Voltages @ 20mA
- Red: ~2.0V → I = (3.3V - 2.0V) / 162Ω = 8.0mA
- Green: ~3.0V → I = (3.3V - 3.0V) / 100Ω = 3.0mA  
- Blue: ~3.0V → I = (3.3V - 3.0V) / 60.4Ω = 5.0mA

---

## 💾 Storage Devices

### External Flash (QSPI)
**Component**: W25Q128JVSIQTR (Winbond)  
**Capacity**: 128Mbit (16MB)  
**Interface**: Quad-SPI (QSPI)  
**Connection**: Dedicated QSPI pins (not GPIOs)  
**Usage**: 
- Firmware storage (bootloader area)
- Parameter backup
- Flight logs (limited capacity)

### MicroSD Card Slot
**Component**: 503398-1892 (Molex)  
**Interface**: SPI (GPIO16-19) or SDIO mode  
**Card Detect**: GPIO17 (if implemented)  
**Usage**:
- High-speed data logging
- Mission files
- Parameter backup
- Blackbox recording

**SPI Pin Assignment**:
| Signal | RP2040 GPIO | SD Card Pin |
|--------|-------------|-------------|
| CS | GPIO16 | CS/DAT3 |
| SCK | GPIO18 | CLK |
| MOSI | GPIO19 | CMD/DI |
| MISO | GPIO16 | DAT0/DO |
| CD (Card Detect) | GPIO17 | CD pin |

---

## 📡 Communication Interfaces

### UART Assignments
| UART | TX GPIO | RX GPIO | Purpose | Baud Rate |
|------|---------|---------|---------|-----------|
| UART0 | GPIO0 | GPIO1 | Telemetry | 57600 default |
| UART1 | GPIO4 | GPIO5 | GPS | 9600/115200 |

### I2C Buses
| Bus | SDA GPIO | SCL GPIO | Purpose | Devices |
|-----|----------|----------|---------|---------|
| I2C0 | GPIO6 | GPIO7 | GPS/Magnetometer | External compass |
| I2C1 | GPIO2 | GPIO3 | Telemetry | General purpose |

### SPI Buses
| Bus | MISO | MOSI | SCK | CS Pins | Purpose |
|-----|------|------|-----|---------|---------|
| SPI0 | GPIO8 | GPIO11 | GPIO10 | GPIO9, GPIO12 | IMU + Barometer |
| SPI1 | GPIO16 | GPIO19 | GPIO18 | GPIO16 | MicroSD Card |

---

## 🔋 Power and ADC

### ADC Channel Assignments
| ADC Channel | GPIO | Function | Voltage Divider | Range |
|-------------|------|----------|-----------------|-------|
| ADC0 | GPIO26 | Unused/Debug | - | 0-3.3V |
| ADC1 | GPIO27 | Battery Voltage | R6 (2kΩ) + R7 (2.7kΩ) | 0-18V* |
| ADC2 | GPIO28 | Battery Current | Current sensor | 0-60A* |
| ADC3 | GPIO29 | USB VBUS Detect | R1 (1kΩ) divider | 0-5V |

**Note**: Actual voltage/current ranges depend on external sensor scaling. Verify from schematic.

### Voltage Divider Calculations
**Battery Voltage (GPIO27)**:
- R6 = 2kΩ (to ADC)
- R7 = 2.7kΩ (to GND)
- Ratio = (R6 + R7) / R7 = 4.7kΩ / 2.7kΩ = 1.74
- Max input voltage = 3.3V × 1.74 = 5.74V

**⚠️ WARNING**: This divider ratio seems low for battery monitoring. Typical 4S LiPo = 16.8V max.  
**Action Required**: Verify actual resistor values and divider configuration from schematic.

---

## 🔊 Audio

### Buzzer
**Component**: QMB-09B-03 (HNDZ)  
**Type**: Passive (requires PWM)  
**GPIO**: GPIO25  
**Optimal Frequency**: 2700Hz ± 300Hz (range: 2400-3000Hz)  
**Drive**: PWM square wave at 2700Hz for maximum volume  
**Current**: ~30mA @ 3.3V  
**Recommended Tone Frequencies**:
- Standard alert: 2700Hz (optimal)
- Warning: 2600Hz
- Error: 2800Hz
- Arming: 2700Hz chirps
**Note**: Stay within 2400-3000Hz range for best acoustic performance

---

## 🎮 User Interface

### Buttons
| Component | Function | Connection | Type |
|-----------|----------|------------|------|
| ARM | Arming button | GPIO (TBD) | Tactile switch |
| BOOTSEL | Boot mode | RP2040 BOOTSEL | Hardware function |

### Status Indicators
- RGB LED (GPIO13/14/15): System status
- Buzzer (GPIO25): Audio feedback

---

## 📝 Configuration Summary

### Verified and Correct
- ✅ IMU (LSM6DS3TR-C) on SPI0: CS=GPIO9
- ✅ Barometer (DPS310) on SPI0: CS=GPIO12
- ✅ PWM outputs (4x ESCs): GPIO20-23
- ✅ RC input: GPIO24
- ✅ Buzzer: GPIO25
- ✅ UART/I2C interfaces correctly assigned
- ✅ ADC channels mapped

### Needs Configuration
- ⚠️ **CRITICAL**: MicroSD card driver (GPIO16-19)
- ⚠️ External flash (QSPI) logging configuration
- ⚠️ RGB LED polarity (common anode = active LOW)
- ⚠️ Battery voltage divider scaling verification
- ⚠️ ARM button GPIO assignment
- ⚠️ 4th LED channel (if used)

### Not Used / Reserved
- GPIO26 (ADC0): Available for expansion
- Debug connector: UART assignment TBD

---

## 🚨 Critical Issues Found

1. **Battery Voltage Divider**: Current ratio (1.74x) cannot measure 4S LiPo batteries
   - Need to verify actual resistor network from schematic
   - May need software scaling factor correction

2. **MicroSD Card**: No driver configured
   - Required for data logging in PX4
   - Should be high priority feature

3. **RGB LED**: Wrong polarity in current config
   - FM-B2020RGBA-HG is common-anode (active LOW)
   - Current code uses active HIGH

---

**Document Version**: 1.0  
**Last Updated**: October 30, 2024  
**Status**: 🟡 Pending schematic verification for battery divider and SD card CD pin
