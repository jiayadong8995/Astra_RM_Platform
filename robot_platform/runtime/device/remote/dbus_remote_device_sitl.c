#include "dbus_remote_device.h"

static platform_device_result_t platform_dbus_remote_init(platform_remote_device_t *device);
static platform_device_result_t platform_dbus_remote_read_input(platform_remote_device_t *device,
                                                                platform_rc_input_t *input);

void platform_dbus_remote_device_bind_default(platform_remote_device_t *device)
{
  device->name = "dbus_remote_sitl_stub";
  device->context = 0;
  device->ops.init = platform_dbus_remote_init;
  device->ops.read_input = platform_dbus_remote_read_input;
}

static platform_device_result_t platform_dbus_remote_init(platform_remote_device_t *device)
{
  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_dbus_remote_read_input(platform_remote_device_t *device,
                                                                platform_rc_input_t *input)
{
  (void)device;

  for (uint8_t i = 0; i < PLATFORM_RC_CHANNEL_COUNT; ++i)
  {
    input->channels[i] = 0;
  }
  for (uint8_t i = 0; i < PLATFORM_RC_SWITCH_COUNT; ++i)
  {
    input->switches[i] = 0U;
  }
  input->mouse_x = 0;
  input->mouse_y = 0;
  input->mouse_z = 0;
  input->mouse_left = 0U;
  input->mouse_right = 0U;
  input->keyboard_mask = 0U;
  input->source = 1U;
  input->sample_time_us = 0U;
  input->valid = false;
  return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
}
