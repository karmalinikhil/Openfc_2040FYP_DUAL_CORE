#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "board_config.h"
#include "uart.h"

void uart_ports_init(void) {
    // Initialize Telemetry UART
    uart_init(TELEM_UART, TELEM_BAUD);
    gpio_set_function(TELEM_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(TELEM_RX_PIN, GPIO_FUNC_UART);
    
    // Initialize GPS UART
    uart_init(GPS_UART, GPS_BAUD);
    gpio_set_function(GPS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);
    
    printf("UART ports initialized (Telem: %d, GPS: %d).\n", TELEM_BAUD, GPS_BAUD);
}

void uart_test(void) {
    printf("Testing UART ports...\n");
    
    // Send test message on Telemetry
    printf("Sending on Telemetry UART (GPIO0/1)...\n");
    const char *telem_msg = "OpenFC2040 Telemetry Test\r\n";
    uart_puts(TELEM_UART, telem_msg);
    printf("  Sent: %s", telem_msg);
    
    // Send test message on GPS port
    printf("Sending on GPS UART (GPIO4/5)...\n");
    const char *gps_msg = "OpenFC2040 GPS Test\r\n";
    uart_puts(GPS_UART, gps_msg);
    printf("  Sent: %s", gps_msg);
    
    // Try to read from GPS (non-blocking)
    printf("Listening for GPS data (3 seconds)...\n");
    absolute_time_t timeout = make_timeout_time_ms(3000);
    char gps_buf[128];
    int gps_idx = 0;
    
    while (!time_reached(timeout) && gps_idx < 127) {
        if (uart_is_readable(GPS_UART)) {
            char c = uart_getc(GPS_UART);
            gps_buf[gps_idx++] = c;
            if (c == '\n') break;
        }
    }
    gps_buf[gps_idx] = '\0';
    
    if (gps_idx > 0) {
        printf("  GPS received: %s\n", gps_buf);
    } else {
        printf("  No GPS data received (GPS may not be connected).\n");
    }
    
    printf("UART test complete.\n");
}
