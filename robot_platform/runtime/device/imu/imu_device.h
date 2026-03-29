#ifndef PLATFORM_IMU_DEVICE_H
#define PLATFORM_IMU_DEVICE_H

#include "../device_types.h"
#include "../../control/contracts/device_input.h"

typedef struct platform_imu_device platform_imu_device_t;

typedef struct {
  platform_device_result_t (*init)(platform_imu_device_t *device);
  platform_device_result_t (*read_sample)(platform_imu_device_t *device, platform_imu_sample_t *sample);
} platform_imu_device_ops_t;

struct platform_imu_device {
  const char *name;
  void *context;
  platform_device_stamp_t stamp;
  platform_imu_device_ops_t ops;
};

#endif
