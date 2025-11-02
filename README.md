# OpenFC2040 Flight Controller (RP2040 + PX4)

This repo contains scripts and documentation to build, flash, and debug PX4 firmware for the OpenFC2040 board. Start here, then dive into the linked docs for deeper explanations.

## Quick Start

### 1. Install prerequisites (Ubuntu/debian)

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi gdb-multiarch cmake ninja-build git python3 python3-pip openocd
sudo apt install picocom   # optional serial console
```

### 2. Configure the workspace (run once)

```bash
cd OpenFC2040_FYP
./scripts/setup_workspace.sh
```

### 3. Build PX4 firmware

```bash
cd firmware/openfc2040
./scripts/build.sh
```

### 4. Flash the board (PX4 build)

```bash
# hold BOOTSEL while connecting USB to enter the UF2 bootloader
./scripts/flash.sh
```

### 5. Access the console

```bash
# recommended: UART on GPIO0/GPIO1 via USB-to-Serial adapter
picocom -b 115200 /dev/ttyUSB0
```

## Debugging PX4
- SWD (Picoprobe/Debugprobe + OpenOCD + GDB): see `docs/DEBUG_GUIDE.md`
- Debug helpers and configs live in `firmware/openfc2040/rsp_2040/debug/`
- UART console steps: `firmware/openfc2040/rsp_2040/NEXT_STEPS_UART_CONSOLE.md`

## Peripheral Test Firmware (Pico SDK)
- Location: `firmware/test-firmware/peripherals_testing`
- Build and flash scripts live in that directory; follow its `README.md`
- Useful for validating LEDs, sensors, PWM, and console before running full PX4

## Deeper Background
- `docs/PROJECT_CONTEXT.md` - How the PX4 build, toolchain, and repo are structured
- `docs/PIN_CONNECTIONS.md` - Complete pin mapping
- `docs/DEBUG_GUIDE.md` - SWD wiring, OpenOCD, and GDB walkthrough
- `docs/GIT_QUICK_REFERENCE.md` - Day-to-day Git workflow
- `CONTRIBUTING.md` - Collaboration guidelines

## Notes
- Commands assume you run them from the repo root unless otherwise stated.
- Avoid editing generated folders such as `px4-autopilot` or `build` outputs directly.

## License and Credits
- PX4 Autopilot: BSD-3-Clause; NuttX: Apache-2.0
- Hardware design: OpenFC2040 by Vatsal Joshi (vxj9800)

Happy building!
