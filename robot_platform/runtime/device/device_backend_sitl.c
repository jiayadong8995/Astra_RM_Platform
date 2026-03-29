#include "device_layer.h"

#include "actuator/motor/motor_actuator_device.h"
#include "imu/bmi088_device.h"
#include "remote/dbus_remote_device.h"

static void bind_sitl_imu(platform_device_layer_t *layer);
static void bind_sitl_remote(platform_device_layer_t *layer);
static void bind_sitl_motor(platform_device_layer_t *layer);

void platform_device_backend_bind_default(platform_device_layer_t *layer)
{
  bind_sitl_imu(layer);
  bind_sitl_remote(layer);
  bind_sitl_motor(layer);
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
