#ifndef PLATFORM_BSP_PORTS_H
#define PLATFORM_BSP_PORTS_H

#include "device_input.h"
#include "device_feedback.h"
#include "actuator_command.h"
#include "device_types.h"

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample);
platform_device_result_t platform_remote_read(platform_rc_input_t *input);
platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd);
platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback);

#endif
