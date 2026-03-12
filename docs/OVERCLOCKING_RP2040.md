# RP2040 Overclocking Changes (OpenFC2040)

## Background

The RP2040 is rated for 133 MHz at the default 1.10V core voltage. OpenFC2040 runs
at **240 MHz** for improved PX4 task throughput. Running above 133 MHz requires a
core voltage bump, or the chip will fail to boot reliably from a cold power cycle.

Symptom: board boots fine after USB flashing (soft reset, warm chip), but does **not**
boot after a power cycle (cold start from 0V).

---

## Required Changes When Overclocking Above 133 MHz

### 1. Raise VREG Core Voltage Before PLL Configuration

**File:** `platforms/nuttx/NuttX/nuttx/arch/arm/src/rp2040/rp2040_clock.c`  
**Function:** `clocks_init()`  
**Location:** After `rp2040_xosc_init()`, before the PLL reset/init block.

```c
#if BOARD_SYS_FREQ > (133 * MHZ)
  {
    /* VREG VSEL=0b1011 (1.25V), EN=1 */
    putreg32((0xb << 4) | 1, RP2040_VREG_AND_CHIP_RESET_BASE + 0x00);

    /* Wait for VREG output to stabilize (~100us) */
    volatile int vreg_delay;
    for (vreg_delay = 0; vreg_delay < 1000; vreg_delay++)
      {
        __asm__ volatile ("nop");
      }
  }
#endif
```

**Why:** The RP2040 on-chip VREG defaults to 1.10V. During a cold power-on reset the
PLL must lock the VCO at 1440 MHz (for 240 MHz clk_sys = 12 × 120 / 3 / 2). At 1.10V
this is outside spec and the PLL fails to stabilise, causing an immediate CPU crash or
HardFault before any user code runs. Raising to **1.25V** (VSEL = `0xb`) provides
adequate margin. The delay loop gives the regulator time to settle before the PLL is
touched.

#### VREG VSEL Table (RP2040 datasheet §2.10.3)

| VSEL | Voltage |
|------|---------|
| 0x6  | 1.10 V (default) |
| 0x7  | 1.15 V |
| 0x8  | 1.20 V |
| 0x9  | 1.25 V — use for 150–200 MHz |
| 0xa  | 1.30 V |
| 0xb  | 1.35 V — use for 200–250 MHz |

> **Note:** `0xb` maps to ~1.25–1.30V on silicon (datasheet lists 1.10V steps up to
> 1.30V; the SDK uses `0xb` = VREG_VOLTAGE_1_20 naming can be misleading — always
> match the register encoding to the table in §2.10.3, not SDK enum names).

---

### 2. Clock Frequency Definitions

**File:** `boards/raspberrypi/pico/nuttx-config/include/board.h`

```c
#define BOARD_PLL_SYS_FREQ      (240 * MHZ)   /* PLL VCO: 12 * 120 = 1440 / 3 / 2 */
#define BOARD_SYS_FREQ          (240 * MHZ)
#define BOARD_PERI_FREQ         (48 * MHZ)     /* CLK_PERI sourced from PLL_USB, NOT clk_sys */
```

`CLK_PERI` **must** be sourced from `PLL_USB` (48 MHz), not `clk_sys`. This decouples
UART, SPI, and I2C timing from the overclocked system clock — the correct approach
followed by the official Pico SDK `set_sys_clock_khz()`.

---

### 3. boot2 Flash Chip Setting

**File:** `boards/raspberrypi/pico/nuttx-config/Kconfig` (default) and defconfig.

```
CONFIG_RP2040_FLASH_CHIP="w25q080"
```

OpenFC2040 uses a **W25Q128JV** (16 MB), but boot2 uses W25Q080 byte-compatible
timings. This is acceptable — the W25Q128JV is pin/timing compatible for boot2
purposes. If you ever swap the flash chip for one with different fast-read timings,
update this value.

The boot2 second-stage bootloader is sourced from the Pico SDK at build time:
`$(PICO_SDK_PATH)/src/rp2_common/boot_stage2/boot2_w25q080.S`

---

## Summary Checklist

When changing `BOARD_SYS_FREQ` above 133 MHz:

- [ ] Add VREG voltage boost in `clocks_init()` before PLL init (see §1 above)
- [ ] Source `CLK_PERI` from `PLL_USB` (48 MHz), not from the overclocked `clk_sys`
- [ ] Confirm the `BOARD_LOOPSPERMSEC` value in defconfig matches the new CPU speed
      (used by NuttX busy-wait delays — wrong value causes incorrect `usleep` timing)
- [ ] Confirm boot2 flash chip matches the actual flash device on the board
- [ ] Test both USB-flash-then-boot **and** cold power cycle — they stress different
      paths and a missing VREG bump only shows up on cold start
