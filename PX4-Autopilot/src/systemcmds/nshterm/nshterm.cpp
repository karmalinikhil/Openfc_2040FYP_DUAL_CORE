/****************************************************************************
 *
 *   Copyright (c) 2012, 2013 PX4 Development Team. All rights reserved.
 *   Author: Andrew Tridgell
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
 * @file nshterm.cpp
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/module.h>
#include <termios.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <nshlib/nshlib.h>
#include <fcntl.h>
#include <systemlib/err.h>
#include <drivers/drv_hrt.h>

static void print_usage()
{
	PRINT_MODULE_DESCRIPTION("Start an NSH shell on a given port.\n"
				 "\n"
				 "This was previously used to start a shell on the USB serial port.\n"
				 "Now there runs mavlink, and it is possible to use a shell over mavlink.\n"
				);

	PRINT_MODULE_USAGE_NAME_SIMPLE("nshterm", "command");
	PRINT_MODULE_USAGE_ARG("<file:dev>", "Device on which to start the shell (eg. /dev/ttyACM0)", false);
}

extern "C" __EXPORT int nshterm_main(int argc, char *argv[])
{
	if (argc < 2) {
		print_usage();
		return 1;
	}

	/* set up the serial port with output processing */
	int fd = open(argv[1], O_RDWR);

	/* Try to set baud rate */
	// Removed unused uart_config and termios_state variables after terminal config code removal

	/* Back up the original uart configuration to restore it after exit */
	// Terminal config code removed to restore working prompt state

	/* setup standard file descriptors */
	close(0);
	close(1);
	close(2);
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	nsh_consolemain(0, nullptr);

	close(fd);

	PX4_INFO("exiting");

	return 0;
}
