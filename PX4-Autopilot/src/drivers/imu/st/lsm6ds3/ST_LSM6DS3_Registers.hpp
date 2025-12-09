/****************************************************************************
 *
 *   Copyright (c) 2024 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file ST_LSM6DS3_Registers.hpp
 *
 * ST LSM6DS3TR-C registers.
 * Based on LSM6DS3TR-C datasheet (DocID029449 Rev 4)
 *
 */

#pragma once

#include <cstdint>

// Bit position helpers
static constexpr uint8_t Bit0 = (1 << 0);
static constexpr uint8_t Bit1 = (1 << 1);
static constexpr uint8_t Bit2 = (1 << 2);
static constexpr uint8_t Bit3 = (1 << 3);
static constexpr uint8_t Bit4 = (1 << 4);
static constexpr uint8_t Bit5 = (1 << 5);
static constexpr uint8_t Bit6 = (1 << 6);
static constexpr uint8_t Bit7 = (1 << 7);

namespace ST_LSM6DS3
{

static constexpr uint32_t SPI_SPEED = 10 * 1000 * 1000; // 10 MHz SPI clock frequency

static constexpr uint8_t DIR_READ = 0x80;

static constexpr uint8_t WHO_AM_I_ID = 0x6A; // LSM6DS3TR-C WHO_AM_I value

// Output Data Rates (ODR) - using 833 Hz
static constexpr uint32_t LA_ODR = 833; // Linear acceleration output data rate
static constexpr uint32_t G_ODR  = 833; // Angular rate output data rate

enum class Register : uint8_t {
	// FIFO configuration registers
	FIFO_CTRL1      = 0x06, // FIFO threshold setting
	FIFO_CTRL2      = 0x07, // FIFO threshold setting
	FIFO_CTRL3      = 0x08, // FIFO decimation setting
	FIFO_CTRL4      = 0x09, // FIFO mode setting
	FIFO_CTRL5      = 0x0A, // FIFO ODR and mode setting

	// Control registers
	INT1_CTRL       = 0x0D, // INT1 pin control
	INT2_CTRL       = 0x0E, // INT2 pin control
	WHO_AM_I        = 0x0F, // Device identification register

	CTRL1_XL        = 0x10, // Accelerometer control register 1
	CTRL2_G         = 0x11, // Gyroscope control register 2
	CTRL3_C         = 0x12, // Control register 3
	CTRL4_C         = 0x13, // Control register 4
	CTRL5_C         = 0x14, // Control register 5
	CTRL6_C         = 0x15, // Control register 6 (Gyro)
	CTRL7_G         = 0x16, // Gyroscope control register 7
	CTRL8_XL        = 0x17, // Accelerometer control register 8
	CTRL9_XL        = 0x18, // Accelerometer control register 9
	CTRL10_C        = 0x19, // Control register 10

	// Status register
	STATUS_REG      = 0x1E, // Status register

	// Temperature output
	OUT_TEMP_L      = 0x20,
	OUT_TEMP_H      = 0x21,

	// Gyroscope output registers (different from LSM9DS1!)
	OUTX_L_G        = 0x22,
	OUTX_H_G        = 0x23,
	OUTY_L_G        = 0x24,
	OUTY_H_G        = 0x25,
	OUTZ_L_G        = 0x26,
	OUTZ_H_G        = 0x27,

	// Accelerometer output registers (same as LSM9DS1)
	OUTX_L_XL       = 0x28,
	OUTX_H_XL       = 0x29,
	OUTY_L_XL       = 0x2A,
	OUTY_H_XL       = 0x2B,
	OUTZ_L_XL       = 0x2C,
	OUTZ_H_XL       = 0x2D,

	// FIFO status registers
	FIFO_STATUS1    = 0x3A, // Number of unread words in FIFO (low)
	FIFO_STATUS2    = 0x3B, // Number of unread words in FIFO (high) + flags
	FIFO_STATUS3    = 0x3C, // FIFO pattern (low)
	FIFO_STATUS4    = 0x3D, // FIFO pattern (high)

	// FIFO data output
	FIFO_DATA_OUT_L = 0x3E,
	FIFO_DATA_OUT_H = 0x3F,
};

// CTRL1_XL - Accelerometer control register 1
enum CTRL1_XL_BIT : uint8_t {
	// ODR_XL [7:4] - Output data rate and power mode selection
	// LSM6DS3TR-C datasheet: 0100 = 104 Hz (Bit6 set)
	ODR_XL_833HZ  = Bit6 | Bit5 | Bit4,  // 0x70: 833 Hz
	ODR_XL_416HZ  = Bit6 | Bit5,         // 0x60: 416 Hz
	ODR_XL_208HZ  = Bit6 | Bit4,         // 0x50: 208 Hz
	ODR_XL_104HZ  = Bit6,                // 0x40: 104 Hz

	// FS_XL [3:2] - Full-scale selection
	FS_XL_2G      = 0,                   // 00: ±2g
	FS_XL_4G      = Bit3,                // 10: ±4g
	FS_XL_8G      = Bit3 | Bit2,         // 11: ±8g
	FS_XL_16G     = Bit2,                // 01: ±16g

	// BW_XL [1:0] - Anti-aliasing filter bandwidth
	BW_XL_400HZ   = 0,                   // 00: 400 Hz
	BW_XL_200HZ   = Bit0,                // 01: 200 Hz
	BW_XL_100HZ   = Bit1,                // 10: 100 Hz
	BW_XL_50HZ    = Bit1 | Bit0,         // 11: 50 Hz
};

// CTRL2_G - Gyroscope control register 2
enum CTRL2_G_BIT : uint8_t {
	// ODR_G [7:4] - Output data rate selection
	// LSM6DS3TR-C datasheet: 0100 = 104 Hz (Bit6 set)
	ODR_G_833HZ   = Bit6 | Bit5 | Bit4,  // 0x70: 833 Hz
	ODR_G_416HZ   = Bit6 | Bit5,         // 0x60: 416 Hz
	ODR_G_208HZ   = Bit6 | Bit4,         // 0x50: 208 Hz
	ODR_G_104HZ   = Bit6,                // 0x40: 104 Hz

	// FS_G [3:2] - Full-scale selection
	FS_G_250DPS   = 0,                   // 00: 250 dps
	FS_G_500DPS   = Bit2,                // 01: 500 dps
	FS_G_1000DPS  = Bit3,                // 10: 1000 dps
	FS_G_2000DPS  = Bit3 | Bit2,         // 11: 2000 dps

	// FS_125 [1] - 125 dps full-scale selection
	FS_G_125DPS   = Bit1,                // 125 dps mode
};

// CTRL3_C - Control register 3
enum CTRL3_C_BIT : uint8_t {
	BOOT        = Bit7, // Reboot memory content
	BDU         = Bit6, // Block data update
	H_LACTIVE   = Bit5, // Interrupt activation level
	PP_OD       = Bit4, // Push-pull/open-drain selection
	SIM         = Bit3, // SPI serial interface mode (0: 4-wire, 1: 3-wire)
	IF_INC      = Bit2, // Register address automatically incremented
	BLE         = Bit1, // Big/little endian data selection
	SW_RESET    = Bit0, // Software reset
};

// CTRL4_C - Control register 4
enum CTRL4_C_BIT : uint8_t {
	XL_BW_SCAL_ODR = Bit7, // Accelerometer bandwidth selection
	SLEEP_G        = Bit6, // Gyroscope sleep mode enable
	INT2_on_INT1   = Bit5, // All interrupt signals on INT1
	FIFO_TEMP_EN   = Bit4, // Temperature data stored in FIFO
	DRDY_MASK      = Bit3, // Data-ready pulsed mode
	I2C_disable    = Bit2, // Disable I2C interface
	LPF1_SEL_G     = Bit1, // Enable gyro LPF1
};

// CTRL6_C - Control register 6 (Gyroscope)
enum CTRL6_C_BIT : uint8_t {
	TRIG_EN       = Bit7, // Trigger mode enable
	LVL_EN        = Bit6, // Level-sensitive latched mode enable
	LVL2_EN       = Bit5, // Level-sensitive mode enable
	XL_HM_MODE    = Bit4, // High-performance operating mode disable for accel
};

// CTRL7_G - Gyroscope control register 7
enum CTRL7_G_BIT : uint8_t {
	G_HM_MODE     = Bit7, // High-performance operating mode disable for gyro
	HP_EN_G       = Bit6, // Gyroscope digital HP filter enable
	HPM_G_16mHz   = 0,           // HP filter cutoff: 16 mHz
	HPM_G_65mHz   = Bit4,        // HP filter cutoff: 65 mHz
	HPM_G_260mHz  = Bit5,        // HP filter cutoff: 260 mHz
	HPM_G_1_04Hz  = Bit5 | Bit4, // HP filter cutoff: 1.04 Hz
};

// STATUS_REG - Status register
enum STATUS_REG_BIT : uint8_t {
	TDA  = Bit2, // Temperature new data available
	GDA  = Bit1, // Gyroscope new data available
	XLDA = Bit0, // Accelerometer new data available
};

// FIFO_CTRL5 - FIFO control register 5
enum FIFO_CTRL5_BIT : uint8_t {
	// ODR_FIFO [6:3] - FIFO output data rate
	// LSM6DS3TR-C datasheet: bits [6:3] encode rate
	ODR_FIFO_833HZ  = Bit5 | Bit4 | Bit3,  // 0x38: 833 Hz
	ODR_FIFO_416HZ  = Bit5 | Bit4,         // 0x30: 416 Hz

	// FIFO_MODE [2:0] - FIFO mode selection
	FIFO_MODE_BYPASS     = 0,                     // 000: Bypass mode
	FIFO_MODE_FIFO       = Bit0,                  // 001: FIFO mode
	FIFO_MODE_CONTINUOUS = Bit2 | Bit1,           // 110: Continuous mode
};

// FIFO_CTRL3 - FIFO decimation setting
enum FIFO_CTRL3_BIT : uint8_t {
	// DEC_FIFO_GYRO [5:3] - Gyro decimation
	DEC_FIFO_GYRO_NOT_IN_FIFO = 0,
	DEC_FIFO_GYRO_NO_DEC      = Bit3,  // No decimation

	// DEC_FIFO_XL [2:0] - Accel decimation
	DEC_FIFO_XL_NOT_IN_FIFO   = 0,
	DEC_FIFO_XL_NO_DEC        = Bit0,  // No decimation
};

// FIFO_STATUS2 bits
enum FIFO_STATUS2_BIT : uint8_t {
	FIFO_FULL_SMART   = Bit6, // Smart FIFO full status
	FIFO_OVER_RUN     = Bit6, // FIFO overrun status (same bit)
	FIFO_EMPTY        = Bit4, // FIFO empty
	DIFF_FIFO_H       = Bit3 | Bit2 | Bit1 | Bit0, // High bits of unread FIFO samples
};

namespace FIFO
{
// LSM6DS3 has a 4KB FIFO = 4096 bytes
// Each sample set (gyro + accel) = 12 bytes (6 bytes gyro + 6 bytes accel)
// Max samples = 4096 / 12 = 341 samples
static constexpr size_t SIZE = 4096;
static constexpr size_t MAX_SAMPLES = 341;
}

} // namespace ST_LSM6DS3
