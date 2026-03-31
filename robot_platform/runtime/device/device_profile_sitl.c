#include "device_profile.h"

#include "device_layer.h"

#include "actuator/motor/motor_actuator_device.h"
#include "imu/bmi088/BMI088driver.h"
#include "imu/bmi088_device.h"
#include "remote/dbus/remote_control.h"
#include "remote/dbus_remote_device.h"

static void bind_sitl_imu(platform_device_layer_t *layer);
static void bind_sitl_remote(platform_device_layer_t *layer);
static void bind_sitl_motor(platform_device_layer_t *layer);

static const platform_bmi088_device_config_t g_platform_bmi088_sitl = {
  .spi_handle = 0,
  .sample_state = &BMI088,
  .calibrate = 0U,
  .init_fn = (void (*)(void *, uint8_t))BMI088_Init,
  .read_fn = (void (*)(void *))BMI088_Read,
};

static const platform_dbus_remote_device_config_t g_platform_remote_sitl = {
  .acquire_fn = (const void *(*)(void))get_remote_control_point,
  .is_error_fn = RC_data_is_error,
};

static const platform_device_profile_t g_platform_device_profile_sitl = {
  .name = "sitl",
  .bind_imu = bind_sitl_imu,
  .bind_remote = bind_sitl_remote,
  .bind_motor = bind_sitl_motor,
};

const platform_device_profile_t *platform_device_profile_sitl(void)
{
  return &g_platform_device_profile_sitl;
}

static void bind_sitl_imu(platform_device_layer_t *layer)
{
  platform_bmi088_device_bind(&layer->imu, &g_platform_bmi088_sitl);
}

static void bind_sitl_remote(platform_device_layer_t *layer)
{
  /* Keep the authoritative chain as:
   * remote input + state observation -> intent parsing / mode constraints -> chassis control -> execution output
   */
  platform_dbus_remote_device_bind(&layer->remote, &g_platform_remote_sitl);
}

static void bind_sitl_motor(platform_device_layer_t *layer)
{
  platform_motor_actuator_device_bind(&layer->motor, 0);
}
