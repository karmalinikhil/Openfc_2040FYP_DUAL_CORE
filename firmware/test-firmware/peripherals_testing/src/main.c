/**
 * OpenFC2040 Peripheral Testing Firmware
 * Simple firmware to test: IMU, Barometer, GPS, LED, Buzzer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "openfc2040_board.h"

// ========================================
// SENSOR CALIBRATION CONSTANTS
// ========================================
// Adjust these values to fine-tune sensor readings

// Buzzer
//
// GPS UART configuration
#define GPS_UART uart0
#define GPS_BAUD_RATE 9600
#define GPS_TX_PIN UART_TX_PIN
#define GPS_RX_PIN UART_RX_PIN

// IMU (LSM6DS3TR-C) Calibration
#define IMU_ACCEL_SCALE_MG_PER_LSB    0.061f    // mg per LSB for ±2g range
#define IMU_GYRO_SCALE_MDPS_PER_LSB   8.75f     // mdps per LSB for ±250dps range

// IMU Offsets (adjust if sensors have bias)
#define IMU_ACCEL_OFFSET_X    0.0f     // mg
#define IMU_ACCEL_OFFSET_Y    0.0f     // mg
#define IMU_ACCEL_OFFSET_Z    0.0f     // mg
#define IMU_GYRO_OFFSET_X     0.0f     // mdps
#define IMU_GYRO_OFFSET_Y     0.0f     // mdps
#define IMU_GYRO_OFFSET_Z     0.0f     // mdps

// Barometer (DPS310) Calibration - Simplified Mode
#define BARO_TEMP_SCALE_FACTOR     7864320.0f    // kT for TMP_PRC = 8x
#define BARO_TEMP_BASELINE_OFFSET  25.0f        // °C - Adjust to match room temperature
#define BARO_TEMP_FINE_TUNE        0.0f         // °C - Fine adjustment

#define BARO_PRESS_SCALE_FACTOR    1040384.0f    // kP for PM_PRC = 64x
#define BARO_PRESS_BASELINE_OFFSET 101325.0f    // Pa - Standard atmospheric pressure
#define BARO_PRESS_FINE_TUNE       0.0f         // Pa - Fine adjustment (+ to increase, - to decrease)

// GPS Calibration
#define GPS_ALTITUDE_OFFSET        0.0f         // meters - Adjust for local elevation

// ========================================
// END CALIBRATION SECTION
// ========================================

// LED control variables
bool led_active_low = true;

void led_init(void) {
    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);

    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    // Turn off all LEDs initially (assuming active low)
    gpio_put(LED_RED_PIN, 1);
    gpio_put(LED_GREEN_PIN, 1);
    gpio_put(LED_BLUE_PIN, 1);
}

void led_set_color(bool red, bool green, bool blue) {
    // Assuming active low LEDs
    gpio_put(LED_RED_PIN, !red);
    gpio_put(LED_GREEN_PIN, !green);
    gpio_put(LED_BLUE_PIN, !blue);
}

// Buzzer control
void buzzer_init(void) {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);
}

void buzzer_set(bool on) {
    gpio_put(BUZZER_PIN, on ? 1 : 0);
}

void buzzer_beep(uint duration_ms) {
    buzzer_set(true);
    sleep_ms(duration_ms);
    buzzer_set(false);
}

// Software SPI for sensors
void spi_init_sensors(void) {
    gpio_init(SPI_SCK_PIN);
    gpio_init(SPI_MOSI_PIN);
    gpio_init(SPI_MISO_PIN);

    gpio_set_dir(SPI_SCK_PIN, GPIO_OUT);
    gpio_set_dir(SPI_MOSI_PIN, GPIO_OUT);
    gpio_set_dir(SPI_MISO_PIN, GPIO_IN);

    gpio_put(SPI_SCK_PIN, 0);
    gpio_put(SPI_MOSI_PIN, 0);

    // Initialize CS pins
    gpio_init(SPI_CS_IMU_PIN);
    gpio_init(SPI_CS_BARO_PIN);

    gpio_set_dir(SPI_CS_IMU_PIN, GPIO_OUT);
    gpio_set_dir(SPI_CS_BARO_PIN, GPIO_OUT);

    gpio_put(SPI_CS_IMU_PIN, 1);
    gpio_put(SPI_CS_BARO_PIN, 1);
}

uint8_t spi_transfer_byte(uint8_t data) {
    uint8_t result = 0;

    for (int i = 7; i >= 0; i--) {
        gpio_put(SPI_MOSI_PIN, (data >> i) & 1);
        sleep_us(1);

        gpio_put(SPI_SCK_PIN, 1);
        sleep_us(1);

        if (gpio_get(SPI_MISO_PIN)) {
            result |= (1 << i);
        }

        gpio_put(SPI_SCK_PIN, 0);
        sleep_us(1);
    }

    return result;
}

uint8_t spi_read_register(uint cs_pin, uint8_t reg) {
    gpio_put(cs_pin, 0);
    sleep_us(2);

    spi_transfer_byte(reg | 0x80);  // Read bit
    uint8_t data = spi_transfer_byte(0x00);

    sleep_us(2);
    gpio_put(cs_pin, 1);

    return data;
}

void spi_write_register(uint cs_pin, uint8_t reg, uint8_t data) {
    gpio_put(cs_pin, 0);
    sleep_us(2);

    spi_transfer_byte(reg);  // Write (no read bit)
    spi_transfer_byte(data);

    sleep_us(2);
    gpio_put(cs_pin, 1);
}

// DPS310 specific read (different timing)
uint8_t spi_read_register_dps310(uint cs_pin, uint8_t reg) {
    gpio_put(cs_pin, 0);
    sleep_us(10);  // Longer setup time for DPS310

    spi_transfer_byte(reg | 0x80);  // Read bit
    uint8_t data = spi_transfer_byte(0x00);

    sleep_us(10);
    gpio_put(cs_pin, 1);
    sleep_us(10);

    return data;
}

void spi_write_register_dps310(uint cs_pin, uint8_t reg, uint8_t data) {
    gpio_put(cs_pin, 0);
    sleep_us(10);

    spi_transfer_byte(reg & 0x7F);  // Clear read bit for write
    spi_transfer_byte(data);

    sleep_us(10);
    gpio_put(cs_pin, 1);
    sleep_us(10);
}

// GPS UART
void gps_init(void) {
    uart_init(GPS_UART, GPS_BAUD_RATE);
    gpio_set_function(GPS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);
}

void gps_read_data(char* buffer, size_t max_len) {
    size_t pos = 0;

    while (uart_is_readable(GPS_UART) && pos < max_len - 1) {
        char c = uart_getc(GPS_UART);
        if (c == '\n' || c == '\r') {
            break;
        }
        buffer[pos++] = c;
    }
    buffer[pos] = '\0';
}

// Initialize LSM6DS3TR-C IMU
void init_imu_sensor(void) {
    printf("Initializing LSM6DS3TR-C IMU...\n");

    // Configure accelerometer: ±2g, 104Hz ODR
    spi_write_register(SPI_CS_IMU_PIN, 0x10, 0x40);  // CTRL1_XL
    sleep_ms(10);

    // Configure gyroscope: ±250dps, 104Hz ODR
    spi_write_register(SPI_CS_IMU_PIN, 0x11, 0x40);  // CTRL2_G
    sleep_ms(10);

    // Enable accelerometer and gyroscope
    spi_write_register(SPI_CS_IMU_PIN, 0x12, 0x00);  // CTRL3_C
    sleep_ms(10);

    printf("IMU configured: ±2g accel, ±250dps gyro, 104Hz\n");
}

// DPS310 Calibration coefficients structure
typedef struct {
    int32_t c0, c1, c00, c10, c01, c11, c20, c21, c30;
    int32_t kp, kt;  // Scaling factors
} dps310_calib_t;

static dps310_calib_t dps310_calib;

// Read DPS310 calibration coefficients
void read_dps310_calibration(void) {
    printf("Reading DPS310 calibration coefficients...\n");

    // Read calibration coefficients (18 bytes from 0x10 to 0x21)
    uint8_t coef[18];
    for (int i = 0; i < 18; i++) {
        coef[i] = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x10 + i);
        sleep_us(100);
    }

    // Parse coefficients according to datasheet
    dps310_calib.c0 = (int32_t)(coef[0] << 4) | (coef[1] >> 4);
    if (dps310_calib.c0 & 0x800) dps310_calib.c0 |= 0xFFFFF000;  // Sign extend

    dps310_calib.c1 = (int32_t)((coef[1] & 0x0F) << 8) | coef[2];
    if (dps310_calib.c1 & 0x800) dps310_calib.c1 |= 0xFFFFF000;

    dps310_calib.c00 = (int32_t)(coef[3] << 12) | (coef[4] << 4) | (coef[5] >> 4);
    if (dps310_calib.c00 & 0x80000) dps310_calib.c00 |= 0xFFF00000;

    dps310_calib.c10 = (int32_t)((coef[5] & 0x0F) << 16) | (coef[6] << 8) | coef[7];
    if (dps310_calib.c10 & 0x80000) dps310_calib.c10 |= 0xFFF00000;

    dps310_calib.c01 = (int32_t)(coef[8] << 8) | coef[9];
    if (dps310_calib.c01 & 0x8000) dps310_calib.c01 |= 0xFFFF0000;

    dps310_calib.c11 = (int32_t)(coef[10] << 8) | coef[11];
    if (dps310_calib.c11 & 0x8000) dps310_calib.c11 |= 0xFFFF0000;

    dps310_calib.c20 = (int32_t)(coef[12] << 8) | coef[13];
    if (dps310_calib.c20 & 0x8000) dps310_calib.c20 |= 0xFFFF0000;

    dps310_calib.c21 = (int32_t)(coef[14] << 8) | coef[15];
    if (dps310_calib.c21 & 0x8000) dps310_calib.c21 |= 0xFFFF0000;

    dps310_calib.c30 = (int32_t)(coef[16] << 8) | coef[17];
    if (dps310_calib.c30 & 0x8000) dps310_calib.c30 |= 0xFFFF0000;

    // Set scaling factors for high precision (64x oversampling)
    dps310_calib.kp = 1040384;   // 2^20 for 64x pressure oversampling
    dps310_calib.kt = 7864320;    // 2^19 for 8x temperature oversampling

    printf("Calibration coefficients loaded:\n");
    printf("  c0=%ld, c1=%ld, c00=%ld, c10=%ld\n", dps310_calib.c0, dps310_calib.c1, dps310_calib.c00, dps310_calib.c10);
    printf("  c01=%ld, c11=%ld, c20=%ld, c21=%ld, c30=%ld\n",
           dps310_calib.c01, dps310_calib.c11, dps310_calib.c20, dps310_calib.c21, dps310_calib.c30);
}

// Initialize DPS310 Barometer
void init_baro_sensor(void) {
    printf("Initializing DPS310 Barometer (High Precision Mode)...\n");

    // Reset the DPS310
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x0C, 0x09);  // RESET register
    sleep_ms(50);  // Wait for reset to complete

    // Read calibration coefficients first
    read_dps310_calibration();

    // Configure pressure measurement: 64x oversampling (High Precision), 2 measurements/sec
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x06, 0x16);    // PRS_CFG: 0001 0110 = 64x OSR, 2 meas/sec
    sleep_ms(10);

    // Configure temperature measurement: 8x oversampling, external sensor
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x07, 0x83);  // TMP_CFG: 1000 0011 = ext sensor, 8x OSR
    sleep_ms(10);

    // Configure interrupt and FIFO (disable for now)
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x09, 0x04);  // CFG_REG: P_SHIFT_EN=1

    sleep_ms(10);

    // Start continuous pressure and temperature measurement
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x08, 0x07);  // MEAS_CFG: continuous P+T
    sleep_ms(100);  // Wait for first measurement

    printf("Barometer configured: 64x OSR (High Precision), 2 Hz, calibrated\n");
}

// IMU data reading
typedef struct {
    float accel_x, accel_y, accel_z;  // mg
    float gyro_x, gyro_y, gyro_z;     // mdps
} imu_data_t;

imu_data_t read_imu_data(void) {
    imu_data_t data = {0};

    // Read accelerometer (6 bytes from 0x28)
    int16_t accel_raw[3];
    for (int i = 0; i < 3; i++) {
        uint8_t low = spi_read_register(SPI_CS_IMU_PIN, 0x28 + (i * 2));
        uint8_t high = spi_read_register(SPI_CS_IMU_PIN, 0x29 + (i * 2));
        accel_raw[i] = (int16_t)((high << 8) | low);
    }

    // Read gyroscope (6 bytes from 0x22)
    int16_t gyro_raw[3];
    for (int i = 0; i < 3; i++) {
        uint8_t low = spi_read_register(SPI_CS_IMU_PIN, 0x22 + (i * 2));
        uint8_t high = spi_read_register(SPI_CS_IMU_PIN, 0x23 + (i * 2));
        gyro_raw[i] = (int16_t)((high << 8) | low);
    }

    // Apply scaling and offsets using calibration constants
    data.accel_x = (accel_raw[0] * IMU_ACCEL_SCALE_MG_PER_LSB) + IMU_ACCEL_OFFSET_X;
    data.accel_y = (accel_raw[1] * IMU_ACCEL_SCALE_MG_PER_LSB) + IMU_ACCEL_OFFSET_Y;
    data.accel_z = (accel_raw[2] * IMU_ACCEL_SCALE_MG_PER_LSB) + IMU_ACCEL_OFFSET_Z;

    data.gyro_x = (gyro_raw[0] * IMU_GYRO_SCALE_MDPS_PER_LSB) + IMU_GYRO_OFFSET_X;
    data.gyro_y = (gyro_raw[1] * IMU_GYRO_SCALE_MDPS_PER_LSB) + IMU_GYRO_OFFSET_Y;
    data.gyro_z = (gyro_raw[2] * IMU_GYRO_SCALE_MDPS_PER_LSB) + IMU_GYRO_OFFSET_Z;

    return data;
}

// Barometer data reading
typedef struct {
    float pressure;     // Pa
    float temperature;  // °C
} baro_data_t;

baro_data_t read_baro_data(void) {
    baro_data_t data = {0};

    // Check if data is ready
    uint8_t status = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x08);

    if (status & 0x20) {  // Temperature data ready
        // Read temperature (3 bytes from 0x03)
        uint8_t tmp_b2 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x03);
        uint8_t tmp_b1 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x04);
        uint8_t tmp_b0 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x05);

        int32_t tmp_raw = (int32_t)((tmp_b2 << 16) | (tmp_b1 << 8) | tmp_b0);
        if (tmp_raw & 0x800000) tmp_raw |= 0xFF000000;  // Sign extend

        // Apply temperature calibration using constants
        data.temperature = ((float)tmp_raw / BARO_TEMP_SCALE_FACTOR) + BARO_TEMP_BASELINE_OFFSET + BARO_TEMP_FINE_TUNE;
    }

    if (status & 0x10) {  // Pressure data ready
        // Read pressure (3 bytes from 0x00)
        uint8_t prs_b2 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x00);
        uint8_t prs_b1 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x01);
        uint8_t prs_b0 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x02);

        int32_t prs_raw = (int32_t)((prs_b2 << 16) | (prs_b1 << 8) | prs_b0);
        if (prs_raw & 0x800000) prs_raw |= 0xFF000000;  // Sign extend

        // Apply pressure calibration using constants
        data.pressure = ((float)prs_raw / BARO_PRESS_SCALE_FACTOR * 100000.0f) + BARO_PRESS_BASELINE_OFFSET + BARO_PRESS_FINE_TUNE;
    }

    return data;
}

// Command processing
void process_commands() {
    int c = getchar_timeout_us(1000);
    if (c == PICO_ERROR_TIMEOUT) return;

    static char cmd_buffer[32];
    static int cmd_pos = 0;

    if (c == '\r' || c == '\n') {
        if (cmd_pos > 0) {
            cmd_buffer[cmd_pos] = '\0';

            printf("\nCommand: %s\n", cmd_buffer);

            if (strcmp(cmd_buffer, "help") == 0) {
                printf("OpenFC2040 Peripheral Test Commands:\n");
                printf("  help     - Show this help\n");
                printf("  imu      - Read IMU data (accel + gyro)\n");
                printf("  baro     - Read barometer data\n");
                printf("  gps      - Read GPS data\n");
                printf("  red      - Turn on red LED\n");
                printf("  green    - Turn on green LED\n");
                printf("  blue     - Turn on blue LED\n");
                printf("  off      - Turn off all LEDs\n");
                printf("  beep     - Test buzzer\n");
                printf("  sensors  - Read sensor IDs\n");
                printf("  live     - Start live sensor data stream\n");
                printf("  debug    - Show raw sensor values for debugging\n");
                printf("  calib    - Show all calibration constants\n");
            }
            else if (strcmp(cmd_buffer, "imu") == 0) {
                imu_data_t imu = read_imu_data();
                printf("IMU Data:\n");
                printf("  Accel: X=%7.2f Y=%7.2f Z=%7.2f mg\n", imu.accel_x, imu.accel_y, imu.accel_z);
                printf("  Gyro:  X=%7.2f Y=%7.2f Z=%7.2f mdps\n", imu.gyro_x, imu.gyro_y, imu.gyro_z);
            }
            else if (strcmp(cmd_buffer, "baro") == 0) {
                baro_data_t baro = read_baro_data();
                printf("Barometer Data:\n");
                printf("  Pressure: %8.2f Pa (%6.2f hPa)\n", baro.pressure, baro.pressure / 100.0f);
                printf("  Temperature: %6.2f °C\n", baro.temperature);
            }
            else if (strcmp(cmd_buffer, "gps") == 0) {
                char gps_buffer[128];
                gps_read_data(gps_buffer, sizeof(gps_buffer));
                printf("GPS Data: %s\n", gps_buffer);
            }
            else if (strcmp(cmd_buffer, "red") == 0) {
                led_set_color(true, false, false);
                printf("Red LED ON\n");
            }
            else if (strcmp(cmd_buffer, "green") == 0) {
                led_set_color(false, true, false);
                printf("Green LED ON\n");
            }
            else if (strcmp(cmd_buffer, "blue") == 0) {
                led_set_color(false, false, true);
                printf("Blue LED ON\n");
            }
            else if (strcmp(cmd_buffer, "off") == 0) {
                led_set_color(false, false, false);
                printf("All LEDs OFF\n");
            }
            else if (strcmp(cmd_buffer, "beep") == 0) {
                printf("Buzzer test...\n");
                buzzer_beep(200);
                printf("Buzzer test complete\n");
            }
            else if (strcmp(cmd_buffer, "sensors") == 0) {
                uint8_t imu_id = spi_read_register(SPI_CS_IMU_PIN, 0x0F);
                uint8_t baro_id = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x0D);
                printf("Sensor IDs:\n");
                printf("  IMU (LSM6DS3TR-C): 0x%02X (expected: 0x6A) %s\n", imu_id, (imu_id == 0x6A) ? "✓" : "✗");
                printf("  Barometer (DPS310): 0x%02X (expected: 0x10) %s\n", baro_id, (baro_id == 0x10) ? "✓" : "✗");

                // Show status registers
                if (imu_id == 0x6A) {
                    uint8_t imu_status = spi_read_register(SPI_CS_IMU_PIN, 0x1E);
                    printf("  IMU Status: 0x%02X\n", imu_status);
                }

                if (baro_id == 0x10) {
                    uint8_t baro_status = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x08);
                    printf("  Baro Status: 0x%02X (PRS_RDY=%d, TMP_RDY=%d)\n",
                           baro_status, (baro_status & 0x10) ? 1 : 0, (baro_status & 0x20) ? 1 : 0);
                }
            }
            else if (strcmp(cmd_buffer, "live") == 0) {
                printf("Starting live data stream (press any key to stop)...\n");
                while (!uart_is_readable(uart_get_instance(0))) {
                    imu_data_t imu = read_imu_data();
                    baro_data_t baro = read_baro_data();

                    printf("IMU: AX=%6.1f AY=%6.1f AZ=%6.1f | GX=%6.1f GY=%6.1f GZ=%6.1f | ",
                           imu.accel_x, imu.accel_y, imu.accel_z,
                           imu.gyro_x, imu.gyro_y, imu.gyro_z);
                    printf("BARO: P=%7.1f T=%5.1f\n", baro.pressure/100.0f, baro.temperature);

                    sleep_ms(100);
                }
                getchar(); // Clear the input
                printf("Live stream stopped\n");
            }
            else if (strcmp(cmd_buffer, "debug") == 0) {
                printf("Raw Sensor Debug:\n");

                // Raw barometer values
                uint8_t baro_status = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x08);
                printf("  Baro Status: 0x%02X (PRS_RDY=%d, TMP_RDY=%d)\n",
                       baro_status, (baro_status & 0x10) ? 1 : 0, (baro_status & 0x20) ? 1 : 0);

                if (baro_status & 0x20) {  // Temperature ready
                    uint8_t tmp_b2 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x03);
                    uint8_t tmp_b1 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x04);
                    uint8_t tmp_b0 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x05);
                    int32_t tmp_raw = (int32_t)((tmp_b2 << 16) | (tmp_b1 << 8) | tmp_b0);
                    if (tmp_raw & 0x800000) tmp_raw |= 0xFF000000;
                    printf("  Raw Temp: 0x%02X%02X%02X = %ld\n", tmp_b2, tmp_b1, tmp_b0, tmp_raw);
                }

                if (baro_status & 0x10) {  // Pressure ready
                    uint8_t prs_b2 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x00);
                    uint8_t prs_b1 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x01);
                    uint8_t prs_b0 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x02);
                    int32_t prs_raw = (int32_t)((prs_b2 << 16) | (prs_b1 << 8) | prs_b0);
                    if (prs_raw & 0x800000) prs_raw |= 0xFF000000;
                    printf("  Raw Press: 0x%02X%02X%02X = %ld\n", prs_b2, prs_b1, prs_b0, prs_raw);
                }

                // Show calibration coefficients
                printf("  Calib c0=%ld, c1=%ld, c00=%ld\n", dps310_calib.c0, dps310_calib.c1, dps310_calib.c00);
            }
            else if (strcmp(cmd_buffer, "calib") == 0) {
                printf("========================================\n");
                printf("CURRENT CALIBRATION CONSTANTS\n");
                printf("========================================\n");
                printf("IMU (LSM6DS3TR-C):\n");
                printf("  Accel Scale: %.3f mg/LSB\n", IMU_ACCEL_SCALE_MG_PER_LSB);
                printf("  Gyro Scale:  %.2f mdps/LSB\n", IMU_GYRO_SCALE_MDPS_PER_LSB);
                printf("  Accel Offsets: X=%.1f Y=%.1f Z=%.1f mg\n",
                       IMU_ACCEL_OFFSET_X, IMU_ACCEL_OFFSET_Y, IMU_ACCEL_OFFSET_Z);
                printf("  Gyro Offsets:  X=%.1f Y=%.1f Z=%.1f mdps\n",
                       IMU_GYRO_OFFSET_X, IMU_GYRO_OFFSET_Y, IMU_GYRO_OFFSET_Z);
                printf("\n");
                printf("Barometer (DPS310):\n");
                printf("  Temp Scale Factor: %.0f\n", BARO_TEMP_SCALE_FACTOR);
                printf("  Temp Baseline: %.1f°C\n", BARO_TEMP_BASELINE_OFFSET);
                printf("  Temp Fine Tune: %.1f°C\n", BARO_TEMP_FINE_TUNE);
                printf("  Press Scale Factor: %.0f\n", BARO_PRESS_SCALE_FACTOR);
                printf("  Press Baseline: %.0f Pa\n", BARO_PRESS_BASELINE_OFFSET);
                printf("  Press Fine Tune: %.0f Pa\n", BARO_PRESS_FINE_TUNE);
                printf("\n");
                printf("GPS:\n");
                printf("  Altitude Offset: %.1f m\n", GPS_ALTITUDE_OFFSET);
                printf("========================================\n");
                printf("To adjust: Edit calibration constants at top of main.c\n");
            }
            else {
                printf("Unknown command. Type 'help' for available commands.\n");
            }

            cmd_pos = 0;
            printf("> ");
        }
    }
    else if (c >= 32 && c < 127 && cmd_pos < sizeof(cmd_buffer) - 1) {
        cmd_buffer[cmd_pos++] = c;
        putchar(c);
    }
}

int main() {
    stdio_init_all();

    printf("\n=== OpenFC2040 Peripheral Test Firmware ===\n");
    printf("Testing: IMU, Barometer, GPS, LED, Buzzer\n");

    // Initialize peripherals
    led_init();
    buzzer_init();
    spi_init_sensors();
    gps_init();

    printf("All peripherals initialized\n");

    // Startup sequence
    printf("Startup test sequence...\n");

    // LED test
    led_set_color(true, false, false);
    sleep_ms(300);
    led_set_color(false, true, false);
    sleep_ms(300);
    led_set_color(false, false, true);
    sleep_ms(300);
    led_set_color(false, false, false);

    // Buzzer test
    buzzer_beep(1000);
    sleep_ms(100);
    buzzer_beep(1000);

    // Check sensors
    uint8_t imu_id = spi_read_register(SPI_CS_IMU_PIN, 0x0F);
    uint8_t baro_id = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x0D);

    printf("Sensor detection:\n");
    printf("  IMU ID: 0x%02X %s\n", imu_id, (imu_id == 0x6A) ? "(OK)" : "(FAIL)");
    printf("  Baro ID: 0x%02X %s\n", baro_id, (baro_id == 0x10) ? "(OK)" : "(FAIL)");

    // Initialize sensors if detected
    if (imu_id == 0x6A) {
        init_imu_sensor();
    } else {
        printf("Warning: IMU not detected - data will be invalid\n");
    }

    if (baro_id == 0x10) {
        init_baro_sensor();
    } else {
        printf("Warning: Barometer not detected - data will be invalid\n");
    }

    printf("\nReady! Type 'help' for commands or 'live' for real-time data\n");
    printf("> ");

    // Main loop
    while (true) {
        process_commands();
        sleep_ms(1);
    }

    return 0;
}
