#ifndef PLATFORM_MOTOR_DEVICE_H
#define PLATFORM_MOTOR_DEVICE_H

#include "../../../device_types.h"
#include "../../../../control/contracts/actuator_command.h"
#include "../../../../control/contracts/device_feedback.h"

typedef struct platform_motor_device platform_motor_device_t;

typedef struct {
  platform_device_result_t (*init)(platform_motor_device_t *device);
  platform_device_result_t (*write_motor_command)(platform_motor_device_t *device,
                                                  const platform_motor_command_set_t *command_set);
  platform_device_result_t (*read_motor_feedback)(platform_motor_device_t *device,
                                                  platform_device_feedback_t *feedback);
} platform_motor_device_ops_t;

struct platform_motor_device {
  const char *name;
  void *context;
  platform_device_stamp_t stamp;
  platform_motor_device_ops_t ops;
};

#endif
