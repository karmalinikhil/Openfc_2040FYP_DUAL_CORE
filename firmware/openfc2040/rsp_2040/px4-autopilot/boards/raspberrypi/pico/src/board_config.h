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
 * @file board_config.h
 *
 * OpenFC2040 board internal definitions
 */

#pragma once

/****************************************************************************************************
 * Included Files
 ****************************************************************************************************/

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>

/* OpenFC2040 Hardware Configuration */

/* RGB LED Configuration */
/* FM-B2020RGBA-HG is a common-anode RGB LED (active LOW) */
#define GPIO_LED_RED    PX4_MAKE_GPIO_OUTPUT_SET(13)  // GPIO13 - Active LOW
#define GPIO_LED_GREEN  PX4_MAKE_GPIO_OUTPUT_SET(14)  // GPIO14 - Active LOW
#define GPIO_LED_BLUE   PX4_MAKE_GPIO_OUTPUT_SET(15)  // GPIO15 - Active LOW

#define BOARD_OVERLOAD_LED     LED_RED
#define BOARD_ARMED_LED        LED_BLUE

/* ADC Channels Configuration */
#define ADC_CHANNELS (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)

#define ADC_BATTERY_VOLTAGE_CHANNEL  1  // GPIO27 - BATT_V
#define ADC_BATTERY_CURRENT_CHANNEL  2  // GPIO28 - BATT_A
#define ADC_RC_RSSI_CHANNEL          0  // Reserved
#define ADC_USB_VBUS_CHANNEL         3  // GPIO29 - VBUS detection

/* High-resolution timer */
#define HRT_TIMER 1
#define HRT_TIMER_CHANNEL 1
#define HRT_PPM_CHANNEL 1
#define GPIO_PPM_IN		(24 | GPIO_FUN(RP2040_GPIO_FUNC_SIO))  // GPIO24 - RC Input
#define RC_SERIAL_PORT               "/dev/ttyS3"
#define BOARD_SUPPORTS_RC_SERIAL_PORT_OUTPUT

/* DMA Configuration */
#define BOARD_DMA_ALLOC_POOL_SIZE 2048

/* Console Buffer */
#define BOARD_ENABLE_CONSOLE_BUFFER
#define BOARD_CONSOLE_BUFFER_SIZE (1024*3)

/* USB Configuration */
#define GPIO_USB_VBUS_VALID     (29 | GPIO_FUN(RP2040_GPIO_FUNC_SIO))  // GPIO29 - VBUS detection

/* PWM Output Configuration */
#define DIRECT_PWM_OUTPUT_CHANNELS      4
#define BOARD_HAS_PWM    DIRECT_PWM_OUTPUT_CHANNELS

/* SPI Bus Configuration for OpenFC2040 */
/* SPI1 is used for IMU and Barometer */
#define PX4_SPI_BUS_SENSORS  1
#define PX4_SPIDEV_LSM6DS3   PX4_MK_SPI_SEL(1, 0)  // CS on GPIO9
#define PX4_SPIDEV_DPS310    PX4_MK_SPI_SEL(1, 1)  // CS on GPIO12

/* I2C Configuration */
/* GPS I2C on GPIO6/7 */
#define PX4_I2C_BUS_GPS      0
/* Telemetry I2C on GPIO2/3 */
#define PX4_I2C_BUS_TELEM    1

/* MicroSD Card Configuration */
/* SD card on SPI0 (GPIO16-19) */
/* Pinout: DAT0(MISO)=GPIO16, CLK(SCK)=GPIO18, CMD(MOSI)=GPIO19, CD/DAT3(CS)=GPIO17 */
#define BOARD_HAS_SDCARD
#define PX4_SPI_BUS_SDCARD   0  // SPI0
#define PX4_SPIDEV_SDCARD    PX4_MK_SPI_SEL(0, 0)  // CS on GPIO17
#define GPIO_SDCARD_CS       (17 | GPIO_FUN(RP2040_GPIO_FUNC_SPI))
#define GPIO_SDCARD_SCK      (18 | GPIO_FUN(RP2040_GPIO_FUNC_SPI))
#define GPIO_SDCARD_MOSI     (19 | GPIO_FUN(RP2040_GPIO_FUNC_SPI))
#define GPIO_SDCARD_MISO     (16 | GPIO_FUN(RP2040_GPIO_FUNC_SPI))
/* Card detect pin (if connected) */
#define GPIO_SDCARD_CD       (17 | GPIO_FUN(RP2040_GPIO_FUNC_SIO))  // Optional, same as CS
#define BOARD_SDCARD_CD_ACTIVE_LOW  1

/* UART Configuration */
/* Telemetry UART on GPIO0/1 */
#define BOARD_SERIAL_TEL1    "/dev/ttyS0"
/* GPS UART on GPIO4/5 */
#define BOARD_SERIAL_GPS1    "/dev/ttyS1"

/* Buzzer Configuration */
/* QMB-09B-03 Passive Buzzer - Requires PWM drive */
#define GPIO_BUZZER           (25 | GPIO_FUN(RP2040_GPIO_FUNC_SIO))  // GPIO25
#define BUZZER_FREQUENCY_HZ   2700  // Optimal frequency: 2700±300Hz (2400-3000Hz)
#define BUZZER_FREQ_MIN_HZ    2400  // Minimum frequency
#define BUZZER_FREQ_MAX_HZ    3000  // Maximum frequency

/* External Flash Configuration */
/* W25Q128JVSIQTR - 16MB (128Mbit) on QSPI */
#define BOARD_HAS_EXTERNAL_FLASH
#define EXT_FLASH_SIZE_MB        16
#define EXT_FLASH_PAGE_SIZE      256
#define EXT_FLASH_SECTOR_SIZE    4096
#define EXT_FLASH_DEVICE_TYPE    0xEF4018  // Winbond W25Q128
#define FLASH_CS_PIN             QSPI_CS

/* Board USB connected detection */
#define BOARD_ADC_USB_CONNECTED (px4_arch_gpioread(GPIO_USB_VBUS_VALID))

/* Battery Voltage/Current Scaling */
/* WARNING: Verify actual voltage divider from schematic */
/* Current values are estimates and need verification */
#define BOARD_BATTERY_V_DIV      1.74f  // (2kΩ + 2.7kΩ) / 2.7kΩ - VERIFY THIS!
#define BOARD_BATTERY_A_PER_V    40.0f  // Current sensor scaling - VERIFY THIS!

/* Board-specific driver defaults */
#define BOARD_NUMBER_I2C_BUSES  2
#define BOARD_NUMBER_SPI_BUSES  2  // SPI0=SD card, SPI1=sensors

__BEGIN_DECLS

#ifndef __ASSEMBLY__

/****************************************************************************************************
 * Name: rp2040_spiinitialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the OpenFC2040 board.
 *
 ****************************************************************************************************/

extern void rp2040_spiinitialize(void);

/****************************************************************************************************
 * Name: rp2040_usbinitialize
 *
 * Description:
 *   Called to configure USB IO.
 *
 ****************************************************************************************************/

extern void rp2040_usbinitialize(void);

extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

#endif /* __ASSEMBLY__ */

__END_DECLS
