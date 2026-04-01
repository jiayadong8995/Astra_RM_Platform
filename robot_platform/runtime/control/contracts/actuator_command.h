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
  platform_motor_command_t joints[PLATFORM_JOINT_MOTOR_COUNT];
  platform_motor_command_t wheels[PLATFORM_WHEEL_MOTOR_COUNT];
} platform_motor_command_set_t;

enum {
  PLATFORM_JOINT_LEFT_FRONT  = 0,
  PLATFORM_JOINT_LEFT_REAR   = 1,
  PLATFORM_JOINT_RIGHT_FRONT = 2,
  PLATFORM_JOINT_RIGHT_REAR  = 3,
};

enum {
  PLATFORM_WHEEL_LEFT  = 0,
  PLATFORM_WHEEL_RIGHT = 1,
};

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  bool start;
  bool control_enable;
  bool actuator_enable;
  platform_motor_command_set_t motors;
} platform_actuator_command_t;

#endif
