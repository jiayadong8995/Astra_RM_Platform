#include "device_backend.h"

#include "actuator/motor/motor_actuator_device.h"
#include "imu/bmi088_device.h"
#include "remote/dbus_remote_device.h"

void platform_device_backend_bind_default(platform_device_layer_t *layer)
{
  platform_bmi088_device_bind(&layer->imu, 0);
  platform_dbus_remote_device_bind(&layer->remote, 0);
  platform_motor_actuator_device_bind(&layer->motor, 0);
}
