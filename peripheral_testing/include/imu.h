#ifndef IMU_H
#define IMU_H

#include <stdbool.h>

#define LSM6DS3_WHO_AM_I        0x0F
#define LSM6DS3_WHO_AM_I_VALUE  0x69

#define LSM6DS3_CTRL1_XL        0x10
#define LSM6DS3_CTRL2_G         0x11
#define LSM6DS3_CTRL3_C         0x12
#define LSM6DS3_STATUS_REG      0x1E
#define LSM6DS3_OUTX_L_G        0x22
#define LSM6DS3_OUTX_L_XL       0x28

bool imu_init(void);
bool imu_read_accel(float *x, float *y, float *z);
bool imu_read_gyro(float *x, float *y, float *z);
void imu_test(void);

#endif
