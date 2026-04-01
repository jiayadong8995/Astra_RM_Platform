#include "ports.h"

#include "imu/bmi088/BMI088driver.h"
#include "imu/bmi088_device.h"
#include "remote/dbus/remote_control.h"
#include "remote/dbus_remote_device.h"
#include "actuator/motor/dm4310/dm4310_drv.h"
#include "actuator/motor/motor_actuator_device.h"
#include "fdcan.h"
#include "spi.h"

static platform_imu_device_t g_imu;
static platform_remote_device_t g_remote;
static platform_motor_device_t g_motor;
static bool g_bound = false;

static const platform_bmi088_device_config_t g_bmi088_cfg = {
    .spi_handle = &hspi2,
    .sample_state = &BMI088,
    .calibrate = 1U,
    .init_fn = (void (*)(void *, uint8_t))BMI088_Init,
    .read_fn = (void (*)(void *))BMI088_Read,
};

static const platform_dbus_remote_device_config_t g_remote_cfg = {
    .acquire_fn = (const void *(*)(void))get_remote_control_point,
    .is_error_fn = RC_data_is_error,
};

static const platform_motor_actuator_device_config_t g_motor_cfg = {
    .joint_can_handle = &hfdcan1,
    .wheel_can_handle = &hfdcan2,
    .get_joint_state_fn = (void *(*)(uint8_t))get_joint_motor_state,
    .get_wheel_state_fn = (void *(*)(uint8_t))get_chassis_motor_measure_point,
    .joint_init_fn = (void (*)(void *, uint16_t, uint16_t))joint_motor_init,
    .enable_mode_fn = (int (*)(void *, uint16_t, uint16_t))enable_motor_mode,
    .mit_ctrl_fn = (void (*)(void *, uint16_t, float, float, float, float, float))mit_ctrl,
    .wheel_cmd_fn = (void (*)(void *, int16_t, int16_t, int16_t, int16_t))CAN_cmd_chassis,
};

static void platform_ports_hw_ensure_bound(void)
{
    if (g_bound)
    {
        return;
    }

    platform_bmi088_device_bind(&g_imu, &g_bmi088_cfg);
    platform_dbus_remote_device_bind(&g_remote, &g_remote_cfg);
    platform_motor_actuator_device_bind(&g_motor, &g_motor_cfg);
    g_bound = true;
}

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample)
{
    platform_ports_hw_ensure_bound();
    if (g_imu.ops.read_sample == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_imu.ops.read_sample(&g_imu, sample);
}

platform_device_result_t platform_remote_read(platform_rc_input_t *input)
{
    platform_ports_hw_ensure_bound();
    if (g_remote.ops.read_input == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_remote.ops.read_input(&g_remote, input);
}

platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd)
{
    platform_ports_hw_ensure_bound();
    if (g_motor.ops.write_motor_command == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_motor.ops.write_motor_command(&g_motor, cmd);
}

platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback)
{
    platform_ports_hw_ensure_bound();
    if (g_motor.ops.read_motor_feedback == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_motor.ops.read_motor_feedback(&g_motor, feedback);
}
