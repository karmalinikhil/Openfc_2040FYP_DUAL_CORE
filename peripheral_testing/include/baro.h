#ifndef BARO_H
#define BARO_H

#include <stdbool.h>

#define DPS310_PRODUCT_ID       0x0D
#define DPS310_PRODUCT_ID_VALUE 0x10

#define DPS310_PRS_B2           0x00
#define DPS310_TMP_B2           0x03
#define DPS310_PRS_CFG          0x06
#define DPS310_TMP_CFG          0x07
#define DPS310_MEAS_CFG         0x08
#define DPS310_CFG_REG          0x09
#define DPS310_RESET            0x0C

bool baro_init(void);
bool baro_read(float *pressure, float *temperature);
void baro_test(void);

#endif
