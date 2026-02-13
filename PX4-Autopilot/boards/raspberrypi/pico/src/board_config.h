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

/*
LED (RGB)
GPIO13 = RED (with 162Ω series resistor)
GPIO14 = GREEN (with 60.4Ω series resistor)
GPIO15 = BLUE (with 100Ω series resistor)
Active LOW polarity (0V = LED ON, 3.3V = LED OFF)
*/
#define GPIO_LED_RED    PX4_MAKE_GPIO_OUTPUT_CLEAR(13)
#define GPIO_LED_GREEN  PX4_MAKE_GPIO_OUTPUT_CLEAR(14)
#define GPIO_LED_BLUE   PX4_MAKE_GPIO_OUTPUT_CLEAR(15)

/* Define standard PX4 LED colors to your hardware */
#define GPIO_LED_SAFETY GPIO_LED_RED
#define BOARD_OVERLOAD_LED     LED_RED
#define BOARD_ARMED_LED        LED_BLUE
#define BOARD_HAS_CONTROL_STATUS_LEDS 1

/* Disable NuttX auto-LED to prevent conflict with PX4 rgbled_gpio driver
 RGB LED control is exclusively handled by PX4 application layer */



/*
ADC / Battery
BATT_V: GPIO27 (ADC1), BATT_A: GPIO28 (ADC2)
*/
// Channel Bitmask: ADC1 (bit 1) and ADC2 (bit 2)
#define ADC_CHANNELS ((1 << 1) | (1 << 2)) 

#define ADC_BATTERY_VOLTAGE_CHANNEL  1  // GPIO 27
#define ADC_BATTERY_CURRENT_CHANNEL  2  // GPIO 28

/* 
Passive buzzer (resosnant frequency = 2700 Hz +- 300 Hz)
Huaneng QMB-09B-03 on GPIO 25
 */
#define GPIO_TONE_ALARM_1    PX4_MAKE_GPIO_OUTPUT_CLEAR(25)

/* 
RC Input: GPIO24
Note: We use PIO/PPM so we don't conflict with GPS UART 
*/
#define HRT_TIMER 1
#define HRT_TIMER_CHANNEL 1
#define HRT_PPM_CHANNEL 1    
// Mapped to GPIO 24
#define GPIO_PPM_IN        PX4_MAKE_GPIO_INPUT(24)

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
 * SPI DEFINITIONS (Pin numbers for reference)
 * ---------------------------------------------------------
 * Actual GPIO function config is in nuttx-config/include/board.h
 */

/* SPI0: SD Card - CLK:18, MOSI:19, MISO:16, CS:17 */
/* SPI1: Sensors - CLK:10, MOSI:11, MISO:8, IMU_CS:9, BARO_CS:12 */

/* * ---------------------------------------------------------
 * I2C EXTENSION (GPS Magnetometer)
 * ---------------------------------------------------------
 * External GPS/Magnetometer module connected via JST-GH connector
 * Mag SDA: GPIO6, Mag SCL: GPIO7 -> Hardware I2C1 on RP2040
 * PX4 Bus 1 = NuttX I2C1 port = Hardware I2C1 controller
 * 
 * Supported magnetometers (auto-detect in rc.board_sensors):
 *   - QMC5883L (I2C address 0x0D)
 *   - HMC5883L (I2C address 0x1E)
 *   - IST8310  (I2C address 0x0E)
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