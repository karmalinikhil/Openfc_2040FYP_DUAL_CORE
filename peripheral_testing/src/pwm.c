#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "board_config.h"
#include "pwm.h"

#define PWM_WRAP    62500  // 125MHz / 62500 = 2kHz base, then /5 = 400Hz

void pwm_outputs_init(void) {
    // Configure ESC pins for PWM
    gpio_set_function(ESC0_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC1_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC2_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC3_PIN, GPIO_FUNC_PWM);
    
    // Get slice numbers
    uint slice0 = pwm_gpio_to_slice_num(ESC0_PIN);
    uint slice1 = pwm_gpio_to_slice_num(ESC1_PIN);
    uint slice2 = pwm_gpio_to_slice_num(ESC2_PIN);
    uint slice3 = pwm_gpio_to_slice_num(ESC3_PIN);
    
    // Configure for 400Hz (2.5ms period)
    // 125MHz / (5 * 62500) = 400Hz
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 5.0f);
    pwm_config_set_wrap(&config, PWM_WRAP - 1);
    
    pwm_init(slice0, &config, true);
    pwm_init(slice1, &config, true);
    pwm_init(slice2, &config, true);
    pwm_init(slice3, &config, true);
    
    // Set to minimum (1000us)
    pwm_set_us(ESC0_PIN, PWM_MIN_US);
    pwm_set_us(ESC1_PIN, PWM_MIN_US);
    pwm_set_us(ESC2_PIN, PWM_MIN_US);
    pwm_set_us(ESC3_PIN, PWM_MIN_US);
    
    printf("PWM outputs initialized (400Hz, 1000us).\n");
}

void pwm_set_us(uint gpio, uint pulse_us) {
    // Convert microseconds to PWM level
    // At 125MHz/5 = 25MHz, 1us = 25 counts
    uint level = pulse_us * 25;
    pwm_set_gpio_level(gpio, level);
}

void pwm_test(void) {
    printf("Testing PWM outputs (ESC0-ESC3)...\n");
    printf("WARNING: Disconnect ESCs before running!\n");
    
    printf("Setting all to 1000us (min)...\n");
    pwm_set_us(ESC0_PIN, 1000);
    pwm_set_us(ESC1_PIN, 1000);
    pwm_set_us(ESC2_PIN, 1000);
    pwm_set_us(ESC3_PIN, 1000);
    sleep_ms(1000);
    
    printf("Sweeping ESC0 1000-1500us...\n");
    for (int us = 1000; us <= 1500; us += 50) {
        pwm_set_us(ESC0_PIN, us);
        printf("  ESC0: %d us\n", us);
        sleep_ms(200);
    }
    
    printf("Sweeping ESC1 1000-1500us...\n");
    for (int us = 1000; us <= 1500; us += 50) {
        pwm_set_us(ESC1_PIN, us);
        printf("  ESC1: %d us\n", us);
        sleep_ms(200);
    }
    
    printf("Returning all to 1000us...\n");
    pwm_set_us(ESC0_PIN, 1000);
    pwm_set_us(ESC1_PIN, 1000);
    pwm_set_us(ESC2_PIN, 1000);
    pwm_set_us(ESC3_PIN, 1000);
    
    printf("PWM test complete.\n");
}
