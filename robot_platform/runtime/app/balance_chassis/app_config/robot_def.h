#ifndef BALANCE_CHASSIS_APP_CONFIG_ROBOT_DEF_H
#define BALANCE_CHASSIS_APP_CONFIG_ROBOT_DEF_H

/**
 * @brief  Centralized robot parameter definitions
 *         All mechanical, motor, control, and topic struct definitions live here
 */

#include "stdint.h"

/* ======================== Mechanical params ======================== */

// 五连杆杆长 (单位: m)
#define LINK_L1     0.15f   // 前/后髋关节连杆 (AB, DE)
#define LINK_L2     0.272f  // 前大腿连杆 (BC)
#define LINK_L3     0.272f  // 后大腿连杆 (CD)
#define LINK_L4     0.15f   // 后髋关节连杆 (DE)
#define LINK_L5     0.15f   // 髋关节间距 (AE)

// 驱动轮
#define WHEEL_RADIUS        0.0675f // 轮子半径 (m)
#define WHEEL_GEAR_RATIO    15.0f   // M3508 减速比

// 关节电机零点偏移 (rad) —— 每台车装配后标定
#define JOINT0_OFFSET   (-0.024f)   // 右前关节
#define JOINT1_OFFSET   (-0.0531f)  // 右后关节
#define JOINT2_OFFSET   (-0.018f)   // 左前关节
#define JOINT3_OFFSET   (+0.074f)   // 左后关节

// theta 零点补偿 (rad) —— IMU 安装偏差
#define THETA_OFFSET_R  (-0.05f)    // 右腿
#define THETA_OFFSET_L  (+0.05f)    // 左腿

// 腿长范围 (m)
#define LEG_LENGTH_MIN  0.14f
#define LEG_LENGTH_MAX  0.35f

/* ======================== 电机配置 ======================== */

// DM4310 关节电机 (FDCAN1, MIT 模式)
#define JOINT_MOTOR_NUM     4
#define JOINT_MOTOR_ID      {1, 2, 3, 4}
#define JOINT_MOTOR_RX_ID   {0x11, 0x12, 0x13, 0x14}

// M3508 轮毂电机 (FDCAN2)
#define WHEEL_MOTOR_NUM     2
#define WHEEL_MOTOR_RX_ID   {0x201, 0x202}

// 电机力矩限幅
#define JOINT_TORQUE_MAX    2.8f
#define WHEEL_TORQUE_MAX    2.0f

// M3508 力矩→电流换算: I = T / Kt, current = I / 20A * 16384
// Kt ≈ 0.0236842, 合并系数 = 16384 / (20 * 0.0236842) ≈ 3458.84
#define M3508_TORQUE_TO_CURRENT     3458.84f
#define M3508_RPM_TO_RADS           (2.0f * 3.1415926f / 60.0f / WHEEL_GEAR_RATIO)

#define WHEEL_TORQUE_RATIO  1.0f
#define TURN_TORQUE_RATIO   0.8f

/* ======================== 控制参数 ======================== */

#define LEG_GRAVITY_COMP    10.4f

#define LEG_PID_KP      220.0f
#define LEG_PID_KI      0.02f
#define LEG_PID_KD      4200.0f
#define LEG_PID_MAX_OUT     60.0f
#define LEG_PID_MAX_IOUT    15.0f

#define TP_PID_KP       9.0f
#define TP_PID_KI       0.0f
#define TP_PID_KD       1.6f
#define TP_PID_MAX_OUT      2.0f
#define TP_PID_MAX_IOUT     0.0f

#define TURN_PID_KP     4.0f
#define TURN_PID_KI     0.0f
#define TURN_PID_KD     0.4f
#define TURN_PID_MAX_OUT    1.0f
#define TURN_PID_MAX_IOUT   0.0f

#define ROLL_PID_KP     2.0f
#define ROLL_PID_KI     0.0f
#define ROLL_PID_KD     1.0f
#define ROLL_PID_MAX_OUT    80.0f
#define ROLL_PID_MAX_IOUT   0.0f

#define PITCH_FALL_THRESHOLD    0.15f
#define PITCH_RECOVER_THRESHOLD 0.20f

#define FN_GROUND_THRESHOLD     10.0f
#define FN_WHEEL_GRAVITY        6.0f

/* ======================== 遥控器映射 ======================== */

#define RC_VX_MAX           6.5f
#define RC_TO_VX            (RC_VX_MAX / 660.0f)
#define RC_TO_TURN          0.0002f
#define RC_SPEED_SLOPE      0.1f

#define LEG_LENGTH_DEFAULT  0.18f

/* ======================== LQR 增益矩阵 ======================== */

#define LQR_K_MATRIX { \
    -12.4527f, -1.0044f, -12.0831f, -9.3426f, 13.4064f, 2.0690f, \
     19.2682f,  2.0679f,  26.8886f, 17.6150f, 33.3348f, 3.2053f  \
}

/* ======================== Pub-Sub topic data structures ======================== */

typedef struct {
    float pitch;
    float roll;
    float yaw_total;
    float gyro[3];
    float accel_b[3];
    uint8_t ready;
} INS_Data_t;

typedef struct {
    int16_t ch[4];
    uint8_t sw[2];
    uint8_t online;
    uint8_t reserved;
} RC_Data_t;

typedef struct {
    float vx_cmd;
    float turn_cmd;
    float leg_set;
    uint8_t start_flag;
    uint8_t jump_flag;
    uint8_t recover_flag;
} Chassis_Cmd_t;

typedef struct {
    float v_filter;
    float x_filter;
    float x_set;
    float total_yaw;
    float roll;
    float turn_set;
} Chassis_State_t;

typedef struct {
    float v_filter;
    float x_filter;
} Chassis_Observe_t;

typedef struct {
    float joint_torque[2];
    float wheel_torque;
    int16_t wheel_current;
    float leg_length;
} Leg_Output_t;

typedef struct {
    float joint_torque[4];
    int16_t wheel_current[2];
    uint8_t start_flag;
    uint8_t reserved[3];
} Actuator_Cmd_t;

typedef struct {
    float joint_pos[4];
    float wheel_speed[2];
    float wheel_angle[2];
    uint8_t ready;
    uint8_t reserved[3];
} Actuator_Feedback_t;

#endif
