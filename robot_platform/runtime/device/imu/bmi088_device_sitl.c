#include "bmi088_device.h"

static platform_device_result_t platform_bmi088_init(platform_imu_device_t *device);
static platform_device_result_t platform_bmi088_read_sample(platform_imu_device_t *device,
                                                            platform_imu_sample_t *sample);

void platform_bmi088_device_bind(platform_imu_device_t *device,
                                 const platform_bmi088_device_config_t *config)
{
  (void)config;
  device->name = "bmi088_sitl_stub";
  device->context = 0;
  device->ops.init = platform_bmi088_init;
  device->ops.read_sample = platform_bmi088_read_sample;
}

static platform_device_result_t platform_bmi088_init(platform_imu_device_t *device)
{
  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_bmi088_read_sample(platform_imu_device_t *device,
                                                            platform_imu_sample_t *sample)
{
  (void)device;

  sample->accel[0] = 0.0f;
  sample->accel[1] = 0.0f;
  sample->accel[2] = 9.81f;
  sample->gyro[0] = 0.0f;
  sample->gyro[1] = 0.0f;
  sample->gyro[2] = 0.0f;
  sample->temperature = 26.0f;
  sample->sample_time_us = 1000U;
  sample->valid = true;
  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}
