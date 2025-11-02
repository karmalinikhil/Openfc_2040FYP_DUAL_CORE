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
 * @file usb.c
 *
 * OpenFC2040 USB configuration
 */

#include <px4_platform_common/px4_config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <debug.h>

#include <nuttx/usb/usbdev.h>
#include <nuttx/usb/usbdev_trace.h>

#include <nuttx/board.h>
#include <arch/board/board.h>

#include <rp2040.h>
#include <rp2040_gpio.h>
#include <hardware/rp2040_sio.h>

#include "board_config.h"

/************************************************************************************
 * Name: board_read_VBUS_state
 *
 * Description:
 *   Check the state of USB VBUS power on GPIO29 (ADC3)
 *
 ************************************************************************************/

__EXPORT bool board_read_VBUS_state(void)
{
	/* Read GPIO29 for VBUS detection */
	return (getreg32(RP2040_SIO_GPIO_IN) & (1 << 29)) != 0;
}

/************************************************************************************
 * Name: board_usb_init
 *
 * Description:
 *   Called to setup USB-related GPIO pins for the OpenFC2040 board.
 *
 ************************************************************************************/

__EXPORT void board_usb_init(void)
{
	/* Configure GPIO29 as input for VBUS detection */
	rp2040_gpioconfig(GPIO_USB_VBUS_VALID);
	
	/* USB D+ and D- are handled by the RP2040 USB peripheral automatically */
}
