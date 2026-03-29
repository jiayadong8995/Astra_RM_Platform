#ifndef PLATFORM_CONTROL_INTERNAL_INS_RUNTIME_H
#define PLATFORM_CONTROL_INTERNAL_INS_RUNTIME_H

#include <stdint.h>

#define X 0
#define Y 1
#define Z 2

#define PITCH_OFFSET   -0.0024f
#define ROLL_OFFSET    0.0333f

typedef struct
{
    float q[4];

    float Gyro[3];
    float Accel[3];
    float MotionAccel_b[3];
    float MotionAccel_n[3];

    float AccelLPF;

    float xn[3];
    float yn[3];
    float zn[3];

    float atanxz;
    float atanyz;

    float Roll;
    float Pitch;
    float Yaw;
    float YawTotalAngle;
    float YawAngleLast;
    float YawRoundCount;

    float v_n;
    float x_n;

    uint8_t ins_flag;
} platform_ins_runtime_t;

#endif
