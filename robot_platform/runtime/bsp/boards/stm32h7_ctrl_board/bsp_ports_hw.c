#include "ports.h"

#include <string.h>

#include "bsp_dwt.h"
#include "can_bsp.h"
#include "drivers/imu/bmi088/BMI088driver.h"
#include "drivers/remote/dbus/remote_control.h"
#include "drivers/actuator/motor/dm4310/dm4310_drv.h"
#include "fdcan.h"
#include "spi.h"
#include "usart.h"

#define MOTOR_ENABLE_DELAY_MS 100U

static Joint_Motor_t *g_joints[PLATFORM_JOINT_MOTOR_COUNT];
static chassis_motor_measure_t *g_wheels[PLATFORM_WHEEL_MOTOR_COUNT];
static bool g_initialized = false;

void platform_ports_init(void)
{
    if (g_initialized)
    {
        return;
    }

    /* DWT cycle counter for INS timing */
    DWT_Init(480);

    /* CAN bus application-level config (filters, notifications, start) */
    FDCAN1_Config();
    FDCAN2_Config();

    /* Start DBUS UART DMA receive */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, sbus_rx_buf, RC_FRAME_LENGTH * 2);

    /* IMU init with calibration (blocks until done) */
    BMI088_Init(&hspi2, 1U);

    /* Joint motors: init state, enable MIT mode */
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        g_joints[i] = get_joint_motor_state(i);
        joint_motor_init(g_joints[i], (uint16_t)(i + 1U), MIT_MODE);
        enable_motor_mode(&hfdcan1, g_joints[i]->para.id, g_joints[i]->mode);
    }

    /* Wheel motors: get state pointers */
    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
    {
        g_wheels[i] = get_chassis_motor_measure_point(i);
    }

    /* Allow DM4310 to complete mode transition before control loop starts */
    HAL_Delay(MOTOR_ENABLE_DELAY_MS);

    g_initialized = true;
}

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample)
{
    BMI088_Read(&BMI088);

    sample->accel[0] = BMI088.Accel[0];
    sample->accel[1] = BMI088.Accel[1];
    sample->accel[2] = BMI088.Accel[2];
    sample->gyro[0] = BMI088.Gyro[0];
    sample->gyro[1] = BMI088.Gyro[1];
    sample->gyro[2] = BMI088.Gyro[2];
    sample->temperature = BMI088.Temperature;
    sample->valid = true;
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_remote_read(platform_rc_input_t *input)
{
    const RC_ctrl_t *rc = get_remote_control_point();

    if (rc == 0)
    {
        return PLATFORM_DEVICE_RESULT_INVALID;
    }

    input->channels[0] = rc->rc.ch[0];
    input->channels[1] = rc->rc.ch[1];
    input->channels[2] = rc->rc.ch[2];
    input->channels[3] = rc->rc.ch[3];
    input->channels[4] = rc->rc.ch[4];
    input->switches[0] = (uint8_t)rc->rc.s[0];
    input->switches[1] = (uint8_t)rc->rc.s[1];
    input->mouse_x = rc->mouse.x;
    input->mouse_y = rc->mouse.y;
    input->mouse_z = rc->mouse.z;
    input->mouse_left = rc->mouse.press_l;
    input->mouse_right = rc->mouse.press_r;
    input->keyboard_mask = rc->key.v;
    input->source = 0U;
    input->valid = (RC_data_is_error() == 0U);
    return input->valid ? PLATFORM_DEVICE_RESULT_OK : PLATFORM_DEVICE_RESULT_INVALID;
}

platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        if (cmd->joints[i].valid)
        {
            mit_ctrl(&hfdcan1, g_joints[i]->para.id, 0.0f, 0.0f,
                     cmd->joints[i].kp, cmd->joints[i].kd,
                     cmd->joints[i].torque_target);
        }
        else
        {
            mit_ctrl(&hfdcan1, g_joints[i]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    if (cmd->wheels[PLATFORM_WHEEL_LEFT].valid && cmd->wheels[PLATFORM_WHEEL_RIGHT].valid)
    {
        CAN_cmd_chassis(&hfdcan2,
                        (int16_t)cmd->wheels[PLATFORM_WHEEL_LEFT].current_target,
                        (int16_t)cmd->wheels[PLATFORM_WHEEL_RIGHT].current_target,
                        0, 0);
    }
    else
    {
        CAN_cmd_chassis(&hfdcan2, 0, 0, 0, 0);
    }

    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        feedback->actuator_feedback.joints[i].id = i;
        feedback->actuator_feedback.joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
        feedback->actuator_feedback.joints[i].position = g_joints[i]->para.pos;
        feedback->actuator_feedback.joints[i].velocity = g_joints[i]->para.vel;
        feedback->actuator_feedback.joints[i].torque_est = g_joints[i]->para.tor;
        feedback->actuator_feedback.joints[i].temperature = g_joints[i]->para.Tmos;
        feedback->actuator_feedback.joints[i].online = true;
    }

    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
    {
        feedback->actuator_feedback.wheels[i].id = i;
        feedback->actuator_feedback.wheels[i].kind = PLATFORM_MOTOR_KIND_WHEEL;
        feedback->actuator_feedback.wheels[i].position = g_wheels[i]->total_angle;
        feedback->actuator_feedback.wheels[i].velocity = g_wheels[i]->speed_rpm;
        feedback->actuator_feedback.wheels[i].torque_est = g_wheels[i]->given_current;
        feedback->actuator_feedback.wheels[i].temperature = g_wheels[i]->temperate;
        feedback->actuator_feedback.wheels[i].online = true;
    }

    feedback->actuator_feedback.valid = true;
    return PLATFORM_DEVICE_RESULT_OK;
}
