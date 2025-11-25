#ifndef SPI_H
#define SPI_H

#include "hardware/spi.h"

void spi_bus_init(void);
void spi_cs_select(uint cs_pin);
void spi_cs_deselect(uint cs_pin);
uint8_t spi_read_reg(spi_inst_t *spi, uint cs_pin, uint8_t reg);
void spi_write_reg(spi_inst_t *spi, uint cs_pin, uint8_t reg, uint8_t data);
void spi_read_bytes(spi_inst_t *spi, uint cs_pin, uint8_t reg, uint8_t *buf, size_t len);

#endif
