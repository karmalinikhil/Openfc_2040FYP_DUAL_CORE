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
 * @file init.c
 *
 * OpenFC2040 board-specific initialization
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/tasks.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <nuttx/spi/spi.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/sdio.h>
#include <nuttx/mmcsd.h>
#include <nuttx/analog/adc.h>
#include <nuttx/mm/gran.h>

#include <rp2040.h>
#include <rp2040_uart.h>
#include <rp2040_spi.h>
#include <rp2040_i2c.h>
#include <rp2040_gpio.h>
#include <hardware/rp2040_pads_bank0.h>

#include <arch/board/board.h>

#include "board_config.h"

#include <drivers/drv_hrt.h>
#include <drivers/drv_board_led.h>

#include <systemlib/px4_macros.h>

#include <px4_arch/io_timer.h>
#include <px4_platform_common/init.h>
#include <px4_platform/gpio.h>
#include <px4_platform/board_determine_hw_info.h>
#include <px4_platform/board_dma_alloc.h>

/************************************************************************************
 * Pre-Processor Definitions
 ************************************************************************************/

/* Configuration ************************************************************/

/*
 * Ideally we'd be able to get these from arm_internal.h,
 * but since we want to be able to disable the NuttX use
 * of leds for system indication at will and there is no
 * separate switch, we need to build independent of the
 * CONFIG_ARCH_LEDS configuration switch.
 */
__BEGIN_DECLS
extern void led_init(void);
extern void led_on(int led);
extern void led_off(int led);
extern void led_toggle(int led);
__END_DECLS

/************************************************************************************
 * Protected Functions
 ************************************************************************************/
/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: rp2040_boardearlyinitialize
 *
 * Description:
 *   This function is taken directly from nuttx
 *
 ************************************************************************************/

void rp2040_boardearlyinitialize(void)
{
	/* Configure UART pins for OpenFC2040 */
	/* Use the same pattern as Pico board for proper initialization */
	
	/* Telemetry UART on GPIO0/1 (UART0) - Console */
#if defined(CONFIG_RP2040_UART0) && CONFIG_RP2040_UART0_GPIO >= 0
	rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO, RP2040_GPIO_FUNC_UART);     /* TX */
	rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO + 1, RP2040_GPIO_FUNC_UART); /* RX */
# ifdef CONFIG_SERIAL_OFLOWCONTROL
	rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO + 2, RP2040_GPIO_FUNC_UART); /* CTS */
# endif /* CONFIG_SERIAL_OFLOWCONTROL */
# ifdef CONFIG_SERIAL_IFLOWCONTROL
	rp2040_gpio_set_function(CONFIG_RP2040_UART0_GPIO + 3, RP2040_GPIO_FUNC_UART); /* RTS */
# endif /* CONFIG_SERIAL_IFLOWCONTROL */
#endif

	/* GPS UART on GPIO4/5 (UART1) */
#if defined(CONFIG_RP2040_UART1) && CONFIG_RP2040_UART1_GPIO >= 0
	rp2040_gpio_set_function(CONFIG_RP2040_UART1_GPIO, RP2040_GPIO_FUNC_UART);     /* TX */
	rp2040_gpio_set_function(CONFIG_RP2040_UART1_GPIO + 1, RP2040_GPIO_FUNC_UART); /* RX */
# ifdef CONFIG_SERIAL_OFLOWCONTROL
	rp2040_gpio_set_function(CONFIG_RP2040_UART1_GPIO + 2, RP2040_GPIO_FUNC_UART); /* CTS */
# endif /* CONFIG_SERIAL_OFLOWCONTROL */
# ifdef CONFIG_SERIAL_IFLOWCONTROL
	rp2040_gpio_set_function(CONFIG_RP2040_UART1_GPIO + 3, RP2040_GPIO_FUNC_UART); /* RTS */
# endif /* CONFIG_SERIAL_IFLOWCONTROL */
#endif
}

/************************************************************************************
 * Name: rp2040_boardinitialize
 *
 * Description:
 *   All architectures must provide the following entry point. This entry point
 *   is called early in the initialization -- after all memory has been configured
 *   and mapped but before any devices have been initialized.
 *
 ************************************************************************************/

__EXPORT void
rp2040_boardinitialize(void)
{
	/* Configure LEDs */
	board_autoled_initialize();

	/* Configure ADC pins (disable IE and enable OD) */
	/* Battery voltage on GPIO27 */
	rp2040_gpioconfig(27 | GPIO_FUN(RP2040_GPIO_FUNC_NULL));
	clrbits_reg32(RP2040_PADS_BANK0_GPIO_IE, RP2040_PADS_BANK0_GPIO(27));
	setbits_reg32(RP2040_PADS_BANK0_GPIO_OD, RP2040_PADS_BANK0_GPIO(27));
	
	/* Battery current on GPIO28 */
	rp2040_gpioconfig(28 | GPIO_FUN(RP2040_GPIO_FUNC_NULL));
	clrbits_reg32(RP2040_PADS_BANK0_GPIO_IE, RP2040_PADS_BANK0_GPIO(28));
	setbits_reg32(RP2040_PADS_BANK0_GPIO_OD, RP2040_PADS_BANK0_GPIO(28));
	
	/* VBUS detection on GPIO29 */
	rp2040_gpioconfig(29 | GPIO_FUN(RP2040_GPIO_FUNC_NULL));
	clrbits_reg32(RP2040_PADS_BANK0_GPIO_IE, RP2040_PADS_BANK0_GPIO(29));
	setbits_reg32(RP2040_PADS_BANK0_GPIO_OD, RP2040_PADS_BANK0_GPIO(29));

	/* Configure I2C pins */
	
	/* GPS I2C on GPIO6/7 (I2C0) */
#if defined(CONFIG_RP2040_I2C0) && CONFIG_RP2040_I2C0_GPIO >= 0
	rp2040_gpio_set_function(6, RP2040_GPIO_FUNC_I2C);  /* SDA */
	rp2040_gpio_set_function(7, RP2040_GPIO_FUNC_I2C);  /* SCL */
	rp2040_gpio_set_pulls(6, true, false);  /* Pull up */
	rp2040_gpio_set_pulls(7, true, false);
#endif

	/* Telemetry I2C on GPIO2/3 (I2C1) */
#if defined(CONFIG_RP2040_I2C1) && CONFIG_RP2040_I2C1_GPIO >= 0
	rp2040_gpio_set_function(2, RP2040_GPIO_FUNC_I2C);  /* SDA */
	rp2040_gpio_set_function(3, RP2040_GPIO_FUNC_I2C);  /* SCL */
	rp2040_gpio_set_pulls(2, true, false);  /* Pull up */
	rp2040_gpio_set_pulls(3, true, false);
#endif

	/* Configure SPI interfaces */
	rp2040_spiinitialize();
	
	/* Configure RC input pin */
	rp2040_gpioconfig(GPIO_PPM_IN);
	
	/* Configure buzzer pin */
	rp2040_gpioconfig(GPIO_BUZZER);
}

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform application specific initialization.  This function is never
 *   called directly from application code, but only indirectly via the
 *   (non-standard) boardctl() interface using the command BOARDIOC_INIT.
 *
 * Input Parameters:
 *   arg - The boardctl() argument is passed to the board_app_initialize()
 *         implementation without modification.  The argument has no
 *         meaning to NuttX; the meaning of the argument is a contract
 *         between the board-specific initialization logic and the
 *         matching application logic.  The value could be such things as a
 *         mode enumeration value, a set of DIP switch switch settings, a
 *         pointer to configuration data read from a file or serial FLASH,
 *         or whatever you would like to do with it.  Every implementation
 *         should accept zero/NULL as a default configuration.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure to indicate the nature of the failure.
 *
 ****************************************************************************/

__EXPORT int board_app_initialize(uintptr_t arg)
{
	/* Need hrt running before using the ADC */
	px4_platform_init();

	/* Configure the DMA allocator */
#if defined(CONFIG_FAT_DMAMEMORY)
	if (board_dma_alloc_init() < 0) {
		syslog(LOG_ERR, "DMA alloc FAILED\n");
	}
#endif

	/* Configure CPU load estimation */
#ifdef CONFIG_SCHED_INSTRUMENTATION
	cpuload_initialize_once();
#endif

	/* Set up the serial DMA polling */
#ifdef SERIAL_HAVE_DMA
	static struct hrt_call serial_dma_call;
	hrt_call_every(&serial_dma_call, 1000, 1000, (hrt_callout)stm32_serial_dma_poll, NULL);
#endif

	/* Initial LED state for OpenFC2040 */
	drv_led_start();
	led_off(LED_RED);
	led_off(LED_GREEN);
	led_off(LED_BLUE);

	if (board_hardfault_init(2, true) != 0) {
		led_on(LED_RED);
	}

#ifdef CONFIG_MMCSD
	int ret = board_sdmmc_initialize();

	if (ret != OK) {
		led_on(LED_RED);
		return ret;
	}
#endif

	return OK;
}

/************************************************************************************
 * Name: rp2040_spiinitialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the OpenFC2040 board.
 *
 ************************************************************************************/

void rp2040_spiinitialize(void)
{
	/* Configure SPI1 pins for sensors */
	/* SCK on GPIO10 */
	rp2040_gpio_set_function(10, RP2040_GPIO_FUNC_SPI);
	/* MOSI on GPIO11 */
	rp2040_gpio_set_function(11, RP2040_GPIO_FUNC_SPI);
	/* MISO on GPIO8 */
	rp2040_gpio_set_function(8, RP2040_GPIO_FUNC_SPI);
	
	/* Configure chip select pins as outputs */
	/* LSM6DS3 CS on GPIO9 */
	rp2040_gpioconfig(9 | GPIO_FUN(RP2040_GPIO_FUNC_SIO) | GPIO_OUT | GPIO_SET);
	/* DPS310 CS on GPIO12 */
	rp2040_gpioconfig(12 | GPIO_FUN(RP2040_GPIO_FUNC_SIO) | GPIO_OUT | GPIO_SET);
}

/************************************************************************************
 * Name: rp2040_usbinitialize
 *
 * Description:
 *   Called to configure USB IO.
 *
 ************************************************************************************/

void rp2040_usbinitialize(void)
{
	/* USB is configured automatically by the RP2040 */
}

/************************************************************************************
 * Name: board_peripheral_reset
 *
 * Description:
 *   Reset peripherals on the board
 *
 ************************************************************************************/

__EXPORT void board_peripheral_reset(int ms)
{
	/* No peripheral reset needed for OpenFC2040 */
	UNUSED(ms);
}
