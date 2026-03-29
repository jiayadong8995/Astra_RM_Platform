#ifndef BALANCE_CHASSIS_APP_CONFIG_RUNTIME_STATE_H
#define BALANCE_CHASSIS_APP_CONFIG_RUNTIME_STATE_H

#include "main.h"
#include "../../../device/actuator/motor/dm4310/dm4310_drv.h"
#include "../../../device/imu/bmi088/BMI088driver.h"
#include "QuaternionEKF.h"

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
} INS_t;

typedef struct
{
    uint8_t flag;

    float scale[3];

    float Yaw;
    float Pitch;
    float Roll;
} IMU_Param_t;

typedef struct
{
    chassis_motor_t wheel_motor[2];

    float v_set;
    float x_set;
    float turn_set;
    float roll_set;
    float leg_set;
    float last_leg_set;

    float v_filter;
    float x_filter;

    float myPithR;
    float myPithGyroR;
    float myPithL;
    float myPithGyroL;
    float roll;
    float total_yaw;
    float theta_err;

    float turn_T;
    float roll_f0;
    float leg_tp;

    uint8_t start_flag;

    uint8_t jump_flag;
    float jump_leg;
    uint32_t jump_time;
    uint8_t jump_status;

    uint8_t last_recover_flag;
    uint8_t recover_flag;
    uint8_t text_jump_true;
} chassis_t;

#endif
