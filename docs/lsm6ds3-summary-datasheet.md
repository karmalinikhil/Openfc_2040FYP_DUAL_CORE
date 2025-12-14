# LSM6DS3TR-C: Comprehensive Datasheet Reference

This document provides an exhaustive reference for the LSM6DS3TR-C iNEMO Inertial Module, with a specific comprehensive focus on the **SPI Interface** and **Data Transmission Registers**.

---

## 1. SPI Interface Characteristics

The LSM6DS3TR-C SPI is a bus slave. The SPI allows writing and reading the registers of the device. The serial interface interacts with the application using 4 wires: **CS, SPC, SDI and SDO**.

### 1.1 Pin Connections


| Pin Name | Function | Description |
| :--- | :--- | :--- |
| **CS** | Chip Select | Serial port enable. Controlled by SPI master. Goes low at start of transmission, high at end. |
| **SPC** | Serial Port Clock | Controlled by SPI master. Stopped high when CS is high (no transmission). |
| **SDI** | Serial Data Input | Driven at the falling edge of SPC; captured at the rising edge of SPC. |
| **SDO** | Serial Data Output | Driven at the falling edge of SPC; captured at the rising edge of SPC. |

### 1.2 SPI Slave Timing Values
*Subject to general operating conditions for Vdd and Top.* 

| Symbol | Parameter | Min | Value | Max | Unit |
| :--- | :--- | :--- | :--- | :--- | :--- |
| $t_{c(SPC)}$ | SPI clock cycle | 100 | | | ns |
| $f_{c(SPC)}$ | SPI clock frequency | | | 10 | MHz |
| $t_{su(CS)}$ | CS setup time | 5 | | | ns |
| $t_{h(CS)}$ | CS hold time | 20 | | | ns |
| $t_{su(SI)}$ | SDI input setup time | 5 | | | ns |
| $t_{h(SI)}$ | SDI input hold time | 15 | | | ns |
| $t_{v(SO)}$ | SDO valid output time | | | 50 | ns |
| $t_{h(SO)}$ | SDO output hold time | 5 | | | ns |
| $t_{dis(SO)}$ | SDO output disable time | | | 50 | ns |

*Note: Values are guaranteed at 10 MHz clock frequency for SPI with both 4 and 3 wires.* 

### 1.3 SPI Read/Write Protocol

**General Rule:**
* Bit duration is the time between two falling edges of SPC.
* Commands are completed in 16 clock pulses (or multiples of 8 for multiple bytes).
* **MSb first**: Data is transferred with the Most Significant Bit first. 

**Command Structure (First 8 bits):**
The first byte contains the instruction and address.
* **Bit 0 (RW bit):**
    * `0`: Write data to device.
    * `1`: Read data from device.
* **Bit 1-7 (AD[6:0]):** Address of the indexed register. [cite: 737-739]

#### 1.3.1 SPI Read Command
1.  **CS** goes Low.
2.  **Master sends:** RW bit (`1`) + Address `AD[6:0]`.
3.  **Device drives SDO:** Sends Data `DO[7:0]` starting at bit 8.
4.  **CS** goes High to end, or continues for multiple bytes. [cite: 763-767]

#### 1.3.2 SPI Write Command
1.  **CS** goes Low.
2.  **Master sends:** RW bit (`0`) + Address `AD[6:0]`.
3.  **Master sends:** Data `DI[7:0]`.
4.  **CS** goes High. [cite: 789-793]

#### 1.3.3 Multiple Byte Read/Write
To read or write multiple bytes sequentially:
1.  Set the **IF_INC** bit in **CTRL3_C (12h)** register to `1` (default is `1`).
2.  Keep **CS** Low after the first data byte.
3.  The address is automatically incremented for every subsequent block of 8 clock pulses. 

#### 1.3.4 SPI 3-Wire Mode
A 3-wire mode (using SDI/O as bidirectional) is entered by setting bit **SIM** to `1` in register **CTRL3_C (12h)**. 

---

## 2. Registers for Accelerometer & Gyroscope Data Transmission

These registers contain the raw output data from the sensors. The data is expressed as a **16-bit word in two's complement**. To retrieve the physical value, combine the High and Low bytes and multiply by the sensitivity associated with the selected Full Scale (FS).

### 2.1 Status Register (Data Ready)
Before reading data, check this register to ensure new data is available.

**STATUS_REG (1Eh)** [cite: 1359-1364]

| Bit | Name | Description |
| :--- | :--- | :--- |
| 2 | **TDA** | Temperature new data available. (0: no new data; 1: new data available) |
| 1 | **GDA** | Gyroscope new data available. (0: no new data; 1: new data available) |
| 0 | **XLDA** | Accelerometer new data available. (0: no new data; 1: new data available) |

### 2.2 Gyroscope Output Registers
*Values are compliant with the FS and ODR settings in CTRL2_G (11h).* 

| Address (Hex) | Name | Description | Source Ref |
| :--- | :--- | :--- | :--- |
| **22** | **OUTX_L_G** | Pitch axis (X) angular rate value (LSByte) |  |
| **23** | **OUTX_H_G** | Pitch axis (X) angular rate value (MSByte) |  |
| **24** | **OUTY_L_G** | Roll axis (Y) angular rate value (LSByte) |  |
| **25** | **OUTY_H_G** | Roll axis (Y) angular rate value (MSByte) |  |
| **26** | **OUTZ_L_G** | Yaw axis (Z) angular rate value (LSByte) |  |
| **27** | **OUTZ_H_G** | Yaw axis (Z) angular rate value (MSByte) |  |

### 2.3 Accelerometer Output Registers
*Values are compliant with the FS and ODR settings in CTRL1_XL (10h).* 

| Address (Hex) | Name | Description | Source Ref |
| :--- | :--- | :--- | :--- |
| **28** | **OUTX_L_XL** | X-axis linear acceleration value (LSByte) |  |
| **29** | **OUTX_H_XL** | X-axis linear acceleration value (MSByte) |  |
| **2A** | **OUTY_L_XL** | Y-axis linear acceleration value (LSByte) |  |
| **2B** | **OUTY_H_XL** | Y-axis linear acceleration value (MSByte) |  |
| **2C** | **OUTZ_L_XL** | Z-axis linear acceleration value (LSByte) |  |
| **2D** | **OUTZ_H_XL** | Z-axis linear acceleration value (MSByte) |  |

### 2.4 Temperature Output Registers
*The value is expressed as two's complement.* 

| Address (Hex) | Name | Description |
| :--- | :--- | :--- |
| **20** | **OUT_TEMP_L** | Temperature output data (LSByte) |
| **21** | **OUT_TEMP_H** | Temperature output data (MSByte) |

---

## 3. Configuration Registers (Context for Data)

To interpret the data read from the registers above correctly, you must know the **Full Scale (FS)** and **Output Data Rate (ODR)** configuration.

### 3.1 CTRL1_XL (10h) - Accelerometer Config
[cite: 1163-1172]

| Bit | Name | Function |
| :--- | :--- | :--- |
| 7-4 | **ODR_XL [3:0]** | Output Data Rate selection (e.g., `1010` = 6.66 kHz). |
| 3-2 | **FS_XL [1:0]** | Full-scale selection: <br> `00`: ±2 g <br> `01`: ±16 g <br> `10`: ±4 g <br> `11`: ±8 g |
| 1 | **LPF1_BW_SEL** | LPF1 bandwidth selection. |
| 0 | **BW0_XL** | Analog chain bandwidth selection. |

### 3.2 CTRL2_G (11h) - Gyroscope Config
[cite: 1181-1187]

| Bit | Name | Function |
| :--- | :--- | :--- |
| 7-4 | **ODR_G [3:0]** | Output Data Rate selection. |
| 3-2 | **FS_G [1:0]** | Full-scale selection: <br> `00`: 245 dps <br> `01`: 500 dps <br> `10`: 1000 dps <br> `11`: 2000 dps |
| 1 | **FS_125** | Full-scale at 125 dps (0: disabled; 1: enabled). |
| 0 | **0** | Must be set to 0. |

### 3.3 CTRL3_C (12h) - Interface Config
[cite: 1195-1200]

| Bit | Name | Function |
| :--- | :--- | :--- |
| 2 | **IF_INC** | **Register address auto-increment.** <br> `0`: Disabled <br> `1`: Enabled (Default). <br> *Critical for multi-byte SPI reading.* |
| 6 | **BDU** | **Block Data Update.** <br> `0`: Continuous update. <br> `1`: Output registers not updated until MSB and LSB have been read. (Prevents reading high/low bytes from different samples). |