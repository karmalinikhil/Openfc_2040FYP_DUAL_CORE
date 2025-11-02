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
 * @file led.c
 *
 * OpenFC2040 RGB LED driver
 */

#include <px4_platform_common/px4_config.h>

#include <stdbool.h>

#include <arch/board/board.h>
#include <hardware/rp2040_sio.h>

#include "board_config.h"

/*
 * OpenFC2040 RGB LED Configuration:
 * - Red:   GPIO13
 * - Green: GPIO14
 * - Blue:  GPIO15
 */

__BEGIN_DECLS
extern void led_init(void);
extern void led_on(int led);
extern void led_off(int led);
extern void led_toggle(int led);
__END_DECLS

static uint32_t g_ledmap[] = {
	1 << 13,  // LED_RED (GPIO13)
	1 << 14,  // LED_GREEN (GPIO14)
	1 << 15,  // LED_BLUE (GPIO15)
};

__EXPORT void led_init(void)
{
	/* Configure LED GPIOs as outputs */
	modifyreg32(RP2040_SIO_GPIO_OE_CLR, 0, g_ledmap[0] | g_ledmap[1] | g_ledmap[2]);
	modifyreg32(RP2040_SIO_GPIO_OUT_CLR, 0, g_ledmap[0] | g_ledmap[1] | g_ledmap[2]);
	modifyreg32(RP2040_SIO_GPIO_OE_SET, 0, g_ledmap[0] | g_ledmap[1] | g_ledmap[2]);
}

static void rp2040_led_set(int led, bool state)
{
	switch (led) {
	case 0:  // LED_AMBER
	case 1:  // LED_RED
		if (state) {
			modifyreg32(RP2040_SIO_GPIO_OUT_SET, 0, g_ledmap[0]);
		} else {
			modifyreg32(RP2040_SIO_GPIO_OUT_CLR, 0, g_ledmap[0]);
		}
		break;

	case 2:  // LED_GREEN
		if (state) {
			modifyreg32(RP2040_SIO_GPIO_OUT_SET, 0, g_ledmap[1]);
		} else {
			modifyreg32(RP2040_SIO_GPIO_OUT_CLR, 0, g_ledmap[1]);
		}
		break;

	case 3:  // LED_BLUE
		if (state) {
			modifyreg32(RP2040_SIO_GPIO_OUT_SET, 0, g_ledmap[2]);
		} else {
			modifyreg32(RP2040_SIO_GPIO_OUT_CLR, 0, g_ledmap[2]);
		}
		break;

	default:
		break;
	}
}

__EXPORT void led_on(int led)
{
	rp2040_led_set(led, true);
}

__EXPORT void led_off(int led)
{
	rp2040_led_set(led, false);
}

__EXPORT void led_toggle(int led)
{
	if (led >= 0 && led <= 2) {
		modifyreg32(RP2040_SIO_GPIO_OUT_XOR, 0, g_ledmap[led]);
	}
}
