#include "dm4310_drv.h"

void joint_motor_init(Joint_Motor_t *motor, uint16_t id, uint16_t mode) {
    (void)motor; (void)id; (void)mode;
}

void wheel_motor_init(Wheel_Motor_t *motor, uint16_t id, uint16_t mode) {
    (void)motor; (void)id; (void)mode;
}

int enable_motor_mode(hcan_t *hcan, uint16_t motor_id, uint16_t mode_id) {
    (void)hcan; (void)motor_id; (void)mode_id;
    return 0;
}

void disable_motor_mode(hcan_t *hcan, uint16_t motor_id, uint16_t mode_id) {
    (void)hcan; (void)motor_id; (void)mode_id;
}

void mit_ctrl(hcan_t *hcan, uint16_t motor_id, float pos, float vel, float kp, float kd, float torq) {
    (void)hcan; (void)motor_id; (void)pos; (void)vel; (void)kp; (void)kd; (void)torq;
}

void dm4310_fbdata(Joint_Motor_t *motor, uint8_t *rx_data, uint32_t data_len) {
    (void)motor; (void)rx_data; (void)data_len;
}

void CAN_cmd_chassis(hcan_t *hcan, int16_t motor1, int16_t motor2, int16_t rev1, int16_t rev2) {
    (void)hcan; (void)motor1; (void)motor2; (void)rev1; (void)rev2;
}

void get_motor_measure(chassis_motor_measure_t *ptr, uint8_t *data, uint32_t data_len) {
    (void)ptr; (void)data; (void)data_len;
}

void DM_motor_zeroset(hcan_t *hcan, uint16_t motor_id) {
    (void)hcan; (void)motor_id;
}

const chassis_motor_measure_t *get_chassis_motor_measure_point(uint8_t i) {
    static chassis_motor_measure_t dummy;
    (void)i;
    return &dummy;
}

