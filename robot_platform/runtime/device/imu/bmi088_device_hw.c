#include "bmi088_device.h"

#include "../../bsp/devices/bmi088/bmi088_node.h"
#include "../../bsp/devices/bmi088/BMI088driver.h"

static platform_device_result_t platform_bmi088_init(platform_imu_device_t *device);
static platform_device_result_t platform_bmi088_read_sample(platform_imu_device_t *device,
                                                            platform_imu_sample_t *sample);

void platform_bmi088_device_bind(platform_imu_device_t *device,
                                 const platform_bmi088_device_config_t *config)
{
  device->name = "bmi088";
  device->context = (void *)config;
  device->ops.init = platform_bmi088_init;
  device->ops.read_sample = platform_bmi088_read_sample;
}

void platform_bmi088_device_bind_default(platform_imu_device_t *device)
{
  platform_bmi088_device_bind(device, platform_bmi088_node_default());
}

static platform_device_result_t platform_bmi088_init(platform_imu_device_t *device)
{
  platform_bmi088_device_config_t *context = (platform_bmi088_device_config_t *)device->context;

  if (context == 0 || context->init_fn == 0 || context->spi_handle == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }

  context->init_fn(context->spi_handle, context->calibrate);
  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_bmi088_read_sample(platform_imu_device_t *device,
                                                            platform_imu_sample_t *sample)
{
  platform_bmi088_device_config_t *context = (platform_bmi088_device_config_t *)device->context;
  IMU_Data_t *bmi088_state = (IMU_Data_t *)context->sample_state;

  if (context == 0 || context->read_fn == 0 || bmi088_state == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }

  context->read_fn(context->sample_state);

  sample->accel[0] = bmi088_state->Accel[0];
  sample->accel[1] = bmi088_state->Accel[1];
  sample->accel[2] = bmi088_state->Accel[2];
  sample->gyro[0] = bmi088_state->Gyro[0];
  sample->gyro[1] = bmi088_state->Gyro[1];
  sample->gyro[2] = bmi088_state->Gyro[2];
  sample->temperature = bmi088_state->Temperature;
  sample->valid = true;
  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}
