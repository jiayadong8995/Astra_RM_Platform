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
    platform_wheel_runtime_t wheel_motor[2];

    float v_set;
    float x_set;
    float turn_set;
    float roll_set;
    float leg_set;
    float last_leg_set;

    float v_filter;
    float x_filter;

    float right_body_pitch;
    float right_body_pitch_rate;
    float left_body_pitch;
    float left_body_pitch_rate;
    float roll;
    float total_yaw;
    float theta_err;

    float turn_torque_compensation;
    float roll_force_compensation;
    float leg_pitch_compensation;

    uint8_t start_flag;

    uint8_t jump_flag;
    float jump_leg;
    uint32_t jump_time;
    uint8_t jump_phase;

    uint8_t last_recover_flag;
    uint8_t recover_flag;
    uint8_t grounded_flag;
} platform_balance_runtime_t;

#endif
