/**
 * OpenFC2040 Board Definitions
 * Hardware pin mappings and constants
 */

#ifndef OPENFC2040_BOARD_H
#define OPENFC2040_BOARD_H

// LED pins (RGB LED) - ACTUAL HARDWARE MAPPING
#define LED_RED_PIN     13
#define LED_GREEN_PIN   14
#define LED_BLUE_PIN    15

// SPI pins for sensors - ACTUAL HARDWARE MAPPING
#define SPI_SCK_PIN     10   // Shared SCK for both IMU and Barometer
#define SPI_MOSI_PIN    11   // SDA for IMU (MOSI)
#define SPI_MISO_PIN    8    // SDO for both IMU and Barometer (MISO)
#define SPI_CS_IMU_PIN  9    // LSM6DS3TR-C CS
#define SPI_CS_BARO_PIN 12   // DPS310 CS
#define SPI_CS_FLASH_PIN 7   // W25Q128 (if present)

// PWM pins for ESCs - ACTUAL HARDWARE MAPPING
#define ESC1_PIN        20   // ESC0 -> ESC1 for consistency
#define ESC2_PIN        21   // ESC1 -> ESC2
#define ESC3_PIN        22   // ESC2 -> ESC3
#define ESC4_PIN        23   // ESC3 -> ESC4

// Additional pins - ACTUAL HARDWARE MAPPING
#define RC_INPUT_PIN    24   // RC receiver input
#define BUZZER_PIN      25   // Buzzer output

// UART pins
#define UART_TX_PIN     0
#define UART_RX_PIN     1

// I2C pins (for external sensors)
#define I2C_SDA_PIN     20
#define I2C_SCL_PIN     21

// Safety button
#define SAFETY_BTN_PIN  6

// ADC pins for battery monitoring
#define BAT_VOLTAGE_PIN 26
#define BAT_CURRENT_PIN 27

#endif // OPENFC2040_BOARD_H
