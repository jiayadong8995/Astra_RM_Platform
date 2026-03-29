#ifndef PLATFORM_CONTROL_INTERNAL_BALANCE_RUNTIME_H
#define PLATFORM_CONTROL_INTERNAL_BALANCE_RUNTIME_H

#include <stdint.h>

typedef struct
{
    float speed;
    float w_speed;
    float torque;
    float torque_set;
    int16_t give_current;
    float chassis_x;
} platform_wheel_runtime_t;

typedef struct
{
    float velocity_target;
    float position_target;
    float yaw_target;
    float roll_target;
    float leg_length_target;
    float previous_leg_length_target;
} platform_balance_target_state_t;

typedef struct
{
    float velocity;
    float position;
} platform_balance_observe_state_t;

typedef struct
{
    float right_pitch;
    float right_pitch_rate;
    float left_pitch;
    float left_pitch_rate;
    float roll;
    float yaw_total;
    float leg_theta_error;
} platform_balance_attitude_state_t;

typedef struct
{
    float turn_torque;
    float roll_force;
    float leg_pitch;
} platform_balance_compensation_state_t;

typedef struct
{
    uint8_t start_enabled;
    uint8_t jump_requested;
    float jump_leg_length;
    uint32_t jump_elapsed_ticks;
    uint8_t jump_phase;
    uint8_t last_recover_requested;
    uint8_t recover_requested;
    uint8_t grounded;
} platform_balance_mode_state_t;

typedef struct
{
    platform_wheel_runtime_t wheel_motor[2];
    platform_balance_target_state_t target;
    platform_balance_observe_state_t observe;
    platform_balance_attitude_state_t attitude;
    platform_balance_compensation_state_t compensation;
    platform_balance_mode_state_t mode;
} platform_balance_runtime_t;

#endif
