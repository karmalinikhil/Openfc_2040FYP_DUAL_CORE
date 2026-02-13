
## Summary

This report documents the successful implementation of RC input via PPM and motor control via PWM output on a custom RP2040-based flight controller running at 200MHz (overclocked from 125MHz). Multiple critical bugs in the PX4 firmware were identified and resolved, enabling full integration with Mission Planner and QGroundControl.

**Key Achievements:**
- ✅ RC input functional via PPM on GPIO24
- ✅ 4-channel PWM motor output on GPIO 20-23 at 50Hz
- ✅ Full calibration support in Mission Planner and QGroundControl
- ✅ Stable operation at 200MHz overclock with 48MHz peripheral clock

---

## Part 1: RC Input Implementation

### Overview

RC (Radio Control) input is critical for manual flight control. The OpenFC2040 uses **PPM (Pulse Position Modulation)** protocol to receive commands from the RC receiver on GPIO24.

### Initial Configuration

The RC input driver was already compiled into the firmware, but not initialized during board startup.

**Hardware Setup:**
- **GPIO24** connected to RC receiver PPM output
- **Protocol:** PPM (8 channels multiplexed on one wire)
- **Driver:** `rc_input` module (NuttX-based)

### Changes Made

#### 1. Board Initialization Script

Modified `/boards/raspberrypi/pico/init/rc.board_defaults`:

```bash
# RC input via PPM on GPIO24
set RC_INPUT_ARGS ""
param set-default RC_INPUT_PROTO 1   # 1 = PPM protocol
```

**What this does:**
- `RC_INPUT_PROTO 1` tells the `rc_input` driver to expect PPM signals
- Empty `RC_INPUT_ARGS` uses default GPIO assignment (GPIO24 for RP2040)
- The `rc_input` driver automatically starts via PX4's module system

#### 2. Hardware Configuration

The RP2040 NuttX configuration already had GPIO24 mapped for PPM input in the board hardware definitions. No additional hardware configuration was needed.

### How It Works

**Signal Flow:**
```
RC Transmitter → RC Receiver → PPM signal (GPIO24) → rc_input driver → uORB rc_channels topic → manual_control module → flight control
```

**Technical Details:**
- **PPM frame rate:** 50Hz (20ms period)
- **Channel pulse widths:** 1000-2000µs (standard RC range)
- **Channels decoded:** Up to 8 channels
- **Interrupt-driven:** Hardware timer captures rising/falling edges


---

## Part 2: PWM Motor Output Implementation

### Overview

Motor control requires generating precise PWM (Pulse Width Modulation) signals at 50Hz frequency with pulse widths between 1000-2000µs to drive ESCs (Electronic Speed Controllers).

**Hardware Configuration:**
- **GPIO 20-23** configured as PWM outputs
- **RP2040 PWM Slices 2-3** used (2 channels per slice)
- **Target frequency:** 50Hz (standard for ESCs)
- **Motor layout:** Quadcopter X configuration (4001)

---

### Problem 1: Configuration Issues (SOLVED)

#### Symptom
Motors did not respond to `actuator_test` commands or QGC actuator testing. However, QGroundControl showed correct configuration (4 motors assigned, 50Hz frequency, 1200-2000µs range).

#### Root Cause Analysis

**Initial Investigation:**
- `pwm_out status` showed `func: 0 0 0 0` (no motor functions assigned)
- PWM signals not present on oscilloscope
- Hardware was functional (verified with direct PWM generation)

**The Bug:**
Located in `/platforms/nuttx/src/px4/rpi/rpi_common/io_pins/pwm_servo.c`, function `up_pwm_servo_init()`:

```c
int up_pwm_servo_init(uint32_t channel_mask)
{
    // ... channel initialization code ...
    return OK;  // ← BUG: OK = 0
}
```

**Why This Was Critical:**
In `/src/drivers/pwm_out/PWMOut.cpp`:
```cpp
int ret = up_pwm_servo_init(_pwm_mask);
// ...
_pwm_mask = ret;  // ← _pwm_mask becomes 0!
```

With `_pwm_mask = 0`:
- Every PWM operation was silently skipped
- `up_pwm_servo_set()` never called → no PWM signals
- Motor function assignments appeared in QGC config but never reached hardware
- `actuator_test` commands flowed through the entire pipeline but stopped at the last step

**Why Configuration Was That Way:**
The RP2040 port copied the function skeleton from reference implementations but didn't match the STM32 return value convention. The STM32 version returns the bitmask of successfully initialized channels, not a status code.

#### The Fix

**Modified `pwm_servo.c`:**
```c
int up_pwm_servo_init(uint32_t channel_mask)
{
    uint32_t current = io_timer_get_mode_channels(IOTimerChanMode_PWMOut);
    int channels_init_mask = 0;  // ← Track successful inits

    // Free existing PWM channels
    for (unsigned channel = 0; current != 0 && channel < MAX_TIMER_IO_CHANNELS; channel++) {
        if (current & (1 << channel)) {
            io_timer_free_channel(channel);
            current &= ~(1 << channel);
        }
    }

    // Allocate new channels
    for (unsigned channel = 0; channel_mask != 0 && channel < MAX_TIMER_IO_CHANNELS; channel++) {
        if (channel_mask & (1 << channel)) {
            if (-EBUSY == io_timer_is_channel_free(channel)) {
                io_timer_free_channel(channel);
            }

            int ret = io_timer_channel_init(channel, IOTimerChanMode_PWMOut, NULL, NULL);
            channel_mask &= ~(1 << channel);

            if (ret == OK) {
                channels_init_mask |= 1 << channel;  // ← Set bit for success
            }
        }
    }

    return channels_init_mask;  // ← Return bitmask, not status
}
```

**Result:** Now `_pwm_mask` contains `0b00001111` (channels 0-3), enabling all PWM operations.

---


## Future Improvements

### Potential Enhancements

1. **Dshot Protocol Support**
   - Digital ESC protocol (no PWM jitter)
   - Bidirectional telemetry (RPM, voltage, current)
   - Requires DMA + TIM implementation

2. **OneShot125 Support**
   - Higher update rate (1-2kHz vs 50Hz)
   - Better throttle resolution
   - Already defined in codebase, needs testing

3. **Automatic ESC Calibration**
   - PX4 module to auto-calibrate on first boot
   - Store calibration in parameters
   - Reduce setup complexity

4. **PWM Input Capture**
   - Read conventional PWM RC receivers
   - Alternative to PPM (more common)
   - RP2040 PWM slices support input mode

5. **Telemetry on UART**
   - MAVLink telemetry on UART0 (instead of debug console)
   - Wireless link to ground station
   - Requires UART flow control implementation

---

## Conclusion

The successful implementation of RC input and PWM motor control on the OpenFC2040 demonstrates the viability of the RP2040 as a flight controller platform. 

Both Mission Planner and QGroundControl fully support the platform with complete calibration capabilities. The 200MHz overclock provides sufficient processing headroom while maintaining peripheral stability through careful clock architecture design.

---

## References

### Code Files Modified

1. `/platforms/nuttx/src/px4/rpi/rpi_common/io_pins/pwm_servo.c`
   - Fixed `up_pwm_servo_init()` return value

2. `/platforms/nuttx/src/px4/rpi/rpi_common/io_pins/io_timer.c`
   - Changed `TIM_SRC_CLOCK_FREQ` to `BOARD_SYS_FREQ`

3. `/platforms/nuttx/NuttX/nuttx/arch/arm/src/rp2040/rp2040_clock.c`
   - Changed CLK_PERI source to PLL_USB

4. `/boards/raspberrypi/pico/init/rc.board_defaults`
   - Added RC input configuration
   - Added PWM range parameters

5. `/boards/raspberrypi/pico/nuttx-config/include/board.h`
   - Set `BOARD_SYS_FREQ = 200MHz`
   - Set `BOARD_PERI_FREQ = 48MHz`


