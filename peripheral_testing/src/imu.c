#include <stdio.h>
#include "pico/stdlib.h"
#include "board_config.h"
#include "spi.h"
#include "imu.h"

bool imu_init(void) {
    // Check WHO_AM_I
    uint8_t who = spi_read_reg(IMU_SPI, IMU_CS_PIN, LSM6DS3_WHO_AM_I);
    if (who != LSM6DS3_WHO_AM_I_VALUE) {
        printf("IMU WHO_AM_I failed: got 0x%02X, expected 0x%02X\n", who, LSM6DS3_WHO_AM_I_VALUE);
        return false;
    }
    
    // Software reset
    spi_write_reg(IMU_SPI, IMU_CS_PIN, LSM6DS3_CTRL3_C, 0x01);
    sleep_ms(10);
    
    // Configure accelerometer: 104 Hz, ±2g
    spi_write_reg(IMU_SPI, IMU_CS_PIN, LSM6DS3_CTRL1_XL, 0x40);
    
    // Configure gyroscope: 104 Hz, 250 dps
    spi_write_reg(IMU_SPI, IMU_CS_PIN, LSM6DS3_CTRL2_G, 0x40);
    
    return true;
}

bool imu_read_accel(float *x, float *y, float *z) {
    uint8_t buf[6];
    spi_read_bytes(IMU_SPI, IMU_CS_PIN, LSM6DS3_OUTX_L_XL, buf, 6);
    
    int16_t raw_x = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t raw_y = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t raw_z = (int16_t)((buf[5] << 8) | buf[4]);
    
    // Convert to g (±2g range, 16-bit)
    float scale = 2.0f / 32768.0f;
    *x = raw_x * scale;
    *y = raw_y * scale;
    *z = raw_z * scale;
    
    return true;
}

bool imu_read_gyro(float *x, float *y, float *z) {
    uint8_t buf[6];
    spi_read_bytes(IMU_SPI, IMU_CS_PIN, LSM6DS3_OUTX_L_G, buf, 6);
    
    int16_t raw_x = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t raw_y = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t raw_z = (int16_t)((buf[5] << 8) | buf[4]);
    
    // Convert to dps (250 dps range, 16-bit)
    float scale = 250.0f / 32768.0f;
    *x = raw_x * scale;
    *y = raw_y * scale;
    *z = raw_z * scale;
    
    return true;
}

void imu_test(void) {
    printf("Testing IMU (LSM6DS3TR-C)...\n");
    
    if (!imu_init()) {
        printf("IMU initialization FAILED!\n");
        return;
    }
    printf("IMU initialized successfully.\n");
    
    printf("Reading 5 samples...\n");
    for (int i = 0; i < 5; i++) {
        float ax, ay, az, gx, gy, gz;
        
        imu_read_accel(&ax, &ay, &az);
        imu_read_gyro(&gx, &gy, &gz);
        
        printf("  Accel: X=%.3f Y=%.3f Z=%.3f g\n", ax, ay, az);
        printf("  Gyro:  X=%.1f Y=%.1f Z=%.1f dps\n", gx, gy, gz);
        
        sleep_ms(200);
    }
    
    printf("IMU test complete.\n");
}
