#ifndef DEVICE_INPUT_H
#define DEVICE_INPUT_H

#include "runtime_contract_common.h"

typedef struct {
  float accel[3];
  float gyro[3];
  float temperature;
  uint32_t sample_time_us;
  bool valid;
} platform_imu_sample_t;

typedef struct {
  int16_t channels[PLATFORM_RC_CHANNEL_COUNT];
  uint8_t switches[PLATFORM_RC_SWITCH_COUNT];
  int16_t mouse_x;
  int16_t mouse_y;
  int16_t mouse_z;
  uint8_t mouse_left;
  uint8_t mouse_right;
  uint16_t keyboard_mask;
  uint8_t source;
  uint32_t sample_time_us;
  bool valid;
} platform_rc_input_t;

typedef struct {
  uint8_t id;
  platform_motor_kind_t kind;
  float position;
  float velocity;
  float torque_est;
  float temperature;
  uint32_t sample_time_us;
  bool online;
} platform_motor_feedback_t;

typedef struct {
  platform_motor_feedback_t joints[PLATFORM_JOINT_MOTOR_COUNT];
  platform_motor_feedback_t wheels[PLATFORM_WHEEL_MOTOR_COUNT];
  uint32_t sample_time_us;
  bool valid;
} platform_actuator_feedback_snapshot_t;

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  platform_imu_sample_t imu_sample;
  platform_rc_input_t rc_input;
  platform_actuator_feedback_snapshot_t actuator_feedback;
} platform_device_input_t;

#endif
