#include "ports.h"
#include <string.h>

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample)
{
    memset(sample, 0, sizeof(*sample));
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_remote_read(platform_rc_input_t *input)
{
    memset(input, 0, sizeof(*input));
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd)
{
    (void)cmd;
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback)
{
    memset(feedback, 0, sizeof(*feedback));
    return PLATFORM_DEVICE_RESULT_OK;
}
