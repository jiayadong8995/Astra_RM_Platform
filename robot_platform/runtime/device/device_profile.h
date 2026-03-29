#ifndef PLATFORM_DEVICE_PROFILE_H
#define PLATFORM_DEVICE_PROFILE_H

#include "device_types.h"

typedef struct platform_device_layer platform_device_layer_t;

typedef struct {
  const char *name;
  void (*bind_imu)(platform_device_layer_t *layer);
  void (*bind_remote)(platform_device_layer_t *layer);
  void (*bind_motor)(platform_device_layer_t *layer);
} platform_device_profile_t;

const platform_device_profile_t *platform_device_profile_hw(void);
const platform_device_profile_t *platform_device_profile_sitl(void);

#endif
