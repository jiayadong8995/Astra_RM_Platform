#ifndef PLATFORM_DBUS_REMOTE_DEVICE_H
#define PLATFORM_DBUS_REMOTE_DEVICE_H

#include <stdint.h>

#include "remote_device.h"

typedef struct {
  const void *(*acquire_fn)(void);
  uint8_t (*is_error_fn)(void);
} platform_dbus_remote_device_config_t;

void platform_dbus_remote_device_bind(platform_remote_device_t *device,
                                      const platform_dbus_remote_device_config_t *config);

void platform_dbus_remote_device_bind_default(platform_remote_device_t *device);

#endif
