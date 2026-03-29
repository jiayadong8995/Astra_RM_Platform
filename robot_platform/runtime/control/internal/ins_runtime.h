#ifndef PLATFORM_CONTROL_INTERNAL_INS_RUNTIME_H
#define PLATFORM_CONTROL_INTERNAL_INS_RUNTIME_H

#include <stdint.h>

#define PLATFORM_AXIS_X 0
#define PLATFORM_AXIS_Y 1
#define PLATFORM_AXIS_Z 2

#define PITCH_OFFSET   -0.0024f
#define ROLL_OFFSET    0.0333f

typedef struct
{
    float quaternion[4];
    float gyro[3];
    float accel[3];
    float body_accel[3];
    float world_accel[3];
    float accel_lpf;
} platform_ins_sensor_state_t;

typedef struct
{
    float roll;
    float pitch;
    float yaw;
    float yaw_total;
    float yaw_last;
    int32_t yaw_turn_count;
} platform_ins_attitude_state_t;

typedef struct
{
    uint8_t ready;
} platform_ins_health_state_t;

typedef struct
{
    platform_ins_sensor_state_t sensor;
    platform_ins_attitude_state_t attitude;
    platform_ins_health_state_t health;
} platform_ins_runtime_t;

#endif
