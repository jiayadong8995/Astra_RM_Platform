#ifndef PLATFORM_REMOTE_DEVICE_H
#define PLATFORM_REMOTE_DEVICE_H

#include "../../device_types.h"
#include "../../../control/contracts/device_input.h"

typedef struct platform_remote_device platform_remote_device_t;

typedef struct {
  platform_device_result_t (*init)(platform_remote_device_t *device);
  platform_device_result_t (*read_input)(platform_remote_device_t *device, platform_rc_input_t *input);
} platform_remote_device_ops_t;

struct platform_remote_device {
  const char *name;
  void *context;
  platform_device_stamp_t stamp;
  platform_remote_device_ops_t ops;
};

#endif
