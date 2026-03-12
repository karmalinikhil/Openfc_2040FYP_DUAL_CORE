/****************************************************************************
 *
 *   Copyright (c) 2021 PX4 Development Team. All rights reserved.
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

 #ifndef __ARCH_BOARD_BOARD_H
 #define __ARCH_BOARD_BOARD_H
 
 /************************************************************************************
  * Included Files
  ************************************************************************************/
 
 #include <nuttx/config.h>
 #ifndef __ASSEMBLY__
 # include <stdint.h>
 #endif
 
 /* Clocking *****************************************************************/
 
 #define MHZ                     1000000
 
 #define BOARD_XOSC_FREQ         (12 * MHZ)
#define BOARD_PLL_SYS_FREQ      (240 * MHZ)
 #define BOARD_PLL_USB_FREQ      (48 * MHZ)
 
 #define BOARD_REF_FREQ          (12 * MHZ)
#define BOARD_SYS_FREQ          (240 * MHZ)
 #define BOARD_PERI_FREQ         (48 * MHZ)    /* CLK_PERI sourced from PLL_USB for stable UART/SPI */
 #define BOARD_USB_FREQ          (48 * MHZ)
 #define BOARD_ADC_FREQ          (48 * MHZ)
 #define BOARD_RTC_FREQ          46875
 
 #define BOARD_UART_BASEFREQ     BOARD_PERI_FREQ
 
 #define BOARD_TICK_CLOCK        (1 * MHZ)
 
 /* Alternate function pin selections ************************************************/
 
 /*
  * UART0: NSH Console
  * TX: GPIO0
  * RX: GPIO1
  */
 #define CONFIG_RP2040_UART0_GPIO    0    
 
 /*
  * UART1: GPS
  * TX: GPIO4
  * RX: GPIO5
  */
 #define CONFIG_RP2040_UART1_GPIO    4    
 
 /*
  * I2C1: External Mag / Expansion
  * SDA: GPIO6
  * SCL: GPIO7
  */
 #define CONFIG_RP2040_I2C1_GPIO     6
 
 /* ---------------------------------------------------------
  * SPI0: SD Card (OpenFC2040)
  * ---------------------------------------------------------
  * CLK:  GPIO18
  * MISO: GPIO16 (DAT0)
  * MOSI: GPIO19 (CMD)
  * CS:   GPIO17 (Handled in board_config.h, not here)
  */
 #define GPIO_SPI0_SCLK  ( 18 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
 #define GPIO_SPI0_MISO  ( 16 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
 #define GPIO_SPI0_MOSI  ( 19 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
 
 /* ---------------------------------------------------------
  * SPI1: Internal Sensors (LSM6DS3 / DPS310)
  * ---------------------------------------------------------
  * CLK:  GPIO10
  * MISO: GPIO8  (SDO)
  * MOSI: GPIO11 (SDA)
  */
 #define GPIO_SPI1_SCLK    ( 10 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
 #define GPIO_SPI1_MISO    ( 8  | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
 #define GPIO_SPI1_MOSI    ( 11 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
 
 #endif  /* __ARCH_BOARD_BOARD_H */