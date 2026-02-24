#ifndef PWM_H
#define PWM_H

/* Initialize all four ESC PWM outputs.
 * Slices 2 (GPIO 20/21) and 3 (GPIO 22/23) are started atomically via
 * pwm_set_mask_enabled() to eliminate inter-slice phase offset. */
void pwm_outputs_init(void);

/* Set a single ESC channel to the given pulse width in microseconds.
 * Input is clamped to [PWM_MIN_US, PWM_MAX_US]. */
void pwm_set_us(uint gpio, uint pulse_us);

/* Update all four ESC channels in rapid succession to minimise skew
 * between channels at the same throttle command. */
void pwm_set_all_us(uint esc0_us, uint esc1_us, uint esc2_us, uint esc3_us);

/* Run the built-in PWM self-test (scope / logic-analyser recommended). */
void pwm_test(void);

#endif /* PWM_H */
