#ifndef ADC_H
#define ADC_H

void adc_inputs_init(void);
float adc_read_battery_voltage(void);
float adc_read_battery_current(void);
bool adc_read_vbus(void);
void adc_test(void);

#endif
