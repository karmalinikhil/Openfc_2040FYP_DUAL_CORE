#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "board_config.h"
#include "spi.h"

void spi_bus_init(void) {
    // Initialize SPI1 for IMU and Barometer
    spi_init(IMU_SPI, SPI_FREQ_SLOW);
    gpio_set_function(IMU_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(IMU_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(IMU_MISO_PIN, GPIO_FUNC_SPI);
    
    // IMU CS
    gpio_init(IMU_CS_PIN);
    gpio_set_dir(IMU_CS_PIN, GPIO_OUT);
    gpio_put(IMU_CS_PIN, 1);
    
    // Barometer CS
    gpio_init(BARO_CS_PIN);
    gpio_set_dir(BARO_CS_PIN, GPIO_OUT);
    gpio_put(BARO_CS_PIN, 1);
    
    // Initialize SPI0 for SD Card
    spi_init(SD_SPI, SPI_FREQ_SLOW);
    gpio_set_function(SD_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
    
    // SD CS
    gpio_init(SD_CS_PIN);
    gpio_set_dir(SD_CS_PIN, GPIO_OUT);
    gpio_put(SD_CS_PIN, 1);
    
    printf("SPI buses initialized.\n");
}

void spi_cs_select(uint cs_pin) {
    gpio_put(cs_pin, 0);
    sleep_us(1);
}

void spi_cs_deselect(uint cs_pin) {
    gpio_put(cs_pin, 1);
    sleep_us(1);
}

uint8_t spi_read_reg(spi_inst_t *spi, uint cs_pin, uint8_t reg) {
    uint8_t tx_buf[2] = {reg | 0x80, 0x00};  // Read flag
    uint8_t rx_buf[2];
    
    spi_cs_select(cs_pin);
    spi_write_read_blocking(spi, tx_buf, rx_buf, 2);
    spi_cs_deselect(cs_pin);
    
    return rx_buf[1];
}

void spi_write_reg(spi_inst_t *spi, uint cs_pin, uint8_t reg, uint8_t data) {
    uint8_t tx_buf[2] = {reg & 0x7F, data};  // Write flag (clear MSB)
    
    spi_cs_select(cs_pin);
    spi_write_blocking(spi, tx_buf, 2);
    spi_cs_deselect(cs_pin);
}

void spi_read_bytes(spi_inst_t *spi, uint cs_pin, uint8_t reg, uint8_t *buf, size_t len) {
    uint8_t tx = reg | 0x80;  // Read flag
    
    spi_cs_select(cs_pin);
    spi_write_blocking(spi, &tx, 1);
    spi_read_blocking(spi, 0, buf, len);
    spi_cs_deselect(cs_pin);
}
