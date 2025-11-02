/**
 * OpenFC2040 Peripheral Testing Firmware
 * Simple firmware to test all peripherals: IMU, Barometer, GPS, LED, Buzzer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "openfc2040_board.h"

// GPS UART configuration
#define GPS_UART uart0
#define GPS_BAUD_RATE 9600
#define GPS_TX_PIN 0
#define GPS_RX_PIN 1

// LED control variables
bool led_active_low = true;

// Enhanced LED control functions
void led_init_pins(uint red_pin, uint green_pin, uint blue_pin) {
    gpio_init(red_pin);
    gpio_init(green_pin);
    gpio_init(blue_pin);
    
    gpio_set_dir(red_pin, GPIO_OUT);
    gpio_set_dir(green_pin, GPIO_OUT);
    gpio_set_dir(blue_pin, GPIO_OUT);
    
    // Turn off all LEDs initially
    gpio_put(red_pin, led_active_low ? 1 : 0);
    gpio_put(green_pin, led_active_low ? 1 : 0);
    gpio_put(blue_pin, led_active_low ? 1 : 0);
}

void led_init(void) {
    led_init_pins(LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);
}

void led_set_rgb_pins(uint red_pin, uint green_pin, uint blue_pin, bool red, bool green, bool blue) {
    gpio_put(red_pin, led_active_low ? !red : red);
    gpio_put(green_pin, led_active_low ? !green : green);
    gpio_put(blue_pin, led_active_low ? !blue : blue);
}

void led_set_rgb(bool red, bool green, bool blue) {
    led_set_rgb_pins(LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN, red, green, blue);
}

// LED testing functions
void test_led_pins(uint set_index) {
    if (set_index >= sizeof(led_pins_red)/sizeof(led_pins_red[0])) return;
    
    printf("Testing LED set %u: R=%u, G=%u, B=%u\n", 
           set_index, led_pins_red[set_index], led_pins_green[set_index], led_pins_blue[set_index]);
    
    led_init_pins(led_pins_red[set_index], led_pins_green[set_index], led_pins_blue[set_index]);
    
    // Test sequence
    for (int i = 0; i < 2; i++) {
        led_set_rgb_pins(led_pins_red[set_index], led_pins_green[set_index], led_pins_blue[set_index], true, false, false);
        sleep_ms(300);
        led_set_rgb_pins(led_pins_red[set_index], led_pins_green[set_index], led_pins_blue[set_index], false, true, false);
        sleep_ms(300);
        led_set_rgb_pins(led_pins_red[set_index], led_pins_green[set_index], led_pins_blue[set_index], false, false, true);
        sleep_ms(300);
        led_set_rgb_pins(led_pins_red[set_index], led_pins_green[set_index], led_pins_blue[set_index], false, false, false);
        sleep_ms(300);
    }
}

void scan_gpio_pins() {
    printf("Scanning GPIO pins for LED activity...\n");
    
    // Test pins 0-28 (RP2040 GPIO range)
    for (uint pin = 0; pin <= 28; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        
        printf("Testing GPIO %u: ", pin);
        
        // Test both polarities
        gpio_put(pin, 1);
        sleep_ms(200);
        gpio_put(pin, 0);
        sleep_ms(200);
        gpio_put(pin, 1);
        sleep_ms(200);
        gpio_put(pin, 0);
        
        printf("done\n");
        sleep_ms(100);
    }
}

// Buzzer control functions
void buzzer_init(void) {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);  // Off initially
}

void buzzer_beep(uint duration_ms) {
    gpio_put(BUZZER_PIN, 1);
    sleep_ms(duration_ms);
    gpio_put(BUZZER_PIN, 0);
}

void buzzer_startup_sequence(void) {
    // Startup beep pattern
    buzzer_beep(100);
    sleep_ms(100);
    buzzer_beep(100);
    sleep_ms(100);
    buzzer_beep(200);
}

// PWM initialization for ESC outputs
void pwm_init_escs(void) {
    // Initialize PWM for ESC outputs
    gpio_set_function(ESC1_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC2_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC3_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ESC4_PIN, GPIO_FUNC_PWM);
    
    uint slice_num1 = pwm_gpio_to_slice_num(ESC1_PIN);
    uint slice_num2 = pwm_gpio_to_slice_num(ESC2_PIN);
    uint slice_num3 = pwm_gpio_to_slice_num(ESC3_PIN);
    uint slice_num4 = pwm_gpio_to_slice_num(ESC4_PIN);
    
    // Set PWM frequency to 400Hz (2.5ms period)
    pwm_set_wrap(slice_num1, 62500);
    pwm_set_wrap(slice_num2, 62500);
    pwm_set_wrap(slice_num3, 62500);
    pwm_set_wrap(slice_num4, 62500);
    
    pwm_set_clkdiv(slice_num1, 4.0f);
    pwm_set_clkdiv(slice_num2, 4.0f);
    pwm_set_clkdiv(slice_num3, 4.0f);
    pwm_set_clkdiv(slice_num4, 4.0f);
    
    // Set initial PWM values (1ms pulse = motor off)
    pwm_set_gpio_level(ESC1_PIN, 6250);  // 1ms pulse
    pwm_set_gpio_level(ESC2_PIN, 6250);
    pwm_set_gpio_level(ESC3_PIN, 6250);
    pwm_set_gpio_level(ESC4_PIN, 6250);
    
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);
    pwm_set_enabled(slice_num4, true);
}

// Software SPI initialization for sensors (custom GPIO pins)
void spi_init_sensors(void) {
    // Initialize GPIO pins for software SPI
    gpio_init(SPI_SCK_PIN);
    gpio_init(SPI_MOSI_PIN);
    gpio_init(SPI_MISO_PIN);
    
    gpio_set_dir(SPI_SCK_PIN, GPIO_OUT);
    gpio_set_dir(SPI_MOSI_PIN, GPIO_OUT);
    gpio_set_dir(SPI_MISO_PIN, GPIO_IN);
    
    // Set initial states
    gpio_put(SPI_SCK_PIN, 0);   // Clock idle low (SPI Mode 0)
    gpio_put(SPI_MOSI_PIN, 0);
    
    // Initialize CS pins
    gpio_init(SPI_CS_IMU_PIN);
    gpio_init(SPI_CS_BARO_PIN);
    gpio_init(SPI_CS_FLASH_PIN);
    
    gpio_set_dir(SPI_CS_IMU_PIN, GPIO_OUT);
    gpio_set_dir(SPI_CS_BARO_PIN, GPIO_OUT);
    gpio_set_dir(SPI_CS_FLASH_PIN, GPIO_OUT);
    
    // Deselect all devices (CS high)
    gpio_put(SPI_CS_IMU_PIN, 1);
    gpio_put(SPI_CS_BARO_PIN, 1);
    gpio_put(SPI_CS_FLASH_PIN, 1);
    
    printf("Software SPI initialized on custom pins\n");
}

// Software SPI bit-banging functions
uint8_t soft_spi_transfer_byte(uint8_t data) {
    uint8_t result = 0;
    
    for (int i = 7; i >= 0; i--) {
        // Set MOSI bit (MSB first)
        gpio_put(SPI_MOSI_PIN, (data >> i) & 1);
        sleep_us(1);  // Setup time
        
        // Clock high
        gpio_put(SPI_SCK_PIN, 1);
        sleep_us(1);  // Clock high time
        
        // Read MISO bit
        if (gpio_get(SPI_MISO_PIN)) {
            result |= (1 << i);
        }
        
        // Clock low
        gpio_put(SPI_SCK_PIN, 0);
        sleep_us(1);  // Clock low time
    }
    
    return result;
}

// Basic sensor reading functions using software SPI
uint8_t spi_read_register(uint cs_pin, uint8_t reg) {
    uint8_t read_reg = reg | 0x80;  // Set read bit for most sensors
    uint8_t data;
    
    gpio_put(cs_pin, 0);
    sleep_us(2);  // CS setup time
    
    // Send register address
    soft_spi_transfer_byte(read_reg);
    
    // Read data
    data = soft_spi_transfer_byte(0x00);  // Send dummy byte to read
    
    sleep_us(2);  // CS hold time
    gpio_put(cs_pin, 1);
    
    return data;
}

// DPS310 read function with proper SPI protocol
uint8_t spi_read_register_dps310(uint cs_pin, uint8_t reg) {
    uint8_t data;
    uint8_t read_reg = reg | 0x80;  // Set MSB for read operation
    
    gpio_put(cs_pin, 0);
    sleep_us(10);  // Longer CS setup time for DPS310
    
    // Send register address with read bit
    soft_spi_transfer_byte(read_reg);
    
    // Read data
    data = soft_spi_transfer_byte(0x00);  // Send dummy byte to read
    
    sleep_us(10);  // Longer CS hold time
    gpio_put(cs_pin, 1);
    sleep_us(10);  // CS deselect time
    
    return data;
}

void spi_write_register(uint cs_pin, uint8_t reg, uint8_t data) {
    gpio_put(cs_pin, 0);
    sleep_us(2);  // CS setup time
    
    // Send register address
    soft_spi_transfer_byte(reg);
    
    // Send data
    soft_spi_transfer_byte(data);
    
    sleep_us(2);  // CS hold time
    gpio_put(cs_pin, 1);
}

// DPS310 write function with proper timing
void spi_write_register_dps310(uint cs_pin, uint8_t reg, uint8_t data) {
    gpio_put(cs_pin, 0);
    sleep_us(10);  // Longer CS setup time for DPS310
    
    // Send register address (no write bit needed, MSB=0 for write)
    soft_spi_transfer_byte(reg & 0x7F);
    
    // Send data
    soft_spi_transfer_byte(data);
    
    sleep_us(10);  // Longer CS hold time
    gpio_put(cs_pin, 1);
    sleep_us(10);  // CS deselect time
}

// DPS310 Barometer scaling factors and configuration
#define DPS310_OSR_16X_SCALE_FACTOR    253952      // Standard precision
#define DPS310_OSR_64X_SCALE_FACTOR    1040384     // High precision
#define DPS310_OSR_128X_SCALE_FACTOR   2088960     // Highest precision

// LSM6DS3TR-C IMU scaling factors
#define LSM6DS3_ACCEL_2G_SCALE         0.061f      // mg/LSB at ±2g
#define LSM6DS3_ACCEL_4G_SCALE         0.122f      // mg/LSB at ±4g
#define LSM6DS3_ACCEL_8G_SCALE         0.244f      // mg/LSB at ±8g
#define LSM6DS3_ACCEL_16G_SCALE        0.488f      // mg/LSB at ±16g

#define LSM6DS3_GYRO_125DPS_SCALE      4.375f      // mdps/LSB at ±125dps
#define LSM6DS3_GYRO_250DPS_SCALE      8.75f       // mdps/LSB at ±250dps
#define LSM6DS3_GYRO_500DPS_SCALE      17.50f      // mdps/LSB at ±500dps
#define LSM6DS3_GYRO_1000DPS_SCALE     35.0f       // mdps/LSB at ±1000dps
#define LSM6DS3_GYRO_2000DPS_SCALE     70.0f       // mdps/LSB at ±2000dps

// Sensor configuration structure
typedef struct {
    float accel_scale;
    float gyro_scale;
    uint32_t pressure_scale;
    uint32_t temp_scale;
} sensor_config_t;

sensor_config_t sensor_config = {
    .accel_scale = LSM6DS3_ACCEL_2G_SCALE,      // Default ±2g
    .gyro_scale = LSM6DS3_GYRO_250DPS_SCALE,    // Default ±250dps
    .pressure_scale = DPS310_OSR_16X_SCALE_FACTOR,  // Standard precision
    .temp_scale = DPS310_OSR_16X_SCALE_FACTOR
};

// Initialize LSM6DS3TR-C IMU
void init_lsm6ds3_imu(void) {
    printf("Initializing LSM6DS3TR-C IMU...\n");
    
    // Configure accelerometer: ±2g, 104Hz ODR
    spi_write_register(SPI_CS_IMU_PIN, 0x10, 0x40);  // CTRL1_XL
    
    // Configure gyroscope: ±250dps, 104Hz ODR  
    spi_write_register(SPI_CS_IMU_PIN, 0x11, 0x40);  // CTRL2_G
    
    // Enable accelerometer and gyroscope
    spi_write_register(SPI_CS_IMU_PIN, 0x12, 0x00);  // CTRL3_C
    
    printf("LSM6DS3TR-C configured: ±2g accel, ±250dps gyro\n");
}

// Initialize DPS310 Barometer
void init_dps310_baro(void) {
    printf("Initializing DPS310 Barometer...\n");
    
    // Reset the DPS310
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x0C, 0x09);  // RESET register
    sleep_ms(50);  // Wait for reset to complete
    
    // Configure pressure measurement: 16x oversampling, 8 measurements/sec
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x06, 0x44);  // PRS_CFG
    sleep_ms(10);
    
    // Configure temperature measurement: 16x oversampling
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x07, 0x44);  // TMP_CFG
    sleep_ms(10);
    
    // Start continuous pressure and temperature measurement
    spi_write_register_dps310(SPI_CS_BARO_PIN, 0x08, 0x07);  // MEAS_CFG
    sleep_ms(10);
    
    printf("DPS310 configured: 16x OSR, continuous mode\n");
}

// Read IMU data with proper scaling
typedef struct {
    float accel_x, accel_y, accel_z;  // mg
    float gyro_x, gyro_y, gyro_z;     // mdps
} imu_data_t;

imu_data_t read_imu_data(void) {
    imu_data_t data = {0};
    
    // Read accelerometer data (6 bytes starting from 0x28)
    int16_t accel_raw[3];
    for (int i = 0; i < 3; i++) {
        uint8_t low = spi_read_register(SPI_CS_IMU_PIN, 0x28 + (i * 2));
        uint8_t high = spi_read_register(SPI_CS_IMU_PIN, 0x29 + (i * 2));
        accel_raw[i] = (int16_t)((high << 8) | low);
    }
    
    // Read gyroscope data (6 bytes starting from 0x22)
    int16_t gyro_raw[3];
    for (int i = 0; i < 3; i++) {
        uint8_t low = spi_read_register(SPI_CS_IMU_PIN, 0x22 + (i * 2));
        uint8_t high = spi_read_register(SPI_CS_IMU_PIN, 0x23 + (i * 2));
        gyro_raw[i] = (int16_t)((high << 8) | low);
    }
    
    // Apply scaling factors
    data.accel_x = accel_raw[0] * sensor_config.accel_scale;
    data.accel_y = accel_raw[1] * sensor_config.accel_scale;
    data.accel_z = accel_raw[2] * sensor_config.accel_scale;
    
    data.gyro_x = gyro_raw[0] * sensor_config.gyro_scale;
    data.gyro_y = gyro_raw[1] * sensor_config.gyro_scale;
    data.gyro_z = gyro_raw[2] * sensor_config.gyro_scale;
    
    return data;
}

// Read barometer data with proper scaling
typedef struct {
    float pressure;     // Pa
    float temperature;  // °C
} baro_data_t;

baro_data_t read_baro_data(void) {
    baro_data_t data = {0};
    
    // Check if data is ready
    uint8_t status = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x08);
    
    if (status & 0x10) {  // Pressure data ready
        // Read pressure data (3 bytes starting from 0x00)
        uint8_t prs_b2 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x00);
        uint8_t prs_b1 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x01);
        uint8_t prs_b0 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x02);
        
        int32_t prs_raw = (int32_t)((prs_b2 << 16) | (prs_b1 << 8) | prs_b0);
        if (prs_raw & 0x800000) prs_raw |= 0xFF000000;  // Sign extend
        
        // Apply scaling factor (simplified - actual DPS310 needs calibration coefficients)
        data.pressure = (float)prs_raw / sensor_config.pressure_scale * 100000.0f;  // Convert to Pa
    }
    
    if (status & 0x20) {  // Temperature data ready
        // Read temperature data (3 bytes starting from 0x03)
        uint8_t tmp_b2 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x03);
        uint8_t tmp_b1 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x04);
        uint8_t tmp_b0 = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x05);
        
        int32_t tmp_raw = (int32_t)((tmp_b2 << 16) | (tmp_b1 << 8) | tmp_b0);
        if (tmp_raw & 0x800000) tmp_raw |= 0xFF000000;  // Sign extend
        
        // Apply scaling factor (simplified)
        data.temperature = (float)tmp_raw / sensor_config.temp_scale;
    }
    
    return data;
}

// Command processing
void process_commands() {
    int c = getchar_timeout_us(1000); // Non-blocking read
    if (c == PICO_ERROR_TIMEOUT) return;
    
    static char cmd_buffer[64];
    static int cmd_pos = 0;
    
    if (c == '\r' || c == '\n') {
        if (cmd_pos > 0) {
            cmd_buffer[cmd_pos] = '\0';
            
            printf("\nCommand: %s\n", cmd_buffer);
            
            if (strcmp(cmd_buffer, "help") == 0) {
                printf("Available commands:\n");
                printf("  help     - Show this help\n");
                printf("  scan     - Scan all GPIO pins for LEDs\n");
                printf("  test0-3  - Test LED pin sets 0-3\n");
                printf("  flip     - Toggle LED polarity\n");
                printf("  red      - Turn on red LED (manual mode)\n");
                printf("  green    - Turn on green LED (manual mode)\n");
                printf("  blue     - Turn on blue LED (manual mode)\n");
                printf("  off      - Turn off all LEDs (manual mode)\n");
                printf("  auto     - Enable automatic heartbeat LED\n");
                printf("  beep     - Test buzzer\n");
                printf("  sensors  - Read sensor IDs\n");
                printf("  imu      - Read IMU data (accel + gyro)\n");
                printf("  baro     - Read barometer data (pressure + temp)\n");
                printf("  spitest  - Test software SPI communication\n");
                printf("  barotest - Test DPS310 barometer communication\n");
                printf("  status   - Show system status\n");
                printf("  arm      - Arm flight controller (check RC first)\n");
                printf("  disarm   - Disarm flight controller\n");
                printf("  rc       - Show RC input values\n");
                printf("  attitude - Show current attitude estimate\n");
                printf("  calibrate - Start sensor calibration\n");
                printf("  flight   - Show flight control status\n");
            }
            else if (strcmp(cmd_buffer, "scan") == 0) {
                scan_gpio_pins();
            }
            else if (strncmp(cmd_buffer, "test", 4) == 0) {
                int set = cmd_buffer[4] - '0';
                if (set >= 0 && set <= 3) {
                    test_led_pins(set);
                }
            }
            else if (strcmp(cmd_buffer, "flip") == 0) {
                led_active_low = !led_active_low;
                printf("LED polarity: %s\n", led_active_low ? "Active Low" : "Active High");
            }
            else if (strcmp(cmd_buffer, "red") == 0) {
                manual_led_mode = true;
                led_set_rgb(true, false, false);
                printf("Red LED on (manual mode)\n");
            }
            else if (strcmp(cmd_buffer, "green") == 0) {
                manual_led_mode = true;
                led_set_rgb(false, true, false);
                printf("Green LED on (manual mode)\n");
            }
            else if (strcmp(cmd_buffer, "blue") == 0) {
                manual_led_mode = true;
                led_set_rgb(false, false, true);
                printf("Blue LED on (manual mode)\n");
            }
            else if (strcmp(cmd_buffer, "off") == 0) {
                manual_led_mode = true;
                led_set_rgb(false, false, false);
                printf("All LEDs off (manual mode)\n");
            }
            else if (strcmp(cmd_buffer, "auto") == 0) {
                manual_led_mode = false;
                printf("Automatic LED mode (heartbeat enabled)\n");
            }
            else if (strcmp(cmd_buffer, "beep") == 0) {
                printf("Testing buzzer...\n");
                buzzer_beep(500);
                printf("Buzzer test complete\n");
            }
            else if (strcmp(cmd_buffer, "sensors") == 0) {
                printf("Reading sensor IDs...\n");
                
                // Read IMU ID (LSM6DS3TR-C WHO_AM_I register)
                uint8_t imu_id = spi_read_register(SPI_CS_IMU_PIN, 0x0F);
                printf("IMU ID: 0x%02X (expected: 0x6A for LSM6DS3TR-C)\n", imu_id);
                
                // Read Barometer ID (DPS310 Product ID register)
                uint8_t baro_id = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x0D);
                printf("Barometer ID: 0x%02X (expected: 0x10 for DPS310)\n", baro_id);
                
                // Additional debugging
                printf("  SPI Configuration:\n");
                printf("  SCK: GPIO%u, MOSI: GPIO%u, MISO: GPIO%u\n", SPI_SCK_PIN, SPI_MOSI_PIN, SPI_MISO_PIN);
                printf("  IMU CS: GPIO%u, Baro CS: GPIO%u\n", SPI_CS_IMU_PIN, SPI_CS_BARO_PIN);
            }
            else if (strcmp(cmd_buffer, "spitest") == 0) {
                printf("Testing Software SPI communication...\n");
                
                // Test SPI loopback (if MOSI and MISO are connected)
                printf("SPI Pin States:\n");
                printf("  SCK (GPIO%u): %d\n", SPI_SCK_PIN, gpio_get(SPI_SCK_PIN));
                printf("  MOSI (GPIO%u): %d\n", SPI_MOSI_PIN, gpio_get(SPI_MOSI_PIN));
                printf("  MISO (GPIO%u): %d\n", SPI_MISO_PIN, gpio_get(SPI_MISO_PIN));
                printf("  IMU CS (GPIO%u): %d\n", SPI_CS_IMU_PIN, gpio_get(SPI_CS_IMU_PIN));
                printf("  Baro CS (GPIO%u): %d\n", SPI_CS_BARO_PIN, gpio_get(SPI_CS_BARO_PIN));
                
                // Test software SPI transfer
                printf("Testing SPI transfer with pattern 0xAA...\n");
                uint8_t test_result = soft_spi_transfer_byte(0xAA);
                printf("Sent: 0xAA, Received: 0x%02X\n", test_result);
                
                printf("Software SPI test complete\n");
            }
            else if (strcmp(cmd_buffer, "barotest") == 0) {
                printf("Testing DPS310 Barometer communication...\n");
                
                // Test CS pin control
                printf("Testing CS pin (GPIO%u):\n", SPI_CS_BARO_PIN);
                gpio_put(SPI_CS_BARO_PIN, 0);
                printf("  CS Low: %d\n", gpio_get(SPI_CS_BARO_PIN));
                sleep_ms(10);
                gpio_put(SPI_CS_BARO_PIN, 1);
                printf("  CS High: %d\n", gpio_get(SPI_CS_BARO_PIN));
                
                // Try reading multiple registers
                printf("Reading DPS310 registers:\n");
                for (uint8_t reg = 0x0D; reg <= 0x0F; reg++) {
                    uint8_t value = spi_read_register_dps310(SPI_CS_BARO_PIN, reg);
                    printf("  Reg 0x%02X: 0x%02X\n", reg, value);
                    sleep_ms(10);
                }
                
                // Try with different timing
                printf("Trying with slower timing...\n");
                gpio_put(SPI_CS_BARO_PIN, 0);
                sleep_ms(1);
                soft_spi_transfer_byte(0x8D);  // Read Product ID with read bit
                sleep_ms(1);
                uint8_t slow_id = soft_spi_transfer_byte(0x00);
                sleep_ms(1);
                gpio_put(SPI_CS_BARO_PIN, 1);
                printf("  Slow read Product ID: 0x%02X\n", slow_id);
                
                printf("DPS310 test complete\n");
            }
            else if (strcmp(cmd_buffer, "imu") == 0) {
                printf("Reading IMU data...\n");
                imu_data_t imu = read_imu_data();
                
                printf("Accelerometer (mg):\n");
                printf("  X: %8.2f  Y: %8.2f  Z: %8.2f\n", imu.accel_x, imu.accel_y, imu.accel_z);
                
                printf("Gyroscope (mdps):\n");
                printf("  X: %8.2f  Y: %8.2f  Z: %8.2f\n", imu.gyro_x, imu.gyro_y, imu.gyro_z);
                
                // Calculate magnitude
                float accel_mag = sqrtf(imu.accel_x*imu.accel_x + imu.accel_y*imu.accel_y + imu.accel_z*imu.accel_z);
                printf("Accel magnitude: %8.2f mg (1g = 1000mg)\n", accel_mag);
            }
            else if (strcmp(cmd_buffer, "baro") == 0) {
                printf("Reading barometer data...\n");
                baro_data_t baro = read_baro_data();
                
                printf("Pressure: %8.2f Pa (%8.2f hPa)\n", baro.pressure, baro.pressure / 100.0f);
                printf("Temperature: %6.2f °C\n", baro.temperature);
                
                // Calculate altitude (simplified)
                float altitude = 44330.0f * (1.0f - powf(baro.pressure / 101325.0f, 0.1903f));
                printf("Estimated altitude: %8.2f m\n", altitude);
            }
            else if (strcmp(cmd_buffer, "status") == 0) {
                printf("OpenFC2040 Status:\n");
                printf("  LED pins: R=%u, G=%u, B=%u\n", LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);
                printf("  LED polarity: %s\n", led_active_low ? "Active Low" : "Active High");
                printf("  SPI pins: SCK=%u, MOSI=%u, MISO=%u\n", SPI_SCK_PIN, SPI_MOSI_PIN, SPI_MISO_PIN);
                printf("  ESC pins: ESC1=%u, ESC2=%u, ESC3=%u, ESC4=%u\n", ESC1_PIN, ESC2_PIN, ESC3_PIN, ESC4_PIN);
                printf("  Other pins: RC_IN=%u, BUZZER=%u\n", RC_INPUT_PIN, BUZZER_PIN);
            }
            else if (strcmp(cmd_buffer, "arm") == 0) {
                if (flight_control_arm(&flight_control, &rc_input)) {
                    printf("Flight controller ARMED\n");
                } else {
                    printf("Arming failed - check RC input and throttle position\n");
                }
            }
            else if (strcmp(cmd_buffer, "disarm") == 0) {
                flight_control_disarm(&flight_control);
                printf("Flight controller DISARMED\n");
            }
            else if (strcmp(cmd_buffer, "rc") == 0) {
                printf("RC Input Status:\n");
                printf("  Valid: %s, Failsafe: %s\n", 
                       rc_input.valid ? "YES" : "NO", 
                       rc_input.failsafe ? "YES" : "NO");
                printf("  Channels (us): ");
                for (int i = 0; i < RC_CHANNEL_COUNT; i++) {
                    printf("%u ", rc_input.channels[i]);
                }
                printf("\n");
                printf("  Normalized: R=%.2f P=%.2f T=%.2f Y=%.2f\n",
                       rc_input_get_normalized(&rc_input, RC_CHANNEL_ROLL),
                       rc_input_get_normalized(&rc_input, RC_CHANNEL_PITCH),
                       rc_input_get_normalized(&rc_input, RC_CHANNEL_THROTTLE),
                       rc_input_get_normalized(&rc_input, RC_CHANNEL_YAW));
            }
            else if (strcmp(cmd_buffer, "attitude") == 0) {
                printf("Attitude Estimate:\n");
                printf("  Roll: %7.2f°  Pitch: %7.2f°  Yaw: %7.2f°\n",
                       attitude_estimator.attitude.roll,
                       attitude_estimator.attitude.pitch,
                       attitude_estimator.attitude.yaw);
                printf("  Rates: R=%6.1f P=%6.1f Y=%6.1f °/s\n",
                       attitude_estimator.attitude.roll_rate,
                       attitude_estimator.attitude.pitch_rate,
                       attitude_estimator.attitude.yaw_rate);
                printf("  Initialized: %s\n", attitude_estimator.initialized ? "YES" : "NO");
            }
            else if (strcmp(cmd_buffer, "calibrate") == 0) {
                sensor_calibration_start(&sensor_calibration);
                printf("Sensor calibration started - move board in all orientations for 30 seconds\n");
            }
            else if (strcmp(cmd_buffer, "flight") == 0) {
                printf("Flight Control Status:\n");
                printf("  Mode: %d, Armed: %s, Motors: %s\n",
                       flight_control.mode,
                       flight_control.armed ? "YES" : "NO",
                       flight_control.motors_enabled ? "ON" : "OFF");
                printf("  Motor outputs: %u %u %u %u\n",
                       flight_control.motor_outputs[0],
                       flight_control.motor_outputs[1],
                       flight_control.motor_outputs[2],
                       flight_control.motor_outputs[3]);
                printf("  Setpoints: R=%.1f P=%.1f Y=%.1f T=%.1f\n",
                       flight_control.roll_setpoint,
                       flight_control.pitch_setpoint,
                       flight_control.yaw_setpoint,
                       flight_control.throttle_setpoint);
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
        putchar(c); // Echo character
    }
}

// Main application
int main() {
    stdio_init_all();
    
    printf("\n=== OpenFC2040 Enhanced Firmware ===\n");
    printf("Firmware Starting...\n");
    
    // Initialize hardware
    led_init();
    buzzer_init();
    pwm_init_escs();
    spi_init_sensors();
    
    // Initialize flight control systems
    flight_control_init(&flight_control);
    rc_input_init(RC_INPUT_PPM);  // Assume PPM input
    attitude_estimator_init(&attitude_estimator);
    sensor_calibration_init(&sensor_calibration);
    
    printf("Hardware and flight control systems initialized successfully\n");
    
    // Startup sequence - flash LEDs and buzzer
    printf("LED startup sequence...\n");
    for (int i = 0; i < 3; i++) {
        led_set_rgb(true, false, false);
        sleep_ms(200);
        led_set_rgb(false, true, false);
        sleep_ms(200);
        led_set_rgb(false, false, true);
        sleep_ms(200);
        led_set_rgb(false, false, false);
        sleep_ms(200);
    }
    
    // Startup buzzer sequence
    printf("Buzzer startup sequence...\n");
    buzzer_startup_sequence();
    
    // Check sensors
    printf("Reading sensor IDs...\n");
    uint8_t imu_id = spi_read_register(SPI_CS_IMU_PIN, 0x0F);  // WHO_AM_I register
    uint8_t baro_id = spi_read_register_dps310(SPI_CS_BARO_PIN, 0x0D);  // Product ID
    printf("IMU ID: 0x%02X (expected: 0x6A for LSM6DS3TR-C)\n", imu_id);
    printf("Barometer ID: 0x%02X (expected: 0x10 for DPS310)\n", baro_id);
    
    // Initialize sensors if IDs are correct
    if (imu_id == 0x6A) {
        init_lsm6ds3_imu();
    } else {
        printf("Warning: IMU not detected or incorrect ID\n");
    }
    
    if (baro_id == 0x10) {
        init_dps310_baro();
    } else {
        printf("Warning: Barometer not detected or incorrect ID\n");
    }
    
    printf("\nType 'help' for available commands\n");
    printf("> ");
    
    // Main loop
    uint32_t loop_count = 0;
    uint32_t last_heartbeat = 0;
    uint32_t last_flight_update = 0;
    uint32_t last_sensor_read = 0;
    
    while (true) {
        uint32_t current_time = time_us_32();
        
        // Process serial commands
        process_commands();
        
        // Update RC input
        rc_input_update(&rc_input);
        
        // Read sensors and update attitude every 5ms (200Hz)
        if (current_time - last_sensor_read >= 5000) {
            // Read IMU data
            imu_data_t imu_data;
            imu_data.accel.x = read_imu_data().accel_x * 9.81f; // Convert to m/s²
            imu_data.accel.y = read_imu_data().accel_y * 9.81f;
            imu_data.accel.z = read_imu_data().accel_z * 9.81f;
            imu_data.gyro.x = read_imu_data().gyro_x * DEG_TO_RAD; // Convert to rad/s
            imu_data.gyro.y = read_imu_data().gyro_y * DEG_TO_RAD;
            imu_data.gyro.z = read_imu_data().gyro_z * DEG_TO_RAD;
            imu_data.valid = true;
            
            // Update attitude estimator
            float dt = (current_time - last_sensor_read) / 1000000.0f;
            attitude_estimator_update(&attitude_estimator, &imu_data, dt);
            
            // Update sensor calibration if in progress
            if (sensor_calibration.in_progress) {
                if (sensor_calibration_update(&sensor_calibration, &imu_data)) {
                    sensor_calibration_finish(&sensor_calibration, &attitude_estimator);
                }
            }
            
            last_sensor_read = current_time;
        }
        
        // Flight control update every 2ms (500Hz)
        if (current_time - last_flight_update >= 2000) {
            float dt = (current_time - last_flight_update) / 1000000.0f;
            flight_control_update(&flight_control, &rc_input, &attitude_estimator.attitude, dt);
            last_flight_update = current_time;
        }
        
        // Status LED indication based on flight state
        if (!manual_led_mode) {
            if (flight_control.armed) {
                // Armed - green LED solid
                led_set_rgb(false, true, false);
            } else if (rc_input.valid && !rc_input.failsafe) {
                // RC valid but disarmed - blue heartbeat
                if (loop_count - last_heartbeat >= 100) {
                    led_set_rgb(false, false, true);
                    sleep_ms(50);
                    led_set_rgb(false, false, false);
                    last_heartbeat = loop_count;
                }
            } else {
                // No RC or failsafe - red LED
                led_set_rgb(true, false, false);
            }
        }
        
        // Print status every 10 seconds
        if (loop_count % 1000 == 0) {
            printf("\nOpenFC2040 Flight Controller - Loop: %lu\n", loop_count);
            printf("  Armed: %s, RC: %s, Attitude: %s\n",
                   flight_control.armed ? "YES" : "NO",
                   rc_input.valid ? "OK" : "FAIL",
                   attitude_estimator.initialized ? "OK" : "INIT");
            printf("> ");
        }
        
        sleep_ms(1);  // 1ms loop for better timing
        loop_count++;
    }
    
    return 0;
}
