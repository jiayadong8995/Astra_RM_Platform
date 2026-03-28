#ifndef ROBOT_DEF_H
#define ROBOT_DEF_H

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
#define JOINT_MOTOR_ID      {1, 2, 3, 4}       // 发送 ID
#define JOINT_MOTOR_RX_ID   {0x11, 0x12, 0x13, 0x14} // 接收 ID

// M3508 轮毂电机 (FDCAN2)
#define WHEEL_MOTOR_NUM     2
#define WHEEL_MOTOR_RX_ID   {0x201, 0x202}

// 电机力矩限幅
#define JOINT_TORQUE_MAX    2.8f    // 关节电机最大力矩 (Nm)
#define WHEEL_TORQUE_MAX    2.0f    // 轮毂电机最大力矩 (Nm)

// M3508 力矩→电流换算: I = T / Kt, current = I / 20A * 16384
// Kt ≈ 0.0236842, 合并系数 = 16384 / (20 * 0.0236842) ≈ 3458.84
#define M3508_TORQUE_TO_CURRENT     3458.84f
#define M3508_RPM_TO_RADS           (2.0f * 3.1415926f / 60.0f / WHEEL_GEAR_RATIO)

#define WHEEL_TORQUE_RATIO  1.0f    // 轮毂力矩缩放系数
#define TURN_TORQUE_RATIO   0.8f    // 转向力矩混合系数

/* ======================== 控制参数 ======================== */

// 重力前馈
#define LEG_GRAVITY_COMP    10.4f   // 腿部重力补偿 (N)

// 腿长 PD
#define LEG_PID_KP      220.0f
#define LEG_PID_KI      0.02f
#define LEG_PID_KD      4200.0f
#define LEG_PID_MAX_OUT     60.0f
#define LEG_PID_MAX_IOUT    15.0f

// 防劈叉 PD
#define TP_PID_KP       9.0f
#define TP_PID_KI       0.0f
#define TP_PID_KD       1.6f
#define TP_PID_MAX_OUT      2.0f
#define TP_PID_MAX_IOUT     0.0f

// 转向 PD
#define TURN_PID_KP     4.0f
#define TURN_PID_KI     0.0f
#define TURN_PID_KD     0.4f
#define TURN_PID_MAX_OUT    1.0f
#define TURN_PID_MAX_IOUT   0.0f

// 横滚补偿 PD
#define ROLL_PID_KP     2.0f
#define ROLL_PID_KI     0.0f
#define ROLL_PID_KD     1.0f
#define ROLL_PID_MAX_OUT    80.0f
#define ROLL_PID_MAX_IOUT   0.0f

// 倒地判定阈值 (rad)
#define PITCH_FALL_THRESHOLD    0.15f
#define PITCH_RECOVER_THRESHOLD 0.20f

// 离地检测支持力阈值 (N)
#define FN_GROUND_THRESHOLD     10.0f
#define FN_WHEEL_GRAVITY        6.0f    // 轮组重力近似

/* ======================== 遥控器映射 ======================== */

#define RC_VX_MAX           6.5f                        // 最大速度 (m/s)
#define RC_TO_VX            (RC_VX_MAX / 660.0f)        // 摇杆→速度
#define RC_TO_TURN          0.0002f                     // 摇杆→转向增量
#define RC_SPEED_SLOPE      0.1f                        // 速度斜坡加速度

#define LEG_LENGTH_DEFAULT  0.18f   // 默认腿长 (m)

/* ======================== 任务周期 ======================== */

#define CHASSIS_TASK_PERIOD     2   // ms
#define OBSERVE_TASK_PERIOD     3   // ms
#define REMOTE_TASK_PERIOD      10  // ms

/* ======================== LQR 增益矩阵 ======================== */

// 状态向量: [theta, d_theta, x, dx, pitch, d_pitch]
// 输出: [wheel_torque, hip_torque]
// 行1: 轮毂力矩, 行2: 髋关节力矩
#define LQR_K_MATRIX { \
    -12.4527f, -1.0044f, -12.0831f, -9.3426f, 13.4064f, 2.0690f, \
     19.2682f,  2.0679f,  26.8886f, 17.6150f, 33.3348f, 3.2053f  \
}

/* ======================== Pub-Sub topic data structures ======================== */

/**
 * @brief  IMU data published by INS_task
 *         Topic name: "ins_data"
 */
typedef struct {
    float pitch;        // rad
    float roll;         // rad
    float yaw_total;    // rad, continuous
    float gyro[3];      // rad/s  [x=roll, y=pitch_rate, z=yaw_rate]
    float accel_b[3];   // m/s^2, body frame
    uint8_t ready;      // 1 after convergence
} INS_Data_t;

/**
 * @brief  Raw remote/operator input normalized by BSP-side bridge
 *         Topic name: "rc_data"
 */
typedef struct {
    int16_t ch[4];      // RC channels after SBUS normalization
    uint8_t sw[2];      // switch positions
    uint8_t online;     // 1=valid frame stream, 0=offline/error
    uint8_t reserved;
} RC_Data_t;

/**
 * @brief  Chassis command published by remote_task
 *         Topic name: "chassis_cmd"
 */
typedef struct {
    float vx_cmd;       // m/s, forward speed command
    float turn_cmd;     // rad, accumulated yaw setpoint
    float leg_set;      // m, desired leg length
    uint8_t start_flag; // 1=enabled, 0=disabled
    uint8_t jump_flag;  // 1=jump requested
    uint8_t recover_flag;
} Chassis_Cmd_t;

/**
 * @brief  Chassis state published by chassis_task / observe_task
 *         Topic name: "chassis_state"
 */
typedef struct {
    float v_filter;     // m/s, estimated body velocity
    float x_filter;     // m, estimated position
    float x_set;        // m, position setpoint
    float total_yaw;    // rad
    float roll;         // rad
    float turn_set;     // rad, yaw setpoint
} Chassis_State_t;

/**
 * @brief  Chassis observer output published by observe_task
 *         Topic name: "chassis_observe"
 */
typedef struct {
    float v_filter;     // m/s, estimated body velocity
    float x_filter;     // m, estimated position
} Chassis_Observe_t;

/**
 * @brief  Single leg output from chassis_task to motor_control_task
 *         Topic name: "leg_right" / "leg_left"
 */
typedef struct {
    float joint_torque[2]; // Nm, [front, back] joint motor torque
    float wheel_torque;    // Nm, wheel motor torque
    int16_t wheel_current; // M3508 current value
    float leg_length;      // m, current leg length estimate
} Leg_Output_t;

/**
 * @brief  Consolidated actuator command published by chassis_task
 *         Topic name: "actuator_cmd"
 */
typedef struct {
    float joint_torque[4]; // Nm, [r_front, r_back, l_front, l_back]
    int16_t wheel_current[2]; // [left, right] wheel current command
    uint8_t start_flag;    // 1=enable output, 0=hold zero
    uint8_t reserved[3];
} Actuator_Cmd_t;

#endif // ROBOT_DEF_H
