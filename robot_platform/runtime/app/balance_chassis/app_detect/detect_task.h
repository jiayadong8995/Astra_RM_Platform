#ifndef BALANCE_CHASSIS_DETECT_TASK_H
#define BALANCE_CHASSIS_DETECT_TASK_H

#include <stdint.h>
#include <stdbool.h>

#define DETECT_TASK_INIT_TIME  57U
#define DETECT_CONTROL_TIME    10U

enum errorList
{
    DBUS_TOE = 0,
    JOINT_MOTOR1_TOE,
    JOINT_MOTOR2_TOE,
    JOINT_MOTOR3_TOE,
    JOINT_MOTOR4_TOE,
    WHEEL_MOTOR1_TOE,
    WHEEL_MOTOR2_TOE,
    IMU_ACCEL_TOE,
    IMU_GYRO_TOE,
    ERROR_LIST_LENGTH,
};

typedef uint8_t (*data_is_error_fn_t)(void);
typedef void (*solve_lost_fn_t)(void);
typedef void (*solve_data_error_fn_t)(void);

typedef struct
{
    uint32_t new_time;
    uint32_t last_time;
    uint32_t lost_time;
    uint32_t work_time;
    uint16_t set_offline_time;
    uint16_t set_online_time;
    uint8_t enable;
    uint8_t priority;
    uint8_t is_lost;
    uint8_t error_exist;
    uint8_t data_is_error;
    float frequency;
    data_is_error_fn_t data_is_error_fun;
    solve_lost_fn_t solve_lost_fun;
    solve_data_error_fn_t solve_data_error_fun;
} error_t;

void detect_task(void);
void detect_hook(uint8_t toe);
bool toe_is_error(uint8_t toe);
const error_t *get_error_list_point(void);

#endif
