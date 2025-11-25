/****************************************************************************
 *
 * Copyright (c) 2021 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 * used to endorse or promote products derived from this software
 * without specific prior written permission.
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
 * OpenFC2040 Internal Definitions
 */

#pragma once

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>

/* * ---------------------------------------------------------
 * LEDs (RGB)
 * ---------------------------------------------------------
 * Red: GPIO13, Green: GPIO14, Blue: GPIO15 
 */
#define GPIO_LED_RED    PX4_MAKE_GPIO_OUTPUT_CLEAR(13)
#define GPIO_LED_GREEN  PX4_MAKE_GPIO_OUTPUT_CLEAR(14)
#define GPIO_LED_BLUE   PX4_MAKE_GPIO_OUTPUT_CLEAR(15)

/* Define standard PX4 LED colors to your hardware */
#define GPIO_LED_SAFETY GPIO_LED_RED
#define BOARD_OVERLOAD_LED     LED_RED
#define BOARD_ARMED_LED        LED_BLUE
#define BOARD_HAS_CONTROL_STATUS_LEDS 1

/* * ---------------------------------------------------------
 * ADC / Battery
 * ---------------------------------------------------------
 * BATT_V: GPIO27 (ADC1), BATT_A: GPIO28 (ADC2)
 */
// Channel Bitmask: ADC1 (bit 1) and ADC2 (bit 2)
#define ADC_CHANNELS ((1 << 1) | (1 << 2)) 

#define ADC_BATTERY_VOLTAGE_CHANNEL  1  // GPIO 27
#define ADC_BATTERY_CURRENT_CHANNEL  2  // GPIO 28

/* * ---------------------------------------------------------
 * BUZZER (Passive)
 * ---------------------------------------------------------
 * Huaneng QMB-09B-03 on GPIO 25
 */
#define GPIO_TONE_ALARM_0    PX4_MAKE_GPIO_OUTPUT_CLEAR(25)

/* * ---------------------------------------------------------
 * RC INPUT
 * ---------------------------------------------------------
 * RC Input: GPIO24
 * Note: We use PIO/PPM so we don't conflict with GPS UART 
 */
#define HRT_TIMER 1
#define HRT_TIMER_CHANNEL 1
#define HRT_PPM_CHANNEL 1    
// Mapped to GPIO 24
#define GPIO_PPM_IN        (24 | GPIO_FUN(RP2040_GPIO_FUNC_SIO)) 

// If using Serial RC (CRSF/ELRS/SBUS) via UART1 on these pins:
// Note: UART1 is also used by GPS (GPIO 4/5). You cannot use Hardware UART1 
// for both. RC on 24 implies using PIO for RC or swapping GPS pins.
// Assuming PPM/SBUS-PIO for now:
//#define RC_SERIAL_PORT             "/dev/ttyS1" 

/* * ---------------------------------------------------------
 * USB / VBUS
 * ---------------------------------------------------------
 * User did not specify VBUS sense pin. 
 * GPIO24 is RC, so we cannot use it for VBUS.
 * We force logic true to assume USB is managed by internal PHY.
 */
#define BOARD_ADC_USB_CONNECTED (true)

/* * ---------------------------------------------------------
 * SPI DEFINITIONS (Mapping for Drivers)
 * ---------------------------------------------------------
 */

/* SPI0: SD Card */
#define GPIO_SPI0_MISO   16
#define GPIO_SPI0_CS     17
#define GPIO_SPI0_SCK    18
#define GPIO_SPI0_MOSI   19

/* SPI1: Sensors (IMU & Baro) */
#define GPIO_SPI1_MISO   8
#define GPIO_SPI1_CS_IMU 9  // LSM6DS3
#define GPIO_SPI1_SCK    10
#define GPIO_SPI1_MOSI   11 
#define GPIO_SPI1_CS_BARO 12 // DPS310

/* * ---------------------------------------------------------
 * I2C EXTENSION (GPS Mag)
 * ---------------------------------------------------------
 * Mag SDA: GPIO6, Mag SCL: GPIO7 -> I2C1 on RP2040
 */
#define PX4_I2C_BUS_EXPANSION 1
#define GPIO_I2C1_SDA    6
#define GPIO_I2C1_SCL    7


/* * ---------------------------------------------------------
 * PWM / ESCs
 * ---------------------------------------------------------
 * ESC0-3: GPIO 20-23
 */
#define DIRECT_PWM_OUTPUT_CHANNELS       4
#define BOARD_HAS_PWM    DIRECT_PWM_OUTPUT_CHANNELS


/* System Config */
#define BOARD_DMA_ALLOC_POOL_SIZE 2048
#define BOARD_ENABLE_CONSOLE_BUFFER
#define BOARD_CONSOLE_BUFFER_SIZE (1024*3)


__BEGIN_DECLS

#ifndef __ASSEMBLY__

extern void rp2040_spiinitialize(void);
extern void rp2040_usbinitialize(void);
extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

#endif /* __ASSEMBLY__ */

__END_DECLS