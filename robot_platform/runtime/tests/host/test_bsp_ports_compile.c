#include "ports.h"

int main(void)
{
    platform_imu_sample_t imu = {0};
    platform_rc_input_t rc = {0};
    platform_motor_command_set_t cmd = {0};
    platform_device_feedback_t fb = {0};

    platform_imu_read(&imu);
    platform_remote_read(&rc);
    platform_motor_write_command(&cmd);
    platform_motor_read_feedback(&fb);

    return 0;
}
