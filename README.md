# OpenFC2040 Flight Controller (RP2040 + PX4)

Minimal, beginner-friendly docs to build, debug, and flash the OpenFC2040 firmware. For full background and architecture, see docs linked below.

— Firmware: PX4 Autopilot on NuttX (RP2040)

## Quick start

Prereqs on Linux (Ubuntu recommended):
- gcc-arm-none-eabi, cmake, ninja-build, git, python3, openocd, gdb-multiarch
- Optional: picocom or screen for serial

Steps:
1) One-time workspace setup
    - From the repo root:
      - Run scripts/setup_workspace.sh (clones/updates PX4 submodule, checks deps)

2) Build firmware (PX4)
    - cd firmware/openfc2040
    - ./scripts/build.sh

3) Flash to board
    - Put the board in BOOTSEL mode (hold BOOTSEL while plugging USB)
    - ./scripts/flash.sh

4) Console access (recommended via UART)
    - Use a USB-to-Serial adapter on GPIO0/1 (see docs/DEBUG_GUIDE.md for details)
If you prefer the Pico SDK test firmware for hardware bring-up, see Peripheral testing below.

## Debugging (short)
- SWD (Picoprobe/Debugprobe + OpenOCD + GDB): see docs/DEBUG_GUIDE.md
- On-target paths used by the scripts are repo-relative; no hardcoded absolute paths
- Helper files: firmware/openfc2040/rsp_2040/debug/

## Peripheral testing (Pico SDK)
Use the standalone test firmware to validate sensors, LEDs, PWM, and USB serial without PX4:
- cd firmware/test-firmware/peripherals_testing
- See its README.md for build/flash steps

## Helpful docs
- docs/PROJECT_CONTEXT.md — How the PX4 build actually works (CMake/Ninja pipeline)
- docs/PIN_CONNECTIONS.md — GPIO/pin mapping for the board
- docs/DEBUG_GUIDE.md — SWD wiring, OpenOCD, and GDB quickstart
- docs/GIT_QUICK_REFERENCE.md — Clean daily Git workflow
- CONTRIBUTING.md — Collaboration and commit guidelines

## Notes
- All instructions are location-agnostic. Run commands from the repo root unless stated.
- Avoid editing generated folders (e.g., px4-autopilot, build outputs) directly.

## License and credits
- PX4 Autopilot: BSD-3-Clause; NuttX: Apache-2.0
- Hardware design: OpenFC2040 by Vatsal Joshi (vxj9800)

Happy building!
