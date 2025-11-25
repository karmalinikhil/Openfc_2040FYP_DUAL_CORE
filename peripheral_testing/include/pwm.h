#ifndef PWM_H
#define PWM_H

void pwm_outputs_init(void);
void pwm_set_us(uint gpio, uint pulse_us);
void pwm_test(void);

#endif
