#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "board_config.h"
#include "pwm.h"

/* ---------------------------------------------------------------------------
 * PWM timing constants  --  system clock OVERCLOCKED to 200 MHz
 *
 * WHY 50 Hz:
 *   ESCs perform their power-on / calibration beep sequence ONLY when they
 *   detect a valid 50 Hz RC PWM signal.  At 400 Hz most ESCs stay silent
 *   and will not enter calibration mode.  50 Hz is the universal standard
 *   for ESC calibration and matches the PX4 flight firmware setting
 *   (PWM_MAIN_TIM0/1 = 50 in rc.board_defaults).
 *
 * Clock:    200 MHz  (set_sys_clock_khz(200000, true) in main)
 * Divider:  200.0   ->  200 MHz / 200 = 1 MHz PWM clock
 * Wrap:     20000   ->  1 MHz / 20000 = 50 Hz period (20 ms)
 *
 * Pulse width conversion:  1 us = 1 count  (at 1 MHz PWM clock)
 *   1000 us ->  1000 counts  (ESC min / disarmed)
 *   1500 us ->  1500 counts  (ESC midpoint)
 *   2000 us ->  2000 counts  (ESC max / full throttle)
 *
 * US_TO_LEVEL is the identity function -- no rounding error possible.
 * ---------------------------------------------------------------------------*/
#define PWM_CLKDIV      200.0f          /* 200 MHz / 200 = 1 MHz PWM clock   */
#define PWM_WRAP        20000u          /* counts per period -> 1 MHz / 20000 = 50 Hz */
#define US_TO_LEVEL(us) (us)            /* 1 count per us at 1 MHz PWM clock  */

/* Compile-time assertions -- catch range errors before they reach hardware */
_Static_assert(US_TO_LEVEL(PWM_MIN_US) < PWM_WRAP,
               "PWM_MIN_US exceeds wrap value");
_Static_assert(US_TO_LEVEL(PWM_MAX_US) < PWM_WRAP,
               "PWM_MAX_US exceeds wrap value");

void pwm_outputs_init(void) {
    /* -----------------------------------------------------------------------
     * Step 1 – assign GPIO function
     * All four pins must be set to PWM before slices are configured.
     * ----------------------------------------------------------------------- */
    gpio_set_function(ESC0_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC1_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC2_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC3_PIN, GPIO_FUNC_PWM);

    /* -----------------------------------------------------------------------
     * Step 2 – resolve slice numbers
     * GPIO20,21 → slice 2   (channels A and B)
     * GPIO22,23 → slice 3   (channels A and B)
     * ----------------------------------------------------------------------- */
    uint slice0 = pwm_gpio_to_slice_num(ESC0_PIN);   /* slice 2 */
    uint slice1 = pwm_gpio_to_slice_num(ESC1_PIN);   /* slice 2 */
    uint slice2 = pwm_gpio_to_slice_num(ESC2_PIN);   /* slice 3 */
    uint slice3 = pwm_gpio_to_slice_num(ESC3_PIN);   /* slice 3 */

    /* Sanity: ESC0/ESC1 must share a slice, ESC2/ESC3 must share a slice */
    if (slice0 != slice1 || slice2 != slice3 || slice0 == slice2) {
        printf("[PWM] ERROR: unexpected slice assignment! "
               "ESC0=%u ESC1=%u ESC2=%u ESC3=%u\n",
               slice0, slice1, slice2, slice3);
    } else {
        printf("[PWM] Slices: ESC0/1 → slice %u, ESC2/3 → slice %u\n",
               slice0, slice2);
    }

    /* -----------------------------------------------------------------------
     * Step 3 – build a SINGLE shared config and apply it IDENTICALLY to both
     *          slices.  Using one config object guarantees that divider,
     *          wrap, polarity, and phase-correct settings cannot diverge.
     * ----------------------------------------------------------------------- */
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, PWM_CLKDIV);
    pwm_config_set_wrap(&cfg, PWM_WRAP - 1u);

    /* Initialize with start=FALSE – do NOT enable the counter yet.
     * Starting slices individually would introduce a phase offset of up to
     * one full PWM period (2.5 ms) between slice 2 and slice 3, which ESC
     * controllers can interpret as unequal throttle at the same command. */
    pwm_init(slice0, &cfg, false);
    pwm_init(slice2, &cfg, false);   /* slice1 == slice0, slice3 == slice2 */

    /* -----------------------------------------------------------------------
     * Step 4 – preload the disarmed pulse width on every channel BEFORE
     *          enabling the counters so that no spurious pulses are emitted.
     * ----------------------------------------------------------------------- */
    pwm_set_gpio_level(ESC0_PIN, US_TO_LEVEL(PWM_MIN_US));
    pwm_set_gpio_level(ESC1_PIN, US_TO_LEVEL(PWM_MIN_US));
    pwm_set_gpio_level(ESC2_PIN, US_TO_LEVEL(PWM_MIN_US));
    pwm_set_gpio_level(ESC3_PIN, US_TO_LEVEL(PWM_MIN_US));

    /* -----------------------------------------------------------------------
     * Step 5 – ATOMIC start of both slices via bitmask.
     * pwm_set_mask_enabled() writes a single register on the RP2040 PWM
     * block that enables multiple slices in the same bus cycle, guaranteeing
     * they begin counting from 0 at the exact same moment.
     * ----------------------------------------------------------------------- */
    pwm_set_mask_enabled((1u << slice0) | (1u << slice2));

    printf("[PWM] Outputs initialized: 50 Hz @ 200 MHz sysclk (div=200, wrap=19999), "
           "1000 µs, slices %u+%u started atomically.\n",
           slice0, slice2);
}

/* ---------------------------------------------------------------------------
 * pwm_set_us – set a single channel pulse width in microseconds.
 *
 * Clamps the input to [PWM_MIN_US, PWM_MAX_US] so that no caller can
 * accidentally drive a channel outside the 1000–2000 µs ESC range.
 * --------------------------------------------------------------------------- */
void pwm_set_us(uint gpio, uint pulse_us) {
    if (pulse_us < PWM_MIN_US) pulse_us = PWM_MIN_US;
    if (pulse_us > PWM_MAX_US) pulse_us = PWM_MAX_US;
    pwm_set_gpio_level(gpio, US_TO_LEVEL(pulse_us));
}

/* ---------------------------------------------------------------------------
 * pwm_set_all_us – update all four ESC channels in rapid succession.
 *
 * Updating channels one-by-one inside a slow loop can mean that channel 0
 * gets its new level at the start of a PWM cycle while channel 3 gets it
 * near the end – a skew of up to one period.  This helper writes all four
 * level registers back-to-back to minimise inter-channel skew.
 * --------------------------------------------------------------------------- */
void pwm_set_all_us(uint esc0_us, uint esc1_us, uint esc2_us, uint esc3_us) {
    /* Clamp */
    if (esc0_us < PWM_MIN_US) esc0_us = PWM_MIN_US;
    if (esc0_us > PWM_MAX_US) esc0_us = PWM_MAX_US;
    if (esc1_us < PWM_MIN_US) esc1_us = PWM_MIN_US;
    if (esc1_us > PWM_MAX_US) esc1_us = PWM_MAX_US;
    if (esc2_us < PWM_MIN_US) esc2_us = PWM_MIN_US;
    if (esc2_us > PWM_MAX_US) esc2_us = PWM_MAX_US;
    if (esc3_us < PWM_MIN_US) esc3_us = PWM_MIN_US;
    if (esc3_us > PWM_MAX_US) esc3_us = PWM_MAX_US;

    /* Write all four level registers consecutively */
    pwm_set_gpio_level(ESC0_PIN, US_TO_LEVEL(esc0_us));
    pwm_set_gpio_level(ESC1_PIN, US_TO_LEVEL(esc1_us));
    pwm_set_gpio_level(ESC2_PIN, US_TO_LEVEL(esc2_us));
    pwm_set_gpio_level(ESC3_PIN, US_TO_LEVEL(esc3_us));
}

void pwm_test(void) {
    printf("Testing PWM outputs (ESC0-ESC3)...\n");
    printf("WARNING: Disconnect propellers / ESC signal wires before running!\n\n");

    /* --- Print the active configuration so it can be verified on a scope --- */
    printf("[PWM Config]\n");
    printf("  Frequency  : 50 Hz  (clkdiv=%.1f, wrap=%u) -- ESC calibration compatible\n",
           PWM_CLKDIV, PWM_WRAP - 1u);
    printf("  Min pulse  : %u µs  (level %u)\n",
           PWM_MIN_US, US_TO_LEVEL(PWM_MIN_US));
    printf("  Mid pulse  : %u µs  (level %u)\n",
           PWM_MID_US, US_TO_LEVEL(PWM_MID_US));
    printf("  Max pulse  : %u µs  (level %u)\n\n",
           PWM_MAX_US, US_TO_LEVEL(PWM_MAX_US));

    /* --- Step 1: all channels to minimum simultaneously --- */
    printf("Step 1: All ESCs → %u µs (armed-idle / disarmed)\n", PWM_MIN_US);
    pwm_set_all_us(PWM_MIN_US, PWM_MIN_US, PWM_MIN_US, PWM_MIN_US);
    sleep_ms(2000);

    /* --- Step 2: simultaneous sweep of all four channels --- */
    printf("Step 2: Simultaneous sweep %u → %u µs on all four channels...\n",
           PWM_MIN_US, PWM_MID_US);
    for (uint us = PWM_MIN_US; us <= PWM_MID_US; us += 25) {
        pwm_set_all_us(us, us, us, us);
        printf("  All ESCs: %u µs\n", us);
        sleep_ms(100);
    }

    /* --- Step 3: return all to minimum --- */
    printf("Step 3: All ESCs → %u µs\n\n", PWM_MIN_US);
    pwm_set_all_us(PWM_MIN_US, PWM_MIN_US, PWM_MIN_US, PWM_MIN_US);
    sleep_ms(1000);

    /* --- Step 4: individual channel identity test (one at a time) ---
     * This makes it possible to verify on a scope / logic analyser that
     * each channel corresponds to the expected motor position.            */
    printf("Step 4: Individual channel identity test (%u µs each for 1 s)...\n",
           PWM_MID_US);
    const uint esc_pins[4] = {ESC0_PIN, ESC1_PIN, ESC2_PIN, ESC3_PIN};
    for (int ch = 0; ch < 4; ch++) {
        printf("  ESC%d (GPIO%u) → %u µs  (others at %u µs)\n",
               ch, esc_pins[ch], PWM_MID_US, PWM_MIN_US);
        pwm_set_all_us(PWM_MIN_US, PWM_MIN_US, PWM_MIN_US, PWM_MIN_US);
        pwm_set_us(esc_pins[ch], PWM_MID_US);
        sleep_ms(1000);
    }

    /* --- Return all to minimum --- */
    pwm_set_all_us(PWM_MIN_US, PWM_MIN_US, PWM_MIN_US, PWM_MIN_US);
    printf("\nPWM test complete.  If any channel showed a different pulse width\n");
    printf("at identical µs commands, this is a hardware ESC calibration issue.\n");
}
