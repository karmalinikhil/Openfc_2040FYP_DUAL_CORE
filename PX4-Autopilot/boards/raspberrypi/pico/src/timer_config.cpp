/****************************************************************************
 *
 *   Copyright (C) 2021 PX4 Development Team. All rights reserved.
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

 #include <px4_arch/io_timer_hw_description.h>
 
 // RP2040 PWM Slices correspond to "Timers" here.
 // Slice 2 handles GPIO 20/21 (ESC 0, 1)
 // Slice 3 handles GPIO 22/23 (ESC 2, 3)
 // Note: Buzzer on GPIO25 is handled separately via tone_alarm driver, not here
 //
 // MOTOR SYNC NOTE:
 // Slices 2 and 3 are initialised by the NuttX io_timer layer individually.
 // This can introduce a phase offset of up to one PWM period between the
 // two slices at startup.  In practice this is a fixed offset (not a
 // drifting one) and does not affect steady-state pulse WIDTH accuracy.
 // Throttle inconsistency between channels is therefore almost always an
 // ESC hardware calibration issue rather than a firmware timing issue.
 // Verify with: pwm_out test -p 1100  (all channels should beep together)
 // If motors activate at different throttle levels, re-calibrate ESCs to
 // the range defined by PWM_MAIN_DIS/MIN/MAX in rc.board_defaults.
 
 constexpr io_timers_t io_timers[MAX_IO_TIMERS] = {
     initIOTimer(Timer::Timer2), // For ESC 0 & 1
     initIOTimer(Timer::Timer3), // For ESC 2 & 3
 };
 
 constexpr timer_io_channels_t timer_io_channels[MAX_TIMER_IO_CHANNELS] = {
     // ESC 0 -> GPIO 20
     initIOTimerChannel(io_timers, {Timer::Timer2, Timer::ChannelA}, {GPIO::Pin20}),
     
     // ESC 1 -> GPIO 21
     initIOTimerChannel(io_timers, {Timer::Timer2, Timer::ChannelB}, {GPIO::Pin21}),
     
     // ESC 2 -> GPIO 22
     initIOTimerChannel(io_timers, {Timer::Timer3, Timer::ChannelA}, {GPIO::Pin22}),
     
     // ESC 3 -> GPIO 23
     initIOTimerChannel(io_timers, {Timer::Timer3, Timer::ChannelB}, {GPIO::Pin23}),
 };
 
 constexpr io_timers_channel_mapping_t io_timers_channel_mapping = initIOTimerChannelMapping(io_timers,
         timer_io_channels);
