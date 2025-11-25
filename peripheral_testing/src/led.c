#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "board_config.h"
#include "led.h"

void led_init(void) {
    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    
    // Common anode: HIGH = OFF
    led_off();
}

void led_set(int red, int green, int blue) {
    // Active LOW (common anode)
    gpio_put(LED_RED_PIN, !red);
    gpio_put(LED_GREEN_PIN, !green);
    gpio_put(LED_BLUE_PIN, !blue);
}

void led_off(void) {
    led_set(0, 0, 0);
}

void led_blink_all(int count) {
    for (int i = 0; i < count; i++) {
        led_set(1, 1, 1);
        sleep_ms(200);
        led_off();
        sleep_ms(200);
    }
}

void led_test(void) {
    printf("Testing RGB LED...\n");
    
    printf("  RED\n");
    led_set(1, 0, 0);
    sleep_ms(500);
    
    printf("  GREEN\n");
    led_set(0, 1, 0);
    sleep_ms(500);
    
    printf("  BLUE\n");
    led_set(0, 0, 1);
    sleep_ms(500);
    
    printf("  YELLOW (R+G)\n");
    led_set(1, 1, 0);
    sleep_ms(500);
    
    printf("  CYAN (G+B)\n");
    led_set(0, 1, 1);
    sleep_ms(500);
    
    printf("  MAGENTA (R+B)\n");
    led_set(1, 0, 1);
    sleep_ms(500);
    
    printf("  WHITE (R+G+B)\n");
    led_set(1, 1, 1);
    sleep_ms(500);
    
    led_off();
    printf("LED test complete.\n");
}
