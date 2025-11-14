/****************************************************************************
 *
 *   Copyright (C) 2024 PX4 Development Team. All rights reserved.
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
 * @file spi.cpp
 * 
 * OpenFC2040 SPI bus configuration
 */

#include <px4_arch/spi_hw_description.h>
#include <drivers/drv_sensor.h>
#include <nuttx/spi/spi.h>

/*
 * OpenFC2040 SPI Configuration:
 * 
 * SPI0 Bus (SD Card):
 * - SCK:  GPIO18
 * - MOSI: GPIO19 (CMD)
 * - MISO: GPIO16 (DAT0)
 * - CS:   GPIO17 (CD/DAT3)
 * 
 * SPI1 Bus (Sensors):
 * - SCK:  GPIO10
 * - MOSI: GPIO11
 * - MISO: GPIO8
 * 
 * Devices on SPI1:
 * - LSM6DS3TR-C IMU: CS on GPIO9
 * - DPS310 Barometer: CS on GPIO12
 */

constexpr px4_spi_bus_t px4_spi_buses[SPI_BUS_MAX_BUS_ITEMS] = {
	// SPI0: SD Card on GPIO16-19
	initSPIBus(SPI::Bus::SPI0, {
		initSPIDevice(SPIDEV_MMCSD(0), SPI::CS{GPIO::Pin17}),  // SD card CS on GPIO17
	}),
	// SPI1: Sensors (IMU and Barometer)
	initSPIBus(SPI::Bus::SPI1, {
		// Temporary IMU device type until LSM6DS3 driver is available
		initSPIDevice(DRV_IMU_DEVTYPE_LSM303D, SPI::CS{GPIO::Pin9}),
		// DPS310 Barometer on GPIO12
		initSPIDevice(DRV_BARO_DEVTYPE_DPS310, SPI::CS{GPIO::Pin12}),
	}),
};

static constexpr bool unused = validateSPIConfig(px4_spi_buses);
