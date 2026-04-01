#include "ports.h"
#include "device_layer.h"

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample)
{
    return platform_device_read_default_imu(sample);
}

platform_device_result_t platform_remote_read(platform_rc_input_t *input)
{
    return platform_device_read_default_remote(input);
}

platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd)
{
    platform_actuator_command_t wrapper = {0};
    wrapper.motors = *cmd;
    return platform_device_write_default_command(&wrapper);
}

platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback)
{
    return platform_device_read_default_feedback(feedback);
}
