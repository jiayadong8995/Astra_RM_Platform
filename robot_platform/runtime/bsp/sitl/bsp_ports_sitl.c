#include "ports.h"

#include "drivers/imu/bmi088/BMI088driver.h"
#include "drivers/imu/bmi088_device.h"
#include "drivers/remote/dbus/remote_control.h"
#include "drivers/remote/dbus_remote_device.h"
#include "drivers/actuator/motor/motor_actuator_device.h"

static platform_imu_device_t g_imu;
static platform_remote_device_t g_remote;
static platform_motor_device_t g_motor;
static bool g_bound = false;

static const platform_bmi088_device_config_t g_bmi088_cfg = {
    .spi_handle = 0,
    .sample_state = &BMI088,
    .calibrate = 0U,
    .init_fn = (void (*)(void *, uint8_t))BMI088_Init,
    .read_fn = (void (*)(void *))BMI088_Read,
};

static const platform_dbus_remote_device_config_t g_remote_cfg = {
    .acquire_fn = (const void *(*)(void))get_remote_control_point,
    .is_error_fn = RC_data_is_error,
};

static void platform_ports_sitl_ensure_bound(void)
{
    if (g_bound)
    {
        return;
    }

    platform_bmi088_device_bind(&g_imu, &g_bmi088_cfg);
    platform_dbus_remote_device_bind(&g_remote, &g_remote_cfg);
    platform_motor_actuator_device_bind(&g_motor, 0);
    g_bound = true;
}

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample)
{
    platform_ports_sitl_ensure_bound();
    if (g_imu.ops.read_sample == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_imu.ops.read_sample(&g_imu, sample);
}

platform_device_result_t platform_remote_read(platform_rc_input_t *input)
{
    platform_ports_sitl_ensure_bound();
    if (g_remote.ops.read_input == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_remote.ops.read_input(&g_remote, input);
}

platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd)
{
    platform_ports_sitl_ensure_bound();
    if (g_motor.ops.write_motor_command == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_motor.ops.write_motor_command(&g_motor, cmd);
}

platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback)
{
    platform_ports_sitl_ensure_bound();
    if (g_motor.ops.read_motor_feedback == 0)
    {
        return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
    }
    return g_motor.ops.read_motor_feedback(&g_motor, feedback);
}
