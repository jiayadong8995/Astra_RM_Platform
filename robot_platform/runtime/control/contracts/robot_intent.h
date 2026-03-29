#ifndef ROBOT_INTENT_H
#define ROBOT_INTENT_H

#include "runtime_contract_common.h"

typedef struct {
  float vx;
  float x;
  float yaw_target;
  float yaw_rate;
  bool yaw_hold;
  platform_velocity_frame_t velocity_frame;
} platform_motion_target_t;

typedef struct {
  float leg_length;
  float body_pitch_ref;
  float stance_height;
} platform_posture_target_t;

typedef struct {
  bool jump_request;
  bool recover_request;
  bool stand_request;
  bool emergency_stop;
} platform_behavior_request_t;

typedef struct {
  bool start;
  bool control_enable;
  bool actuator_enable;
} platform_enable_state_t;

typedef struct {
  uint32_t timestamp_us;
  uint32_t sequence;
  platform_robot_mode_t mode;
  platform_motion_target_t motion_target;
  platform_posture_target_t posture_target;
  platform_behavior_request_t behavior_request;
  platform_enable_state_t enable;
} platform_robot_intent_t;

#endif
