#ifndef DEVICE_COMMAND_H
#define DEVICE_COMMAND_H

#include "runtime_contract_common.h"

typedef struct {
  uint8_t device_id;
  platform_motor_kind_t kind;
  platform_motor_control_mode_t control_mode;
  float torque_target;
  float velocity_target;
  float position_target;
  float current_target;
  float kp;
  float kd;
  bool valid;
} platform_motor_device_command_t;

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  platform_motor_device_command_t joints[PLATFORM_JOINT_MOTOR_COUNT];
  platform_motor_device_command_t wheels[PLATFORM_WHEEL_MOTOR_COUNT];
  uint32_t backend_flags;
} platform_device_command_t;

#endif
