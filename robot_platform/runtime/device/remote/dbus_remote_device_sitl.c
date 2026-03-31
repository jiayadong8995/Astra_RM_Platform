#include "dbus_remote_device.h"

#include <time.h>

#include "dbus/remote_control.h"

static platform_device_result_t platform_dbus_remote_init(platform_remote_device_t *device);
static platform_device_result_t platform_dbus_remote_read_input(platform_remote_device_t *device,
                                                                platform_rc_input_t *input);
static uint32_t platform_remote_sample_time_us(void);

void platform_dbus_remote_device_bind(platform_remote_device_t *device,
                                      const platform_dbus_remote_device_config_t *config)
{
  device->name = "dbus_remote_sitl_udp";
  device->context = (void *)config;
  device->ops.init = platform_dbus_remote_init;
  device->ops.read_input = platform_dbus_remote_read_input;
}

static platform_device_result_t platform_dbus_remote_init(platform_remote_device_t *device)
{
  platform_dbus_remote_device_config_t *config = (platform_dbus_remote_device_config_t *)device->context;

  if (config == 0 || config->acquire_fn == 0 || config->is_error_fn == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }

  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_dbus_remote_read_input(platform_remote_device_t *device,
                                                                platform_rc_input_t *input)
{
  platform_dbus_remote_device_config_t *config = (platform_dbus_remote_device_config_t *)device->context;
  const RC_ctrl_t *rc = 0;

  if (config == 0 || config->acquire_fn == 0 || config->is_error_fn == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }

  rc = (const RC_ctrl_t *)config->acquire_fn();
  if (rc == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }

  input->channels[0] = rc->rc.ch[0];
  input->channels[1] = rc->rc.ch[1];
  input->channels[2] = rc->rc.ch[2];
  input->channels[3] = rc->rc.ch[3];
  input->channels[4] = rc->rc.ch[4];
  input->switches[0] = (uint8_t)rc->rc.s[0];
  input->switches[1] = (uint8_t)rc->rc.s[1];
  input->mouse_x = rc->mouse.x;
  input->mouse_y = rc->mouse.y;
  input->mouse_z = rc->mouse.z;
  input->mouse_left = rc->mouse.press_l;
  input->mouse_right = rc->mouse.press_r;
  input->keyboard_mask = rc->key.v;
  input->source = 1U;
  input->sample_time_us = platform_remote_sample_time_us();
  input->valid = (config->is_error_fn() == 0U);
  device->stamp.valid = input->valid;
  return input->valid ? PLATFORM_DEVICE_RESULT_OK : PLATFORM_DEVICE_RESULT_INVALID;
}

static uint32_t platform_remote_sample_time_us(void)
{
  struct timespec now = {0};

  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
  {
    return 0U;
  }

  return (uint32_t)(((uint64_t)now.tv_sec * 1000000ULL) + ((uint64_t)now.tv_nsec / 1000ULL));
}
