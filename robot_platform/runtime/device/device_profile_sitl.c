#include "device_profile.h"

#include "device_layer.h"

#include "actuator/motor/motor_actuator_device.h"
#include "imu/bmi088_device.h"
#include "remote/dbus_remote_device.h"

static void bind_sitl_imu(platform_device_layer_t *layer);
static void bind_sitl_remote(platform_device_layer_t *layer);
static void bind_sitl_motor(platform_device_layer_t *layer);

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
  platform_bmi088_device_bind(&layer->imu, 0);
}

static void bind_sitl_remote(platform_device_layer_t *layer)
{
  platform_dbus_remote_device_bind(&layer->remote, 0);
}

static void bind_sitl_motor(platform_device_layer_t *layer)
{
  platform_motor_actuator_device_bind(&layer->motor, 0);
}
