# RP2350B (FMU) + STM32F103C8 (IO) Hardware & Firmware Guide

**Target hardware:**
- FMU: Raspberry Pi RP2350B (80-pin QFN, 48 GPIO)
- IO: STM32F103C8T6 (LQFP-48, Cortex-M3)

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Hardware Connections](#2-hardware-connections)
3. [STM32F103C8 IO Pin Mapping](#3-stm32f103c8-io-pin-mapping)
4. [How the Firmware Update Works](#4-how-the-firmware-update-works)
5. [First-Time Flash (Blank STM32)](#5-first-time-flash-blank-stm32)
6. [Building the IO Firmware](#6-building-the-io-firmware)
7. [Building the FMU Firmware](#7-building-the-fmu-firmware)
8. [FMU Board Config Required Defines](#8-fmu-board-config-required-defines)

---

## 1. Architecture Overview

```
┌─────────────────────────────────┐     UART 1.5 Mbit/s     ┌──────────────────────────────┐
│          RP2350B (FMU)          │ ◄──────────────────────► │     STM32F103C8 (IO)         │
│                                 │                           │                              │
│  PX4 autopilot stack            │   Normal operation:       │  px4iofirmware               │
│  Sensors / GPS / MAVLink        │   Register R/W protocol   │  8x PWM outputs              │
│  Commander / EKF / etc.         │                           │  PPM / SBUS RC input         │
│                                 │   Firmware update:        │  Safety button               │
│  Controls IO via px4io driver   │   Bootloader protocol     │  RC failsafe logic           │
│                                 │   @ 115200 baud           │                              │
│  GPIO → IO BOOT0                │ ──────────────────────►   │  BOOT0 pin (PA44/BOOT0)      │
│  GPIO → IO NRST                 │ ──────────────────────►   │  NRST pin (pin 7)            │
└─────────────────────────────────┘                           └──────────────────────────────┘
```

**Two separate firmware binaries** are needed:

| Binary | Target | Runs on |
|--------|--------|---------|
| `px4_io-v2_default.bin` | `px4/io-v2` | STM32F103C8 |
| `raspberrypi_pico2_default.px4` | your FMU board | RP2350B |

The IO `.bin` is **embedded inside the FMU firmware** in ROMFS at build time.
At boot, if the IO firmware version does not match, the RP2350B automatically reflashes the STM32 over UART.

---

## 2. Hardware Connections

### UART Link (FMU ↔ IO) — permanent connection

This is the main communication bus running at **1.5 Mbit/s** during normal operation,
and drops to **115200 baud** during firmware update.

| RP2350B GPIO | Direction | STM32F103C8 Pin | STM32 Signal |
|:------------:|:---------:|:---------------:|:------------:|
| GPIO 4 (UART1 TX) | → | PA3 (pin 17) | USART2 RX |
| GPIO 5 (UART1 RX) | ← | PA2 (pin 16) | USART2 TX |
| GND | — | GND | GND |

> **Note:** Cross the TX/RX as usual — FMU TX connects to IO RX and vice versa.
> UART1 is used in this example. You can remap to any UART on RP2350B but
> update `PX4IO_SERIAL_DEVICE` accordingly.

### IO Bootloader Control — for FMU-controlled firmware update

| RP2350B GPIO | Direction | STM32F103C8 Pin | Purpose |
|:------------:|:---------:|:---------------:|:-------:|
| GPIO 6 | → | BOOT0 (pin 44) | HIGH = enter bootloader, LOW = run flash |
| GPIO 7 | → | NRST (pin 7) | Active LOW reset (pulse to reset IO) |

Wire these via a **100Ω series resistor** to protect both devices.
Add a **10kΩ pull-down** on BOOT0 to GND so it defaults to run-from-flash on power-up.

### Crystal

The `px4/io-v2` NuttX config has been updated to use a **12 MHz HSE crystal** with the
STM32F103 internal PLL (×2) to produce a 24 MHz SYSCLK. All peripheral clock frequencies
are identical to the original 24 MHz direct-HSE design.

| STM32 Pin | Crystal Pin |
|-----------|------------|
| OSC_IN (pin 5) | Crystal pin 1 |
| OSC_OUT (pin 6) | Crystal pin 2 |

Use a **12 MHz, ±20 ppm** crystal with **10–22 pF** load capacitors to GND on both pins.

> 12 MHz crystals are cheaper, more common, and shared with many other MCUs
> (RP2040, RP2350, etc.), so you may be able to use the same part across your BOM.

#### Alternative: 12 MHz crystal → 72 MHz SYSCLK (maximum STM32F103 performance)

If you want the STM32 to run at its maximum 72 MHz speed, replace the PLL section
in `boards/px4/io-v2/nuttx-config/include/board.h` with:

```c
/* PLL: HSE (12MHz) × 6 = 72MHz SYSCLK */
#define STM32_CFGR_PLLSRC       RCC_CFGR_PLLSRC
#define STM32_CFGR_PLLXTPRE     0
#define STM32_CFGR_PLLMUL       RCC_CFGR_PLLMUL_CLKx6  /* × 6 */
#define STM32_PLL_FREQUENCY     (6 * STM32_BOARD_XTAL)  /* 72MHz */

#define STM32_SYSCLK_SW         RCC_CFGR_SW_PLL
#define STM32_SYSCLK_SWS        RCC_CFGR_SWS_PLL
#define STM32_SYSCLK_FREQUENCY  STM32_PLL_FREQUENCY     /* 72MHz */

/* AHB = 72MHz */
#define STM32_RCC_CFGR_HPRE     RCC_CFGR_HPRE_SYSCLK
#define STM32_HCLK_FREQUENCY    STM32_SYSCLK_FREQUENCY
#define STM32_BOARD_HCLK        STM32_HCLK_FREQUENCY

/* APB2 = 72MHz (max allowed) */
#define STM32_RCC_CFGR_PPRE2    RCC_CFGR_PPRE2_HCLK
#define STM32_PCLK2_FREQUENCY   STM32_HCLK_FREQUENCY

/* APB1 = 36MHz (APB1 max is 36MHz on STM32F103) */
#define STM32_RCC_CFGR_PPRE1    RCC_CFGR_PPRE1_HCLKd2
#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY / 2)  /* 36MHz */
```

> ⚠️ At 72 MHz you also need to set Flash wait states.
> Add to `defconfig`: `CONFIG_STM32_FLASH_PREFETCH=y` and ensure
> `CONFIG_STM32_FLASHCFG_TWO_WAIT_STATES=y` (2 wait states required above 48 MHz).
>
> Timer clocks on APB1 will be **72 MHz** (2 × PCLK1) because the APB1 prescaler ≠ 1.
> The 1.5 Mbit/s UART on USART2 (APB1 = 36 MHz) still works:
> BRR = 36,000,000 / 1,500,000 = **24.0** — exact, no error.

### Power

| Supply | Voltage | Notes |
|--------|---------|-------|
| VDD (STM32 pins 1, 32, 48) | 3.3V | Connect all three, 100nF cap each |
| VDDA (pin 9) | 3.3V | Analog supply, separate 100nF + 1µF cap |
| VSS (pins 8, 23, 35, 47) | GND | Connect all |
| VBAT (pin 1) | 3.3V | Can tie to VDD if no RTC battery needed |

---

## 3. STM32F103C8 IO Pin Mapping

### FMU Serial Link
| Pin | GPIO | Function |
|-----|------|----------|
| 16 | PA2 | USART2 TX → FMU RX |
| 17 | PA3 | USART2 RX ← FMU TX |

### PWM Outputs (8 channels)
| Pin | GPIO | PWM Channel |
|-----|------|-------------|
| 10 | PA0 | PWM1 (TIM2 CH1) |
| 11 | PA1 | PWM2 (TIM2 CH2) |
| 45 | PB8 | PWM3 (TIM4 CH3) |
| 46 | PB9 | PWM4 (TIM4 CH4) |
| 42 | PA6 | PWM5 (TIM3 CH1) |
| 43 | PA7 | PWM6 (TIM3 CH2) |
| 26 | PB0 | PWM7 (TIM3 CH3) |
| 27 | PB1 | PWM8 (TIM3 CH4) |

### RC Input / SBUS
| Pin | GPIO | Function |
|-----|------|----------|
| 29 | PA8 | PPM input (TIM1 CH1) |
| 33 | PB10 | SBUS output (USART3 TX) |
| 34 | PB11 | SBUS input  (USART3 RX) |
| 40 | PB4 | SBUS output enable (active LOW) |

### Safety & LEDs
| Pin | GPIO | Function |
|-----|------|----------|
| 41 | PB5 | Safety button input (floating, active HIGH) |
| 37 | PB13 | LED Safety (active LOW, open-drain) |
| 38 | PB14 | LED Blue / IMU heater (active LOW, open-drain) |
| 39 | PB15 | LED Amber (active LOW, open-drain) |
| 30 | PA11 | LED Green / power breathing (active HIGH) |

### Boot / Reset
| Pin | GPIO | Function |
|-----|------|----------|
| 44 | BOOT0 | HIGH on reset = enter ROM bootloader |
| 7  | NRST | Active LOW reset |

---

## 4. How the Firmware Update Works

After the first flash, the IO runs the **PX4 IO firmware's built-in bootloader** (not the STM32 ROM bootloader).
Firmware updates are fully automatic:

```
RP2350B boot sequence
        │
        ├─► px4io driver starts
        │
        ├─► Read IO firmware version over UART
        │
        ├─► Version matches ROMFS binary?
        │       YES → normal operation, skip to step 5
        │       NO  → begin update:
        │
        ├─► [1] Assert GPIO 6 (BOOT0) HIGH
        ├─► [2] Pulse GPIO 7 (NRST) LOW for 10ms, then HIGH
        │         IO reboots into its built-in bootloader
        │
        ├─► [3] UART drops to 115200 baud
        ├─► [4] Uploader sends SYNC, gets bootloader ACK
        ├─► [5] Uploader erases IO flash
        ├─► [6] Uploader sends px4_io-v2_default.bin in chunks
        ├─► [7] Uploader verifies CRC
        ├─► [8] Uploader sends PROTO_REBOOT
        │
        ├─► [9] Release GPIO 6 (BOOT0) LOW
        ├─► IO reboots and runs new firmware from flash
        │
        └─► UART returns to 1.5 Mbit/s, normal operation
```

The `.bin` file is read from ROMFS at `/etc/extras/px4_io-v2_default.bin`
which is embedded in the FMU firmware at build time.

---

## 5. First-Time Flash (Blank STM32)

A blank STM32F103C8 from the factory has no PX4 IO bootloader yet.
You need to flash it once using the STM32 ROM bootloader via USART1.

### Hardware setup for first flash

The STM32 ROM bootloader uses USART1 (not USART2):

| USB-Serial Adapter | STM32 Pin |
|-------------------|-----------|
| TX | PA9 (pin 30, USART1 RX) |
| RX | PA10 (pin 31, USART1 TX) |
| GND | GND |

### Steps

1. Pull **BOOT0 (pin 44) HIGH** (connect to 3.3V)
2. Keep **BOOT1 / PB2 (pin 28) LOW** (connect to GND or leave floating with pull-down)
3. Power the board (or pulse NRST LOW→HIGH)
   - STM32 boots into ROM bootloader on USART1
4. Flash using `stm32flash`:
   ```bash
   stm32flash -w px4_io-v2_default.bin -v -g 0x08000000 /dev/ttyUSB0
   ```
5. Pull **BOOT0 LOW** (back to GND)
6. Reset → STM32 runs PX4 IO firmware

After this, all future updates happen automatically from the RP2350B over UART.

---

## 6. Building the IO Firmware

```bash
cd PX4-Autopilot

# Build the IO firmware for STM32F103C8
make px4_io-v2_default

# Output binary location:
# build/px4_io-v2_default/px4_io-v2_default.bin
```

Copy the output binary into your FMU board's extras folder so it gets bundled at build time:

```bash
cp build/px4_io-v2_default/px4_io-v2_default.bin \
   boards/raspberrypi/pico2/extras/px4_io-v2_default.bin
```

> The extras folder must exist: `mkdir -p boards/raspberrypi/pico2/extras/`

---

## 7. Building the FMU Firmware

```bash
cd PX4-Autopilot

# Build both IO and FMU in one step (IO is built as an ExternalProject)
make raspberrypi_pico2_default

# Output files:
# build/raspberrypi_pico2_default/raspberrypi_pico2_default.bin  ← flash to RP2350B
# The IO binary is embedded inside the FMU firmware in ROMFS
```

To flash the FMU to the RP2350B via USB:

```bash
# Convert to UF2
picotool uf2 convert build/raspberrypi_pico2_default/raspberrypi_pico2_default.bin \
    raspberrypi_pico2_default.uf2 --family rp2350-arm-s --offset 0x10000000

# Enter RP2350B bootloader:
# 1. Short external flash CS (QSPI_SS_N) to GND
# 2. Plug in USB while holding the short
# 3. Release → board appears as USB drive

# Drag and drop the .uf2 file onto the drive
```

---

## 8. FMU Board Config Required Defines

In your FMU board's `default.px4board`:

```kconfig
CONFIG_BOARD_IO="px4_io-v2_default"
CONFIG_DRIVERS_PX4IO=y
```

In `boards/raspberrypi/pico2/src/board_config.h`:

```c
/* PX4IO connection via UART1 */
#define BOARD_USES_PX4IO_VERSION    2
#define PX4IO_SERIAL_DEVICE         "/dev/ttyS1"    // RP2350B UART1
#define PX4IO_SERIAL_BITRATE        1500000          // 1.5 Mbit/s

/* IO bootloader control GPIOs */
#define GPIO_PX4IO_RESET            PX4_MAKE_GPIO_OUTPUT_CLEAR(7)  // → STM32 NRST
#define GPIO_PX4IO_BOOT             PX4_MAKE_GPIO_OUTPUT_CLEAR(6)  // → STM32 BOOT0
```

In `boards/raspberrypi/pico2/src/board_peripheral_reset.c` (implement the reset function):

```c
void board_peripheral_reset(int ms)
{
    // Assert BOOT0 HIGH so STM32 enters bootloader on next reset
    px4_arch_gpiowrite(GPIO_PX4IO_BOOT, 1);

    // Pulse NRST LOW to reset the STM32
    px4_arch_gpiowrite(GPIO_PX4IO_RESET, 0);
    up_mdelay(ms > 0 ? ms : 10);
    px4_arch_gpiowrite(GPIO_PX4IO_RESET, 1);

    // Wait for bootloader to start
    up_mdelay(100);
}
```

---

## Summary: What to Build and When

| Situation | Action |
|-----------|--------|
| First time, blank STM32 | Flash `px4_io-v2_default.bin` via USART1 + `stm32flash` with BOOT0=HIGH |
| Normal FMU firmware update | `make raspberrypi_pico2_default` → flash `.uf2` to RP2350B via USB |
| IO firmware version mismatch | RP2350B handles it automatically at boot — no user action needed |
| IO firmware only changed | Rebuild FMU firmware (it re-embeds the `.bin`), flash FMU, RP2350B auto-updates IO |

---

**References:**
- IO firmware source: `PX4-Autopilot/src/modules/px4iofirmware/`
- IO board config: `PX4-Autopilot/boards/px4/io-v2/`
- IO uploader: `PX4-Autopilot/src/drivers/px4io/px4io_uploader.cpp`
- FMU px4io driver: `PX4-Autopilot/src/drivers/px4io/`
