#include "dm4310_drv.h"

#include "arm_math.h"
#include "chassis_state_bridge.h"

static chassis_motor_measure_t chassis_motor[2];

#define ABS(x) ( (x > 0) ? (x) : (-(x)) )

void get_total_angle(chassis_motor_measure_t *p)
{
    if (platform_consume_begin_flag() == 1U)
    {
        p->total_angle = 0.0f;
    }

    int res1, res2, delta;

    if (p->ecd < p->last_ecd)
    {
        res1 = p->ecd + 8192 - p->last_ecd;
        res2 = p->ecd - p->last_ecd;
    }
    else
    {
        res1 = p->ecd - 8192 - p->last_ecd;
        res2 = p->ecd - p->last_ecd;
    }

    if (ABS(res1) < ABS(res2))
    {
        delta = res1;
    }
    else
    {
        delta = res2;
    }

    p->total_angle += delta * MOTOR_ECD_TO_RAD;
    p->last_encode = p->ecd;
}

float Hex_To_Float(uint32_t *Byte, int num)
{
    (void) num;
    return *((float *)Byte);
}

uint32_t FloatTohex(float HEX)
{
    return *(uint32_t *)&HEX;
}

int float_to_uint(float x_float, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return (int)((x_float - offset) * ((float)((1 << bits) - 1)) / span);
}

float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + offset;
}

void joint_motor_init(Joint_Motor_t *motor, uint16_t id, uint16_t mode)
{
    motor->mode = mode;
    motor->para.id = id;
}

void wheel_motor_init(Wheel_Motor_t *motor, uint16_t id, uint16_t mode)
{
    motor->mode = mode;
    motor->para.id = id;
}

void dm4310_fbdata(Joint_Motor_t *motor, uint8_t *rx_data, uint32_t data_len)
{
    if (data_len == FDCAN_DLC_BYTES_8)
    {
        motor->para.id = (rx_data[0]) & 0x0F;
        motor->para.state = (rx_data[0]) >> 4;
        motor->para.p_int = (rx_data[1] << 8) | rx_data[2];
        motor->para.v_int = (rx_data[3] << 4) | (rx_data[4] >> 4);
        motor->para.t_int = ((rx_data[4] & 0xF) << 8) | rx_data[5];
        motor->para.pos = uint_to_float(motor->para.p_int, P_MIN, P_MAX, 16);
        motor->para.vel = uint_to_float(motor->para.v_int, V_MIN, V_MAX, 12);
        motor->para.tor = uint_to_float(motor->para.t_int, T_MIN, T_MAX, 12);
        motor->para.Tmos = (float)(rx_data[6]);
        motor->para.Tcoil = (float)(rx_data[7]);
    }
}

int enable_motor_mode(hcan_t *hcan, uint16_t motor_id, uint16_t mode_id)
{
    uint8_t data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
    uint16_t id = motor_id + mode_id;
    return canx_send_data(hcan, id, data, 8);
}

void disable_motor_mode(hcan_t *hcan, uint16_t motor_id, uint16_t mode_id)
{
    uint8_t data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD};
    uint16_t id = motor_id + mode_id;
    canx_send_data(hcan, id, data, 8);
}

void mit_ctrl(hcan_t *hcan, uint16_t motor_id, float pos, float vel, float kp, float kd, float torq)
{
    uint8_t data[8];
    uint16_t pos_tmp, vel_tmp, kp_tmp, kd_tmp, tor_tmp;
    uint16_t id = motor_id + MIT_MODE;

    pos_tmp = float_to_uint(pos, P_MIN, P_MAX, 16);
    vel_tmp = float_to_uint(vel, V_MIN, V_MAX, 12);
    kp_tmp = float_to_uint(kp, KP_MIN, KP_MAX, 12);
    kd_tmp = float_to_uint(kd, KD_MIN, KD_MAX, 12);
    tor_tmp = float_to_uint(torq, T_MIN, T_MAX, 12);

    data[0] = (pos_tmp >> 8);
    data[1] = pos_tmp;
    data[2] = (vel_tmp >> 4);
    data[3] = ((vel_tmp & 0xF) << 4) | (kp_tmp >> 8);
    data[4] = kp_tmp;
    data[5] = (kd_tmp >> 4);
    data[6] = ((kd_tmp & 0xF) << 4) | (tor_tmp >> 8);
    data[7] = tor_tmp;

    canx_send_data(hcan, id, data, 8);
}

void pos_speed_ctrl(hcan_t *hcan, uint16_t motor_id, float pos, float vel)
{
    uint16_t id = motor_id + POS_MODE;
    uint8_t *pbuf = (uint8_t *)&pos;
    uint8_t *vbuf = (uint8_t *)&vel;
    uint8_t data[8];

    data[0] = *pbuf;
    data[1] = *(pbuf + 1);
    data[2] = *(pbuf + 2);
    data[3] = *(pbuf + 3);
    data[4] = *vbuf;
    data[5] = *(vbuf + 1);
    data[6] = *(vbuf + 2);
    data[7] = *(vbuf + 3);

    canx_send_data(hcan, id, data, 8);
}

void speed_ctrl(hcan_t *hcan, uint16_t motor_id, float vel)
{
    uint16_t id = motor_id + SPEED_MODE;
    uint8_t *vbuf = (uint8_t *)&vel;
    uint8_t data[4];

    data[0] = *vbuf;
    data[1] = *(vbuf + 1);
    data[2] = *(vbuf + 2);
    data[3] = *(vbuf + 3);

    canx_send_data(hcan, id, data, 4);
}

void mit_ctrl2(hcan_t *hcan, uint16_t motor_id, float pos, float vel, float kp, float kd, float torq)
{
    uint8_t data[8];
    uint16_t pos_tmp, vel_tmp, kp_tmp, kd_tmp, tor_tmp;
    uint16_t id = motor_id + MIT_MODE;

    pos_tmp = float_to_uint(pos, P_MIN2, P_MAX2, 16);
    vel_tmp = float_to_uint(vel, V_MIN2, V_MAX2, 12);
    kp_tmp = float_to_uint(kp, KP_MIN2, KP_MAX2, 12);
    kd_tmp = float_to_uint(kd, KD_MIN2, KD_MAX2, 12);
    tor_tmp = float_to_uint(torq, T_MIN2, T_MAX2, 12);

    data[0] = (pos_tmp >> 8);
    data[1] = pos_tmp;
    data[2] = (vel_tmp >> 4);
    data[3] = ((vel_tmp & 0xF) << 4) | (kp_tmp >> 8);
    data[4] = kp_tmp;
    data[5] = (kd_tmp >> 4);
    data[6] = ((kd_tmp & 0xF) << 4) | (tor_tmp >> 8);
    data[7] = tor_tmp;

    canx_send_data(hcan, id, data, 8);
}

void CAN_cmd_chassis(hcan_t *hcan, int16_t motor1, int16_t motor2, int16_t rev1, int16_t rev2)
{
    (void) rev1;
    (void) rev2;
    uint8_t data[8] = {0};
    uint16_t Motor_All_Id = 0x200;
    data[0] = (motor1 >> 8) & 0xFF;
    data[1] = motor1 & 0xFF;
    data[2] = (motor2 >> 8) & 0xFF;
    data[3] = motor2 & 0xFF;

    canx_send_data(hcan, Motor_All_Id, data, 8);
}

void get_motor_measure(chassis_motor_measure_t *ptr, uint8_t *data, uint32_t data_len)
{
    (void) data_len;
    ptr->last_ecd = ptr->ecd;
    ptr->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);
    ptr->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]);
    ptr->given_current = (uint16_t)((data)[4] << 8 | (data)[5]);
    ptr->temperate = (data)[6];
    get_total_angle(ptr);
}

chassis_motor_measure_t *get_chassis_motor_measure_point(uint8_t i)
{
    return &chassis_motor[i];
}

float motor_speed_to_angle(MotorData *motors, float *speed)
{
    int32_t now = HAL_GetTick();
    motors->speed = *speed;
    motors->filtered_speed = motors->filtered_speed * (1 - FILTER_COEFFICIENT) + motors->speed * FILTER_COEFFICIENT;
    float delta_time = (now - motors->last_time) / 1000.0f;
    motors->angle += motors->filtered_speed * delta_time;
    motors->last_time = now;
    return motors->angle;
}

void DM_motor_zeroset(hcan_t *hcan, uint16_t motor_id)
{
    uint8_t data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE};
    canx_send_data(hcan, motor_id, data, 8);
}
