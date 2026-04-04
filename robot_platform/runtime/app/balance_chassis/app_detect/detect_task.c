#include "detect_task.h"
#include "cmsis_os.h"
#include <stddef.h>

static error_t error_list[ERROR_LIST_LENGTH + 1];

static void detect_init(uint32_t time)
{
    /* offlineTime, onlineTime, priority */
    uint16_t set_item[ERROR_LIST_LENGTH][3] =
    {
        {30,  40,  15},  /* DBUS_TOE */
        {10,  10,  11},  /* JOINT_MOTOR1_TOE */
        {10,  10,  10},  /* JOINT_MOTOR2_TOE */
        {10,  10,  9},   /* JOINT_MOTOR3_TOE */
        {10,  10,  8},   /* JOINT_MOTOR4_TOE */
        {10,  10,  7},   /* WHEEL_MOTOR1_TOE */
        {10,  10,  6},   /* WHEEL_MOTOR2_TOE */
        {2,   3,   14},  /* IMU_ACCEL_TOE */
        {2,   3,   13},  /* IMU_GYRO_TOE */
    };

    for (uint8_t i = 0; i < ERROR_LIST_LENGTH; i++)
    {
        error_list[i].set_offline_time = set_item[i][0];
        error_list[i].set_online_time = set_item[i][1];
        error_list[i].priority = set_item[i][2];
        error_list[i].data_is_error_fun = NULL;
        error_list[i].solve_lost_fun = NULL;
        error_list[i].solve_data_error_fun = NULL;

        error_list[i].enable = 1;
        error_list[i].error_exist = 1;
        error_list[i].is_lost = 1;
        error_list[i].data_is_error = 1;
        error_list[i].frequency = 0.0f;
        error_list[i].new_time = time;
        error_list[i].last_time = time;
        error_list[i].lost_time = time;
        error_list[i].work_time = time;
    }
}

void detect_task(void)
{
    uint32_t system_time = osKernelSysTick();
    detect_init(system_time);
    osDelay(DETECT_TASK_INIT_TIME);

    while (1)
    {
        uint8_t error_num_display = ERROR_LIST_LENGTH;
        system_time = osKernelSysTick();

        error_list[ERROR_LIST_LENGTH].is_lost = 0;
        error_list[ERROR_LIST_LENGTH].error_exist = 0;

        for (int i = 0; i < ERROR_LIST_LENGTH; i++)
        {
            if (error_list[i].enable == 0)
            {
                continue;
            }

            if (system_time - error_list[i].new_time > error_list[i].set_offline_time)
            {
                if (error_list[i].error_exist == 0)
                {
                    error_list[i].is_lost = 1;
                    error_list[i].error_exist = 1;
                    error_list[i].lost_time = system_time;
                }

                if (error_list[i].priority > error_list[error_num_display].priority)
                {
                    error_num_display = i;
                }

                error_list[ERROR_LIST_LENGTH].is_lost = 1;
                error_list[ERROR_LIST_LENGTH].error_exist = 1;

                if (error_list[i].solve_lost_fun != NULL)
                {
                    error_list[i].solve_lost_fun();
                }
            }
            else if (system_time - error_list[i].work_time < error_list[i].set_online_time)
            {
                error_list[i].is_lost = 0;
                error_list[i].error_exist = 1;
            }
            else
            {
                error_list[i].is_lost = 0;

                if (error_list[i].data_is_error_fun != NULL)
                {
                    error_list[i].error_exist = 1;
                }
                else
                {
                    error_list[i].error_exist = 0;
                }

                if (error_list[i].new_time > error_list[i].last_time)
                {
                    error_list[i].frequency =
                        1000.0f / (float)(error_list[i].new_time - error_list[i].last_time);
                }
            }
        }

        osDelay(DETECT_CONTROL_TIME);
    }
}

void detect_hook(uint8_t toe)
{
    error_list[toe].last_time = error_list[toe].new_time;
    error_list[toe].new_time = osKernelSysTick();

    if (error_list[toe].is_lost)
    {
        error_list[toe].is_lost = 0;
        error_list[toe].work_time = error_list[toe].new_time;
    }

    if (error_list[toe].data_is_error_fun != NULL)
    {
        if (error_list[toe].data_is_error_fun())
        {
            error_list[toe].error_exist = 1;
            error_list[toe].data_is_error = 1;

            if (error_list[toe].solve_data_error_fun != NULL)
            {
                error_list[toe].solve_data_error_fun();
            }
        }
        else
        {
            error_list[toe].data_is_error = 0;
        }
    }
    else
    {
        error_list[toe].data_is_error = 0;
    }
}

bool toe_is_error(uint8_t toe)
{
    return (error_list[toe].error_exist == 1);
}

const error_t *get_error_list_point(void)
{
    return error_list;
}
