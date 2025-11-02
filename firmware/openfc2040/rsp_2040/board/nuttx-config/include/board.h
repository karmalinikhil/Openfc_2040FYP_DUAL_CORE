/****************************************************************************
 * boards/arm/rp2040/openfc2040/include/board.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.
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
#define BOARD_PLL_SYS_FREQ      (125 * MHZ)
#define BOARD_PLL_USB_FREQ      (48 * MHZ)

#define BOARD_REF_FREQ          (12 * MHZ)
#define BOARD_SYS_FREQ          (125 * MHZ)
#define BOARD_PERI_FREQ         (125 * MHZ)
#define BOARD_USB_FREQ          (48 * MHZ)
#define BOARD_ADC_FREQ          (48 * MHZ)
#define BOARD_RTC_FREQ          46875

#define BOARD_UART_BASEFREQ     BOARD_PERI_FREQ

#define BOARD_TICK_CLOCK        (1 * MHZ)

/* GPIO Configuration for OpenFC2040 ************************************************/

/*
 * UARTs:
 * UART0 (Telemetry):
 *   TX: GPIO0
 *   RX: GPIO1
 * UART1 (GPS):
 *   TX: GPIO4
 *   RX: GPIO5
 */
#define CONFIG_RP2040_UART0_GPIO	0	/* Telemetry */
#define CONFIG_RP2040_UART1_GPIO	4	/* GPS */

/*
 * I2C Configuration:
 * I2C0 (GPS):
 *   SDA: GPIO6
 *   SCL: GPIO7
 * I2C1 (Telemetry):
 *   SDA: GPIO2
 *   SCL: GPIO3
 */
#define CONFIG_RP2040_I2C0_GPIO		6	/* GPS I2C */
#define CONFIG_RP2040_I2C1_GPIO		2	/* Telemetry I2C */

/*
 * SPI1 (Sensors):
 * - SCK:  GPIO10
 * - MOSI: GPIO11
 * - MISO: GPIO8
 * 
 * Chip Selects:
 * - LSM6DS3 IMU: GPIO9
 * - DPS310 Baro: GPIO12
 */
#define GPIO_SPI1_SCLK	( 10 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
#define GPIO_SPI1_MOSI	( 11 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )
#define GPIO_SPI1_MISO	( 8 | GPIO_FUN(RP2040_GPIO_FUNC_SPI) )

/* PWM Outputs for ESCs */
#define GPIO_PWM0	( 20 | GPIO_FUN(RP2040_GPIO_FUNC_PWM) )
#define GPIO_PWM1	( 21 | GPIO_FUN(RP2040_GPIO_FUNC_PWM) )
#define GPIO_PWM2	( 22 | GPIO_FUN(RP2040_GPIO_FUNC_PWM) )
#define GPIO_PWM3	( 23 | GPIO_FUN(RP2040_GPIO_FUNC_PWM) )

#endif  /* __ARCH_BOARD_BOARD_H */
