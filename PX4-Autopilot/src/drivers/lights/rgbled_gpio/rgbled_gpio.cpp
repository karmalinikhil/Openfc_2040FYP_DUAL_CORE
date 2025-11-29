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
 * @file rgbled_gpio.cpp
 *
 * Driver for GPIO-based RGB LED (active low)
 * Listens to led_control uORB messages and controls GPIO pins
 */

#include <string.h>

#include <lib/led/led.h>
#include <px4_platform_common/getopt.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>
#include <drivers/drv_board_led.h>
#include <px4_arch/io_timer.h>

// Declare board LED functions as extern "C" so we can call them from C++
extern "C" {
	__EXPORT void led_on(int led);
	__EXPORT void led_off(int led);
}

class RGBLED_GPIO : public ModuleBase<RGBLED_GPIO>, public px4::ScheduledWorkItem
{
public:
	RGBLED_GPIO();
	virtual ~RGBLED_GPIO();

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	/** @see ModuleBase */
	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	int init();

	int status();

private:
	bool _r{false};
	bool _g{false};
	bool _b{false};

	volatile bool _running{false};
	volatile bool _should_run{true};

	LedController _led_controller;

	void Run() override;

	void set_rgb(bool r, bool g, bool b);
};

RGBLED_GPIO::RGBLED_GPIO() :
	ModuleBase(),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::lp_default)
{
}

RGBLED_GPIO::~RGBLED_GPIO()
{
	_should_run = false;
	int counter = 0;

	while (_running && ++counter < 10) {
		px4_usleep(100000);
	}

	// Turn off all LEDs on exit
	set_rgb(false, false, false);
}

int RGBLED_GPIO::init()
{
	// Initialize LEDs - they should already be initialized by board init
	// Turn all off initially
	set_rgb(false, false, false);

	_running = true;

	// Start work queue
	ScheduleNow();

	return OK;
}

int RGBLED_GPIO::status()
{
	PX4_INFO("state: R=%s G=%s B=%s",
		 _r ? "ON" : "OFF",
		 _g ? "ON" : "OFF",
		 _b ? "ON" : "OFF");
	return OK;
}

void RGBLED_GPIO::set_rgb(bool r, bool g, bool b)
{
	_r = r;
	_g = g;
	_b = b;

	// Use board LED functions (active low handled in led.c)
	if (r) {
		led_on(LED_RED);
	} else {
		led_off(LED_RED);
	}

	if (g) {
		led_on(LED_GREEN);
	} else {
		led_off(LED_GREEN);
	}

	if (b) {
		led_on(LED_BLUE);
	} else {
		led_off(LED_BLUE);
	}
}

void RGBLED_GPIO::Run()
{
	if (!_should_run) {
		_running = false;
		return;
	}

	LedControlData led_control_data;

	if (_led_controller.update(led_control_data) == 1) {
		uint8_t brightness = led_control_data.leds[0].brightness;

		// For GPIO-based LEDs, we treat any brightness > 0 as ON
		bool on = (brightness > 0);

		switch (led_control_data.leds[0].color) {
		case led_control_s::COLOR_RED:
			set_rgb(on, false, false);
			break;

		case led_control_s::COLOR_GREEN:
			set_rgb(false, on, false);
			break;

		case led_control_s::COLOR_BLUE:
			set_rgb(false, false, on);
			break;

		case led_control_s::COLOR_AMBER:
		case led_control_s::COLOR_YELLOW:
			set_rgb(on, on, false);
			break;

		case led_control_s::COLOR_PURPLE:
			set_rgb(on, false, on);
			break;

		case led_control_s::COLOR_CYAN:
			set_rgb(false, on, on);
			break;

		case led_control_s::COLOR_WHITE:
			set_rgb(on, on, on);
			break;

		default: // led_control_s::COLOR_OFF
			set_rgb(false, false, false);
			break;
		}
	}

	// Re-schedule
	ScheduleDelayed(_led_controller.maximum_update_interval());
}

int RGBLED_GPIO::task_spawn(int argc, char *argv[])
{
	RGBLED_GPIO *instance = new RGBLED_GPIO();

	if (instance == nullptr) {
		PX4_ERR("alloc failed");
		return PX4_ERROR;
	}

	_object.store(instance);
	_task_id = task_id_is_work_queue;

	if (instance->init() != OK) {
		PX4_ERR("init failed");
		delete instance;
		_object.store(nullptr);
		_task_id = -1;
		return PX4_ERROR;
	}

	return PX4_OK;
}

int RGBLED_GPIO::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

int RGBLED_GPIO::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
GPIO-based RGB LED driver. Controls RGB LED via GPIO pins.
Listens to led_control uORB topic.

)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("rgbled_gpio", "driver");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();

	return 0;
}

extern "C" __EXPORT int rgbled_gpio_main(int argc, char *argv[])
{
	return RGBLED_GPIO::main(argc, argv);
}
