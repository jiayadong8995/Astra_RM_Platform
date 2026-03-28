#ifndef DM4310_DRV_H
#define DM4310_DRV_H

#include "can_bsp.h"
#include "fdcan.h"
#include "main.h"

#define FILTER_COEFFICIENT 0.9f
#define MOTOR_ECD_TO_RAD  0.000766990394f

#define MIT_MODE 0x000
#define POS_MODE 0x100
#define SPEED_MODE 0x200

#define P_MIN -12.5f
#define P_MAX 12.5f
#define V_MIN -30.0f
#define V_MAX 30.0f
#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f
#define T_MIN -10.0f
#define T_MAX 10.0f

#define P_MIN2 -12.0f
#define P_MAX2 12.0f
#define V_MIN2 -45.0f
#define V_MAX2 45.0f
#define KP_MIN2 0.0f
#define KP_MAX2 500.0f
#define KD_MIN2 0.0f
#define KD_MAX2 5.0f
#define T_MIN2 -18.0f
#define T_MAX2 18.0f

typedef struct
{
    uint16_t id;
    uint16_t state;
    int p_int;
    int v_int;
    int t_int;
    int kp_int;
    int kd_int;
    float pos;
    float vel;
    float tor;
    float Kp;
    float Kd;
    float Tmos;
    float Tcoil;
} motor_fbpara_t;

typedef struct
{
    uint16_t ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;
    int16_t last_ecd;
    float total_angle;
    int16_t last_encode;
} chassis_motor_measure_t;

typedef struct
{
    const chassis_motor_measure_t *chassis_motor_measure;
    float speed;
    float w_speed;
    float torque;
    float torque_set;
    int16_t give_current;
    float chassis_x;
} chassis_motor_t;

typedef struct
{
    float speed;
    float filtered_speed;
    float angle;
    uint32_t last_time;
} MotorData;

typedef struct
{
    uint16_t mode;
    motor_fbpara_t para;
} Joint_Motor_t;

typedef struct
{
    uint16_t mode;
    float wheel_T;
    motor_fbpara_t para;
} Wheel_Motor_t;

void dm4310_fbdata(Joint_Motor_t *motor, uint8_t *rx_data, uint32_t data_len);
int enable_motor_mode(hcan_t *hcan, uint16_t motor_id, uint16_t mode_id);
void disable_motor_mode(hcan_t *hcan, uint16_t motor_id, uint16_t mode_id);
void mit_ctrl(hcan_t *hcan, uint16_t motor_id, float pos, float vel, float kp, float kd, float torq);
void pos_speed_ctrl(hcan_t *hcan, uint16_t motor_id, float pos, float vel);
void speed_ctrl(hcan_t *hcan, uint16_t motor_id, float vel);
void mit_ctrl2(hcan_t *hcan, uint16_t motor_id, float pos, float vel, float kp, float kd, float torq);
void joint_motor_init(Joint_Motor_t *motor, uint16_t id, uint16_t mode);
void wheel_motor_init(Wheel_Motor_t *motor, uint16_t id, uint16_t mode);
float Hex_To_Float(uint32_t *Byte, int num);
uint32_t FloatTohex(float HEX);
float uint_to_float(int x_int, float x_min, float x_max, int bits);
int float_to_uint(float x_float, float x_min, float x_max, int bits);
void CAN_cmd_chassis(hcan_t *hcan, int16_t motor1, int16_t motor2, int16_t rev1, int16_t rev2);
void get_motor_measure(chassis_motor_measure_t *ptr, uint8_t *data, uint32_t data_len);
void get_total_angle(chassis_motor_measure_t *p);
chassis_motor_measure_t *get_chassis_motor_measure_point(uint8_t i);
float motor_speed_to_angle(MotorData *motors, float *speed);
void DM_motor_zeroset(hcan_t *hcan, uint16_t motor_id);

#endif
