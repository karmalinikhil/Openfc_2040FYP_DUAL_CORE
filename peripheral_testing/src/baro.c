#include <stdio.h>
#include "pico/stdlib.h"
#include "board_config.h"
#include "spi.h"
#include "baro.h"

static int32_t c0, c1, c00, c10, c01, c11, c20, c21, c30;

bool baro_init(void) {
    // Check Product ID
    uint8_t id = spi_read_reg(BARO_SPI, BARO_CS_PIN, DPS310_PRODUCT_ID);
    if ((id & 0xF0) != DPS310_PRODUCT_ID_VALUE) {
        printf("Barometer ID failed: got 0x%02X, expected 0x1X\n", id);
        return false;
    }
    
    // Soft reset
    spi_write_reg(BARO_SPI, BARO_CS_PIN, DPS310_RESET, 0x89);
    sleep_ms(50);
    
    // Read calibration coefficients (simplified)
    uint8_t coef[18];
    for (int i = 0; i < 18; i++) {
        coef[i] = spi_read_reg(BARO_SPI, BARO_CS_PIN, 0x10 + i);
    }
    
    // Parse coefficients (simplified)
    c0 = ((int16_t)(coef[0] << 4) | (coef[1] >> 4));
    if (c0 & 0x800) c0 |= 0xFFFFF000;
    
    c1 = ((int16_t)((coef[1] & 0x0F) << 8) | coef[2]);
    if (c1 & 0x800) c1 |= 0xFFFFF000;
    
    // Configure: 1 measurement/sec, 1x oversampling
    spi_write_reg(BARO_SPI, BARO_CS_PIN, DPS310_PRS_CFG, 0x00);
    spi_write_reg(BARO_SPI, BARO_CS_PIN, DPS310_TMP_CFG, 0x80);  // External sensor
    spi_write_reg(BARO_SPI, BARO_CS_PIN, DPS310_MEAS_CFG, 0x07);  // Continuous
    
    return true;
}

bool baro_read(float *pressure, float *temperature) {
    // Read raw pressure
    uint8_t prs[3];
    prs[0] = spi_read_reg(BARO_SPI, BARO_CS_PIN, DPS310_PRS_B2);
    prs[1] = spi_read_reg(BARO_SPI, BARO_CS_PIN, DPS310_PRS_B2 + 1);
    prs[2] = spi_read_reg(BARO_SPI, BARO_CS_PIN, DPS310_PRS_B2 + 2);
    
    int32_t raw_prs = ((int32_t)prs[0] << 16) | ((int32_t)prs[1] << 8) | prs[2];
    if (raw_prs & 0x800000) raw_prs |= 0xFF000000;
    
    // Read raw temperature
    uint8_t tmp[3];
    tmp[0] = spi_read_reg(BARO_SPI, BARO_CS_PIN, DPS310_TMP_B2);
    tmp[1] = spi_read_reg(BARO_SPI, BARO_CS_PIN, DPS310_TMP_B2 + 1);
    tmp[2] = spi_read_reg(BARO_SPI, BARO_CS_PIN, DPS310_TMP_B2 + 2);
    
    int32_t raw_tmp = ((int32_t)tmp[0] << 16) | ((int32_t)tmp[1] << 8) | tmp[2];
    if (raw_tmp & 0x800000) raw_tmp |= 0xFF000000;
    
    // Simplified conversion (without full calibration)
    float scaled_tmp = (float)raw_tmp / 524288.0f;
    float scaled_prs = (float)raw_prs / 524288.0f;
    
    *temperature = c0 * 0.5f + c1 * scaled_tmp;
    *pressure = raw_prs / 100.0f;  // Rough approximation in hPa
    
    return true;
}

void baro_test(void) {
    printf("Testing Barometer (DPS310)...\n");
    
    if (!baro_init()) {
        printf("Barometer initialization FAILED!\n");
        return;
    }
    printf("Barometer initialized successfully.\n");
    
    sleep_ms(100);  // Wait for first measurement
    
    printf("Reading 5 samples...\n");
    for (int i = 0; i < 5; i++) {
        float pressure, temperature;
        
        baro_read(&pressure, &temperature);
        printf("  Pressure: %.2f hPa, Temperature: %.2f C\n", pressure, temperature);
        
        sleep_ms(200);
    }
    
    printf("Barometer test complete.\n");
}
