#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

#include "runtime_contract_common.h"

typedef struct {
  float roll;
  float pitch;
  float yaw;
  float gyro[3];
  float accel[3];
  bool orientation_valid;
} platform_body_state_t;

typedef struct {
  float x;
  float v;
  float vx;
  float vy;
  float yaw_total;
  float turn_rate;
  bool state_valid;
} platform_chassis_state_t;

typedef struct {
  float length;
  float leg_angle;
  float joint_pos[2];
  float joint_vel[2];
  float joint_torque_est[2];
  uint8_t support_phase;
} platform_single_leg_state_t;

typedef struct {
  platform_single_leg_state_t left;
  platform_single_leg_state_t right;
} platform_leg_state_t;

typedef struct {
  float speed;
  float position;
  float torque_est;
  bool online;
} platform_single_wheel_state_t;

typedef struct {
  platform_single_wheel_state_t left;
  platform_single_wheel_state_t right;
} platform_wheel_state_t;

typedef struct {
  bool grounded;
  bool left_support;
  bool right_support;
  float land_confidence;
} platform_contact_state_t;

typedef struct {
  bool imu_ok;
  bool remote_ok;
  bool actuator_ok;
  bool state_valid;
  bool degraded_mode;
} platform_health_state_t;

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  platform_body_state_t body;
  platform_chassis_state_t chassis;
  platform_leg_state_t legs;
  platform_wheel_state_t wheels;
  platform_contact_state_t contact;
  platform_health_state_t health;
} platform_robot_state_t;

#endif
