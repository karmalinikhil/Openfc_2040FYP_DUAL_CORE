#include <stdio.h>
#include "pico/stdlib.h"
#include "board_config.h"
#include "led.h"
#include "spi.h"
#include "imu.h"
#include "baro.h"
#include "pwm.h"
#include "adc.h"
#include "uart.h"

void print_menu(void) {
    printf("\n");
    printf("=== OpenFC2040 Peripheral Test ===\n");
    printf("1. LED Test (RGB cycle)\n");
    printf("2. IMU Test (LSM6DS3TR-C)\n");
    printf("3. Barometer Test (DPS310)\n");
    printf("4. PWM Test (ESC outputs)\n");
    printf("5. ADC Test (Battery/VBUS)\n");
    printf("6. UART Test (GPS/Telemetry)\n");
    printf("7. SD Card Test\n");
    printf("8. Run All Tests\n");
    printf("0. Exit\n");
    printf("Select: ");
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Wait for USB enumeration
    
    printf("\n\n");
    printf("*********************************\n");
    printf("*   OpenFC2040 Peripheral Test  *\n");
    printf("*********************************\n");
    
    // Initialize all peripherals
    led_init();
    spi_bus_init();
    pwm_outputs_init();
    adc_inputs_init();
    uart_ports_init();
    
    printf("All peripherals initialized.\n");
    
    while (true) {
        print_menu();
        
        int c = getchar_timeout_us(10000000);  // 10 second timeout
        if (c == PICO_ERROR_TIMEOUT) {
            printf("\nTimeout - running LED blink...\n");
            led_blink_all(3);
            continue;
        }
        
        printf("%c\n", c);
        
        switch (c) {
            case '1':
                printf("\n--- LED Test ---\n");
                led_test();
                break;
            case '2':
                printf("\n--- IMU Test ---\n");
                imu_test();
                break;
            case '3':
                printf("\n--- Barometer Test ---\n");
                baro_test();
                break;
            case '4':
                printf("\n--- PWM Test ---\n");
                pwm_test();
                break;
            case '5':
                printf("\n--- ADC Test ---\n");
                adc_test();
                break;
            case '6':
                printf("\n--- UART Test ---\n");
                uart_test();
                break;
            case '7':
                printf("\n--- SD Card Test ---\n");
                sd_card_test();
                break;
            case '8':
                printf("\n--- Running All Tests ---\n");
                led_test();
                imu_test();
                baro_test();
                pwm_test();
                adc_test();
                uart_test();
                sd_card_test();
                printf("--- All Tests Complete ---\n");
                break;
            case '0':
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid option\n");
        }
    }
    
    return 0;
}
