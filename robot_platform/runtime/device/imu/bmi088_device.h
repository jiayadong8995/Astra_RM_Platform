#ifndef PLATFORM_BMI088_DEVICE_H
#define PLATFORM_BMI088_DEVICE_H

#include <stdint.h>

#include "imu_device.h"

typedef struct {
  void *spi_handle;
  void *sample_state;
  uint8_t calibrate;
  void (*init_fn)(void *spi_handle, uint8_t calibrate);
  void (*read_fn)(void *sample_state);
} platform_bmi088_device_config_t;

void platform_bmi088_device_bind(platform_imu_device_t *device,
                                 const platform_bmi088_device_config_t *config);

void platform_bmi088_device_bind_default(platform_imu_device_t *device);

#endif
