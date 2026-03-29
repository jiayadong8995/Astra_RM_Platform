#ifndef PLATFORM_DEVICE_INTERNAL_DEVICE_BACKEND_PROFILES_H
#define PLATFORM_DEVICE_INTERNAL_DEVICE_BACKEND_PROFILES_H

#include "../device_layer.h"

void platform_device_backend_bind_hw(platform_device_layer_t *layer);
void platform_device_backend_bind_sitl(platform_device_layer_t *layer);

#endif
