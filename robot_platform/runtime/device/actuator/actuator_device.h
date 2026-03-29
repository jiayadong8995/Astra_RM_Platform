#ifndef PLATFORM_ACTUATOR_DEVICE_H
#define PLATFORM_ACTUATOR_DEVICE_H

#include "../device_types.h"
#include "../../control/contracts/device_command.h"
#include "../../control/contracts/device_feedback.h"

typedef struct platform_actuator_device platform_actuator_device_t;

typedef struct {
  platform_device_result_t (*init)(platform_actuator_device_t *device);
  platform_device_result_t (*write_command)(platform_actuator_device_t *device,
                                            const platform_device_command_t *command);
  platform_device_result_t (*read_feedback)(platform_actuator_device_t *device,
                                            platform_device_feedback_t *feedback);
} platform_actuator_device_ops_t;

struct platform_actuator_device {
  const char *name;
  void *context;
  platform_device_stamp_t stamp;
  platform_actuator_device_ops_t ops;
};

#endif
