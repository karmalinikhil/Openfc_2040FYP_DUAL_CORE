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

/**
 * @file led.c
 *
 * OpenFC2040 LED driver
 * RGB LED on GPIO 13/14/15 (active low)
 * 
 * PX4 LED mapping (from drv_board_led.h):
 * LED_BLUE  = 0 -> GPIO15 (physically verified)
 * LED_RED   = 1 -> GPIO13 (physically verified)
 * LED_GREEN = 3 -> GPIO14 (physically verified)
 * 
 * Also provides led_pwm_servo functions for rgbled_pwm driver
 */

#include <px4_platform_common/px4_config.h>
#include <stdbool.h>
#include <nuttx/board.h>
#include <arch/board/board.h>
#include "board_config.h"

// LED state tracking
static bool _led_state[4] = {false, false, false, false};

// PWM values for rgbled_pwm driver (0-255)
static uint8_t _led_pwm[3] = {0, 0, 0};  // R, G, B
static bool _led_pwm_initialized = false;

/*
 * LED polarity: true = active low (write false to turn ON)
 *               false = active high (write true to turn ON)
 * Based on hardware testing - all LEDs are active LOW
 */
#define LED_RED_ACTIVE_LOW    true
#define LED_GREEN_ACTIVE_LOW  true
#define LED_BLUE_ACTIVE_LOW   true

static inline void led_write_red(bool on) {
    px4_arch_gpiowrite(GPIO_LED_RED, LED_RED_ACTIVE_LOW ? !on : on);
}
static inline void led_write_green(bool on) {
    px4_arch_gpiowrite(GPIO_LED_GREEN, LED_GREEN_ACTIVE_LOW ? !on : on);
}
static inline void led_write_blue(bool on) {
    px4_arch_gpiowrite(GPIO_LED_BLUE, LED_BLUE_ACTIVE_LOW ? !on : on);
}

__EXPORT void led_init(void)
{
    // Configure RGB LED GPIOs as outputs
    px4_arch_configgpio(GPIO_LED_RED);
    px4_arch_configgpio(GPIO_LED_GREEN);
    px4_arch_configgpio(GPIO_LED_BLUE);
    
    // Turn all LEDs off initially
    led_write_red(false);
    led_write_green(false);
    led_write_blue(false);
    
    _led_pwm_initialized = true;
}

__EXPORT void led_on(int led)
{
    switch (led) {
    case 0: // LED_BLUE
        led_write_blue(true);
        _led_state[0] = true;
        break;
    case 1: // LED_RED
        led_write_red(true);
        _led_state[1] = true;
        break;
    case 3: // LED_GREEN
        led_write_green(true);
        _led_state[3] = true;
        break;
    default:
        break;
    }
}

__EXPORT void led_off(int led)
{
    switch (led) {
    case 0: // LED_BLUE
        led_write_blue(false);
        _led_state[0] = false;
        break;
    case 1: // LED_RED
        led_write_red(false);
        _led_state[1] = false;
        break;
    case 3: // LED_GREEN
        led_write_green(false);
        _led_state[3] = false;
        break;
    default:
        break;
    }
}

__EXPORT void led_toggle(int led)
{
    switch (led) {
    case 0:
        _led_state[0] ? led_off(0) : led_on(0);
        break;
    case 1:
        _led_state[1] ? led_off(1) : led_on(1);
        break;
    case 3:
        _led_state[3] ? led_off(3) : led_on(3);
        break;
    default:
        break;
    }
}

/**
 * NuttX auto LED functions - DISABLED to prevent OS-level LED control
 * RGB LEDs are now exclusively controlled by PX4 rgbled_gpio driver
 * These functions are stubbed to satisfy NuttX linkage requirements
 */
void board_autoled_initialize_impl(void)
{
    //led_init(); //Full control to PX4 driver
}

void board_autoled_on_impl(int led)
{
   // led_on(led);
    /* Stubbed - LED control now handled by PX4 rgbled_gpio driver */
    (void)led;
}

void board_autoled_off_impl(int led)
{
    //led_off(led);
    /* Stubbed - LED control now handled by PX4 rgbled_gpio driver */
    (void)led;
}

/**
 * LED PWM servo functions for rgbled_pwm driver
 * These simulate PWM with simple on/off based on value threshold
 * Channel 0 = Red, Channel 1 = Green, Channel 2 = Blue
 */

__EXPORT int led_pwm_servo_init(void)
{
    if (!_led_pwm_initialized) {
        led_init();
    }
    
    // Turn all LEDs off
    _led_pwm[0] = 0;
    _led_pwm[1] = 0;
    _led_pwm[2] = 0;
    
    led_write_red(false);
    led_write_green(false);
    led_write_blue(false);
    
    return 0;
}

__EXPORT void led_pwm_servo_deinit(void)
{
    // Turn all LEDs off
    led_write_red(false);
    led_write_green(false);
    led_write_blue(false);
}

__EXPORT int led_pwm_servo_set(unsigned channel, uint8_t value)
{
    if (channel > 2) {
        return -1;
    }
    
    _led_pwm[channel] = value;
    
    // For GPIO-based LEDs, any value > 0 turns LED on
    bool on = (value > 0);
    
    switch (channel) {
    case 0: // Red
        led_write_red(on);
        break;
    case 1: // Green
        led_write_green(on);
        break;
    case 2: // Blue
        led_write_blue(on);
        break;
    }
    
    return 0;
}

__EXPORT unsigned led_pwm_servo_get(unsigned channel)
{
    if (channel > 2) {
        return 0;
    }
    return _led_pwm[channel];
}
