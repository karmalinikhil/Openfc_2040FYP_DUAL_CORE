#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "board_config.h"
#include "adc.h"

// ADC channel mapping: GPIO26=ADC0, GPIO27=ADC1, GPIO28=ADC2, GPIO29=ADC3
#define ADC_BATT_V_CH   1   // GPIO27
#define ADC_BATT_A_CH   2   // GPIO28
#define ADC_VBUS_CH     3   // GPIO29

// Voltage divider ratio (adjust based on actual hardware)
#define VDIV_RATIO      11.0f   // e.g., 10k + 1k divider

void adc_inputs_init(void) {
    adc_init();
    
    adc_gpio_init(BATT_V_PIN);
    adc_gpio_init(BATT_A_PIN);
    adc_gpio_init(VBUS_PIN);
    
    printf("ADC inputs initialized.\n");
}

float adc_read_battery_voltage(void) {
    adc_select_input(ADC_BATT_V_CH);
    uint16_t raw = adc_read();
    
    // Convert: 12-bit ADC, 3.3V reference, then apply divider ratio
    float voltage = (raw / 4095.0f) * 3.3f * VDIV_RATIO;
    return voltage;
}

float adc_read_battery_current(void) {
    adc_select_input(ADC_BATT_A_CH);
    uint16_t raw = adc_read();
    
    // Convert: depends on current sensor (assuming 40mV/A)
    float voltage = (raw / 4095.0f) * 3.3f;
    float current = voltage / 0.04f;  // 40mV/A
    return current;
}

bool adc_read_vbus(void) {
    adc_select_input(ADC_VBUS_CH);
    uint16_t raw = adc_read();
    
    // VBUS present if voltage > ~4V (after divider)
    float voltage = (raw / 4095.0f) * 3.3f;
    return voltage > 1.5f;  // Threshold depends on divider
}

void adc_test(void) {
    printf("Testing ADC inputs...\n");
    
    printf("Reading 5 samples...\n");
    for (int i = 0; i < 5; i++) {
        float batt_v = adc_read_battery_voltage();
        float batt_a = adc_read_battery_current();
        bool vbus = adc_read_vbus();
        
        printf("  Battery: %.2f V, %.2f A\n", batt_v, batt_a);
        printf("  USB VBUS: %s\n", vbus ? "CONNECTED" : "NOT CONNECTED");
        
        sleep_ms(500);
    }
    
    printf("ADC test complete.\n");
}
