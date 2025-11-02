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
 *
 */

#pragma once

#include <cstdint>

namespace ST_LSM6DS3
{

// SPI protocol uses register address with MSB set for read operation
static constexpr uint8_t DIR_READ = 0x80;
static constexpr uint8_t DIR_WRITE = 0x00;

// Device ID
static constexpr uint8_t WHO_AM_I_ID = 0x6A;

// Module name for perf counters
static constexpr const char *MODULE_NAME = "LSM6DS3";

enum class Register : uint8_t {
	FUNC_CFG_ACCESS     = 0x01,

	FIFO_CTRL1          = 0x06,
	FIFO_CTRL2          = 0x07,
	FIFO_CTRL3          = 0x08,
	FIFO_CTRL4          = 0x09,
	FIFO_CTRL5          = 0x0A,
	ORIENT_CFG_G        = 0x0B,

	INT1_CTRL           = 0x0D,
	INT2_CTRL           = 0x0E,
	WHO_AM_I            = 0x0F,
	CTRL1_XL            = 0x10,
	CTRL2_G             = 0x11,
	CTRL3_C             = 0x12,
	CTRL4_C             = 0x13,
	CTRL5_C             = 0x14,
	CTRL6_C             = 0x15,
	CTRL7_G             = 0x16,
	CTRL8_XL            = 0x17,
	CTRL9_XL            = 0x18,
	CTRL10_C            = 0x19,

	WAKE_UP_SRC         = 0x1B,
	TAP_SRC             = 0x1C,
	D6D_SRC             = 0x1D,
	STATUS_REG          = 0x1E,

	OUT_TEMP_L          = 0x20,
	OUT_TEMP_H          = 0x21,
	OUTX_L_G            = 0x22,
	OUTX_H_G            = 0x23,
	OUTY_L_G            = 0x24,
	OUTY_H_G            = 0x25,
	OUTZ_L_G            = 0x26,
	OUTZ_H_G            = 0x27,
	OUTX_L_XL           = 0x28,
	OUTX_H_XL           = 0x29,
	OUTY_L_XL           = 0x2A,
	OUTY_H_XL           = 0x2B,
	OUTZ_L_XL           = 0x2C,
	OUTZ_H_XL           = 0x2D,

	FIFO_STATUS1        = 0x3A,
	FIFO_STATUS2        = 0x3B,
	FIFO_STATUS3        = 0x3C,
	FIFO_STATUS4        = 0x3D,
	FIFO_DATA_OUT_L     = 0x3E,
	FIFO_DATA_OUT_H     = 0x3F,
	TIMESTAMP0_REG      = 0x40,
	TIMESTAMP1_REG      = 0x41,
	TIMESTAMP2_REG      = 0x42,

	STEP_TIMESTAMP_L    = 0x49,
	STEP_TIMESTAMP_H    = 0x4A,
	STEP_COUNTER_L      = 0x4B,
	STEP_COUNTER_H      = 0x4C,

	FUNC_SRC            = 0x53,

	TAP_CFG             = 0x58,
	TAP_THS_6D          = 0x59,
	INT_DUR2            = 0x5A,
	WAKE_UP_THS         = 0x5B,
	WAKE_UP_DUR         = 0x5C,
	FREE_FALL           = 0x5D,
	MD1_CFG             = 0x5E,
	MD2_CFG             = 0x5F,
};

// CTRL1_XL register bits
enum CTRL1_XL_BIT : uint8_t {
	ODR_XL_POWER_DOWN   = 0x00,  // 0000
	ODR_XL_12_5HZ       = 0x10,  // 0001
	ODR_XL_26HZ         = 0x20,  // 0010
	ODR_XL_52HZ         = 0x30,  // 0011
	ODR_XL_104HZ        = 0x40,  // 0100
	ODR_XL_208HZ        = 0x50,  // 0101
	ODR_XL_416HZ        = 0x60,  // 0110
	ODR_XL_833HZ        = 0x70,  // 0111
	ODR_XL_1_66KHZ      = 0x80,  // 1000
	ODR_XL_3_33KHZ      = 0x90,  // 1001
	ODR_XL_6_66KHZ      = 0xA0,  // 1010

	FS_XL_2G            = 0x00,  // 00
	FS_XL_16G           = 0x04,  // 01
	FS_XL_4G            = 0x08,  // 10
	FS_XL_8G            = 0x0C,  // 11

	BW_XL_400HZ         = 0x00,
	BW_XL_200HZ         = 0x01,
	BW_XL_100HZ         = 0x02,
	BW_XL_50HZ          = 0x03,
};

// CTRL2_G register bits
enum CTRL2_G_BIT : uint8_t {
	ODR_G_POWER_DOWN    = 0x00,  // 0000
	ODR_G_12_5HZ        = 0x10,  // 0001
	ODR_G_26HZ          = 0x20,  // 0010
	ODR_G_52HZ          = 0x30,  // 0011
	ODR_G_104HZ         = 0x40,  // 0100
	ODR_G_208HZ         = 0x50,  // 0101
	ODR_G_416HZ         = 0x60,  // 0110
	ODR_G_833HZ         = 0x70,  // 0111
	ODR_G_1_66KHZ       = 0x80,  // 1000

	FS_G_250DPS         = 0x00,  // 00
	FS_G_500DPS         = 0x04,  // 01
	FS_G_1000DPS        = 0x08,  // 10
	FS_G_2000DPS        = 0x0C,  // 11
};

// CTRL3_C register bits
enum CTRL3_C_BIT : uint8_t {
	SW_RESET            = 0x01,
	BLE                 = 0x02,
	IF_INC              = 0x04,
	SIM                 = 0x08,
	PP_OD               = 0x10,
	H_LACTIVE           = 0x20,
	BDU                 = 0x40,
	BOOT                = 0x80,
};

// CTRL4_C register bits
enum CTRL4_C_BIT : uint8_t {
	STOP_ON_FTH         = 0x01,
	NOT_USED_01         = 0x02,
	I2C_DISABLE         = 0x04,
	DRDY_MASK           = 0x08,
	FIFO_TEMP_EN        = 0x10,
	INT2_ON_INT1        = 0x20,
	SLEEP_G             = 0x40,
	XL_BW_SCAL_ODR      = 0x80,
};

// CTRL5_C register bits
enum CTRL5_C_BIT : uint8_t {
	ST_XL_0             = 0x01,
	ST_XL_1             = 0x02,
	ST_G_0              = 0x04,
	ST_G_1              = 0x08,
	ROUNDING_0          = 0x10,
	ROUNDING_1          = 0x20,
	ROUNDING_2          = 0x40,
	XL_ULP_EN           = 0x80,
};

// FIFO_CTRL5 register bits
enum FIFO_CTRL5_BIT : uint8_t {
	FIFO_MODE_BYPASS    = 0x00,
	FIFO_MODE_FIFO      = 0x01,
	FIFO_MODE_CONT_TO_FIFO = 0x03,
	FIFO_MODE_BYPASS_TO_CONT = 0x04,
	FIFO_MODE_CONTINUOUS = 0x06,

	ODR_FIFO_DISABLED   = 0x00,
	ODR_FIFO_12_5HZ     = 0x08,
	ODR_FIFO_26HZ       = 0x10,
	ODR_FIFO_52HZ       = 0x18,
	ODR_FIFO_104HZ      = 0x20,
	ODR_FIFO_208HZ      = 0x28,
	ODR_FIFO_416HZ      = 0x30,
	ODR_FIFO_833HZ      = 0x38,
	ODR_FIFO_1_66KHZ    = 0x40,
	ODR_FIFO_3_33KHZ    = 0x48,
	ODR_FIFO_6_66KHZ    = 0x50,
};

// STATUS_REG bits
enum STATUS_REG_BIT : uint8_t {
	XLDA                = 0x01,  // Accelerometer new data available
	GDA                 = 0x02,  // Gyroscope new data available
	TDA                 = 0x04,  // Temperature new data available
};

} // namespace ST_LSM6DS3
