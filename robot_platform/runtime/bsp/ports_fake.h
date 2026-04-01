#ifndef PLATFORM_BSP_PORTS_FAKE_H
#define PLATFORM_BSP_PORTS_FAKE_H

#include "ports.h"
#include "device_command.h"

typedef platform_device_result_t (*platform_ports_fake_imu_hook_t)(platform_imu_sample_t *sample, void *context);
typedef platform_device_result_t (*platform_ports_fake_remote_hook_t)(platform_rc_input_t *input, void *context);
typedef platform_device_result_t (*platform_ports_fake_feedback_hook_t)(platform_device_feedback_t *feedback, void *context);
typedef platform_device_result_t (*platform_ports_fake_command_hook_t)(const platform_device_command_t *command, void *context);

typedef struct
{
    platform_ports_fake_imu_hook_t read_imu;
    platform_ports_fake_remote_hook_t read_remote;
    platform_ports_fake_feedback_hook_t read_feedback;
    platform_ports_fake_command_hook_t write_command;
    void *context;
} platform_ports_fake_hooks_t;

void platform_ports_fake_set_hooks(const platform_ports_fake_hooks_t *hooks);
void platform_ports_fake_reset_hooks(void);

const platform_device_command_t *platform_ports_fake_last_command(void);

#endif
