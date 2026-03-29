#include "bmi088_device.h"

#include "../../bsp/devices/bmi088/BMI088driver.h"
#include "spi.h"

typedef struct {
  uint8_t calibrate;
} platform_bmi088_device_context_t;

static platform_device_result_t platform_bmi088_init(platform_imu_device_t *device);
static platform_device_result_t platform_bmi088_read_sample(platform_imu_device_t *device,
                                                            platform_imu_sample_t *sample);

static platform_bmi088_device_context_t g_platform_bmi088_context = {
  .calibrate = 1U,
};

void platform_bmi088_device_bind_default(platform_imu_device_t *device)
{
  device->name = "bmi088_hw";
  device->context = &g_platform_bmi088_context;
  device->ops.init = platform_bmi088_init;
  device->ops.read_sample = platform_bmi088_read_sample;
}

static platform_device_result_t platform_bmi088_init(platform_imu_device_t *device)
{
  platform_bmi088_device_context_t *context = (platform_bmi088_device_context_t *)device->context;

  BMI088_Init(&hspi2, context->calibrate);
  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_bmi088_read_sample(platform_imu_device_t *device,
                                                            platform_imu_sample_t *sample)
{
  (void)device;

  BMI088_Read(&BMI088);

  sample->accel[0] = BMI088.Accel[0];
  sample->accel[1] = BMI088.Accel[1];
  sample->accel[2] = BMI088.Accel[2];
  sample->gyro[0] = BMI088.Gyro[0];
  sample->gyro[1] = BMI088.Gyro[1];
  sample->gyro[2] = BMI088.Gyro[2];
  sample->temperature = BMI088.Temperature;
  sample->valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}
