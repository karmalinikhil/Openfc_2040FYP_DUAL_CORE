#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

// OpenFC2040 Pin Definitions

// RGB LED (common anode, active LOW)
#define LED_RED_PIN     13
#define LED_GREEN_PIN   14
#define LED_BLUE_PIN    15

// IMU (LSM6DS3TR-C) on SPI1
#define IMU_SPI         spi1
#define IMU_CS_PIN      9
#define IMU_SCK_PIN     10
#define IMU_MOSI_PIN    11
#define IMU_MISO_PIN    8

// Barometer (DPS310) on SPI1 (shared bus with IMU)
#define BARO_SPI        spi1
#define BARO_CS_PIN     12
#define BARO_SCK_PIN    10   // Shared with IMU
#define BARO_MOSI_PIN   11   // Shared with IMU
#define BARO_MISO_PIN   8    // Shared with IMU

// SD Card on SPI0
#define SD_SPI          spi0
#define SD_CS_PIN       17
#define SD_SCK_PIN      18
#define SD_MOSI_PIN     19
#define SD_MISO_PIN     16

// ESC PWM Outputs
#define ESC0_PIN        20
#define ESC1_PIN        21
#define ESC2_PIN        22
#define ESC3_PIN        23

// GPS (UART1)
#define GPS_UART        uart1
#define GPS_TX_PIN      4
#define GPS_RX_PIN      5

// GPS I2C (I2C1)
#define GPS_I2C         i2c1
#define GPS_SDA_PIN     6
#define GPS_SCL_PIN     7

// Telemetry (UART0)
#define TELEM_UART      uart0
#define TELEM_TX_PIN    0
#define TELEM_RX_PIN    1

// Telemetry I2C (I2C0)
#define TELEM_I2C       i2c0
#define TELEM_SDA_PIN   2
#define TELEM_SCL_PIN   3

// ADC Inputs
#define BATT_V_PIN      27   // ADC1
#define BATT_A_PIN      28   // ADC2
#define VBUS_PIN        29   // ADC3 (USB VBUS detection)

// Other
#define RC_INPUT_PIN    24
#define BUZZER_PIN      25

// SPI Speeds
#define SPI_FREQ_SLOW   1000000   // 1 MHz for init
#define SPI_FREQ_FAST   10000000  // 10 MHz for normal operation

// UART Baud Rates
#define GPS_BAUD        9600
#define TELEM_BAUD      57600

// PWM Configuration
#define PWM_FREQ_HZ     400       // Standard ESC PWM frequency
#define PWM_MIN_US      1000      // Minimum pulse width
#define PWM_MAX_US      2000      // Maximum pulse width

#endif // BOARD_CONFIG_H
