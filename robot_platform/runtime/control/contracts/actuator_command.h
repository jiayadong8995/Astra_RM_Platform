#ifndef ACTUATOR_COMMAND_H
#define ACTUATOR_COMMAND_H

#include "runtime_contract_common.h"

typedef struct {
  platform_motor_control_mode_t control_mode;
  float torque_target;
  float velocity_target;
  float position_target;
  float current_target;
  float kp;
  float kd;
  bool valid;
} platform_motor_command_t;

typedef struct {
  platform_motor_command_t left_leg_joint[2];
  platform_motor_command_t right_leg_joint[2];
  platform_motor_command_t left_wheel;
  platform_motor_command_t right_wheel;
} platform_motor_command_set_t;

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  bool start;
  bool control_enable;
  bool actuator_enable;
  platform_motor_command_set_t motors;
} platform_actuator_command_t;

#endif
