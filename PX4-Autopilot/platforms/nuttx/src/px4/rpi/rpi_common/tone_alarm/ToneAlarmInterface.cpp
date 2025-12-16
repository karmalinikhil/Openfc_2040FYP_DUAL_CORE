/****************************************************************************
 *
 * Copyright (c) 2025 OpenFC2040. All rights reserved.
 *
 * RP2040 Hardware-PWM Tone Alarm Interface
 * Targeted for: GPIO 25 (PWM Slice 4, Channel B)
 * Optimization: 12mA Drive Strength + Fast Slew Rate
 *
 ****************************************************************************/

#if defined(CONFIG_ARCH_CHIP_RP2040) || defined(CONFIG_ARCH_BOARD_OPENFC2040)

#include <drivers/drv_tone_alarm.h>
#include <px4_platform_common/px4_config.h>
#include <board_config.h>
#include <nuttx/arch.h>
#include <stdint.h>

/* --- RP2040 HARDWARE REGISTERS (Safe Definitions) --- */

#ifndef RP2040_IO_BANK0_BASE
#define RP2040_IO_BANK0_BASE    0x40014000u
#endif

#ifndef RP2040_PADS_BANK0_BASE
#define RP2040_PADS_BANK0_BASE  0x4001c000u
#endif

#ifndef RP2040_PWM_BASE
#define RP2040_PWM_BASE         0x40050000u
#endif

#ifndef RP2040_RESETS_BASE
#define RP2040_RESETS_BASE      0x4000c000u
#endif

// Resets
#define RESETS_RESET_OFFSET     0x0
#define RESETS_DONE_OFFSET      0x8
#define RESETS_PWM_BIT          (1 << 14)

// GPIO 25 Control
#define GPIO25_CTRL_OFFSET      0xCC
#define GPIO_FUNC_PWM           4

// GPIO 25 Pad Control (Electrical Properties)
#define PADS_GPIO25_OFFSET      0x68
#define PADS_SLEWFAST           (1 << 0)
#define PADS_DRIVE_12MA         (3 << 4) // 0=2mA, 1=4mA, 2=8mA, 3=12mA

// PWM Slice 4
#define PWM_SLICE4_OFFSET       0x50
#define PWM_CSR_OFFSET          0x00
#define PWM_DIV_OFFSET          0x04
#define PWM_CTR_OFFSET          0x08
#define PWM_CC_OFFSET           0x0C
#define PWM_TOP_OFFSET          0x10

#define REG32(addr)             (*(volatile uint32_t *)(addr))

namespace ToneAlarmInterface
{

void init()
{
#ifdef GPIO_TONE_ALARM_1
    // 1. UN-RESET PWM BLOCK
    uint32_t reset_addr = RP2040_RESETS_BASE + RESETS_RESET_OFFSET;
    uint32_t done_addr  = RP2040_RESETS_BASE + RESETS_DONE_OFFSET;
    
    // Clear Reset Bit
    uint32_t current_reset = REG32(reset_addr);
    REG32(reset_addr) = current_reset & ~RESETS_PWM_BIT;

    // Wait for Done Bit
    while (!(REG32(done_addr) & RESETS_PWM_BIT));

    // 2. BOOST DRIVE STRENGTH (The Volume Fix)
    uint32_t pads_addr = RP2040_PADS_BANK0_BASE + PADS_GPIO25_OFFSET;
    
    // Read - Modify - Write
    uint32_t pads_val = REG32(pads_addr);
    pads_val &= ~PADS_DRIVE_12MA; 
    pads_val &= ~PADS_SLEWFAST;
    pads_val |= PADS_DRIVE_12MA | PADS_SLEWFAST;
    REG32(pads_addr) = pads_val;

    // 3. Switch GPIO 25 to PWM Function
    uint32_t gpio_ctrl_addr = RP2040_IO_BANK0_BASE + GPIO25_CTRL_OFFSET;
    REG32(gpio_ctrl_addr) = GPIO_FUNC_PWM; 
#endif
}

hrt_abstime start_note(unsigned frequency)
{
    if (frequency == 0) {
        stop_note();
        return 0;
    }

#ifdef GPIO_TONE_ALARM_1
    if (frequency < 20) frequency = 20;
    if (frequency > 20000) frequency = 20000;

    // Clock Setup
    uint32_t sys_clk = 125000000; 
    uint32_t div_int = 100;       
    uint32_t pwm_clk = sys_clk / div_int; 

    // Calculate Period
    uint32_t top_val = pwm_clk / frequency;
    if (top_val > 65535) top_val = 65535; 

    // Duty Cycle 50%
    uint32_t cc_val = (top_val / 2) << 16; 

    // Write Registers
    uint32_t slice_base = RP2040_PWM_BASE + PWM_SLICE4_OFFSET;

    REG32(slice_base + PWM_CSR_OFFSET) = 0; // Disable
    REG32(slice_base + PWM_DIV_OFFSET) = (div_int << 4);
    REG32(slice_base + PWM_TOP_OFFSET) = top_val;
    
    uint32_t current_cc = REG32(slice_base + PWM_CC_OFFSET);
    REG32(slice_base + PWM_CC_OFFSET) = (current_cc & 0x0000FFFF) | cc_val;

    REG32(slice_base + PWM_CTR_OFFSET) = 0;
    REG32(slice_base + PWM_CSR_OFFSET) = 1; // Enable

    return hrt_absolute_time();
#else
    return 0;
#endif
}

void stop_note()
{
#ifdef GPIO_TONE_ALARM_1
    uint32_t slice_base = RP2040_PWM_BASE + PWM_SLICE4_OFFSET;
    REG32(slice_base + PWM_CSR_OFFSET) = 0;
#endif
}

} // namespace ToneAlarmInterface

#endif // CONFIG_ARCH_CHIP_RP2040