# OpenFC2040 RGB LED Fix Report

## Overview

The onboard RGB LED (GPIO13/14/15, active-low) was unstable when using `led_control` or commander indications. NuttX auto-LED logic and the PX4 `rgbled_gpio` driver both drove the same pins, causing flicker, color mixing, and ghosting.

## Symptoms

- `led_control on -c <color>` would briefly set the color, then revert or flicker.  
- Mixed colors (yellow, purple, cyan) appeared incorrectly.  
- LED pulsed even when forced to a steady color.

## Root Cause

1. `board_autoled_initialize()` and related NuttX callbacks toggled the RGB pins during boot/heartbeat.  
2. PX4 `rgbled_gpio` driver simultaneously wrote the same pins using the uORB `led_control` topic.

## Fix Summary

| Subsystem | Action |
| --- | --- |
| `boards/raspberrypi/pico/src/init.c` | Commented out `board_autoled_initialize()` and removed the boot-time `drv_led_start()` / `led_on()` calls. |
| `boards/raspberrypi/pico/src/led.c` | Stubbed `board_autoled_initialize_impl/on/off_impl` so NuttX keeps the symbols but no longer drives the pins. |
| `boards/raspberrypi/pico/src/board_config.h` | Added documentation noting that PX4 exclusively controls the RGB LED. |
| `boards/raspberrypi/pico/default.px4board` | Verified `CONFIG_DRIVERS_LIGHTS_RGBLED_GPIO=y` is enabled. |
| `boards/raspberrypi/pico/init/rc.board_defaults` | Verified `rgbled_gpio start` and initial `led_control on -c white` remain. |

Result: only PX4’s `rgbled_gpio` driver touches GPIO13/14/15. `led_control` behaves predictably and the normal flight status patterns stay steady.

## Behavior After Fix

1. Boot: NuttX no longer toggles the RGB LED.  
2. `rc.board_defaults`: starts `rgbled_gpio` and sets the initial color.  
3. Runtime: `led_control` publishes uORB messages, `rgbled_gpio` subscribes, and `set_rgb()` calls the board-level `led_on/led_off` helpers (active-low).

## Test Procedure

```
led_control on -c red
led_control on -c green
led_control on -c blue
led_control on -c white
led_control blink -c blue -s fast
led_control breathe -c green
led_control flash -c red -n 5
led_control off
```

Every command now holds its color or pattern without flicker or ghosting.

## Troubleshooting

1. `rgbled_gpio status` – driver running?  
2. `uorb top led_control` – messages arriving?  
3. If hardware changes, confirm the active-low defines in `boards/.../led.c`.

## File Log

| File | Change |
| --- | --- |
| `boards/raspberrypi/pico/src/init.c` | Disabled auto-LED init and removed direct LED writes. |
| `boards/raspberrypi/pico/src/led.c` | Stubbed `board_autoled_*` hooks. |
| `boards/raspberrypi/pico/src/board_config.h` | Added comment documenting PX4-only ownership. |
| `boards/raspberrypi/pico/default.px4board` | Verified `CONFIG_DRIVERS_LIGHTS_RGBLED_GPIO=y`. |
| `boards/raspberrypi/pico/init/rc.board_defaults` | Verified `rgbled_gpio start` + initial color command. |

## Status

The RGB LED is now under PX4 control only and stays in sync with flight status indicators.
