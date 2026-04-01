#include "ports_fake.h"
#include "device_layer.h"
#include <string.h>

static platform_ports_fake_hooks_t g_fake_hooks;
static platform_device_command_t g_last_command;

/* Bridge adapters: forward device_layer hook calls to ports_fake hooks */
static platform_device_result_t bridge_read_imu(platform_imu_sample_t *sample, void *context)
{
    (void)context;
    if (g_fake_hooks.read_imu != 0)
    {
        return g_fake_hooks.read_imu(sample, g_fake_hooks.context);
    }
    memset(sample, 0, sizeof(*sample));
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t bridge_read_remote(platform_rc_input_t *input, void *context)
{
    (void)context;
    if (g_fake_hooks.read_remote != 0)
    {
        return g_fake_hooks.read_remote(input, g_fake_hooks.context);
    }
    memset(input, 0, sizeof(*input));
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t bridge_read_feedback(platform_device_feedback_t *feedback, void *context)
{
    (void)context;
    if (g_fake_hooks.read_feedback != 0)
    {
        return g_fake_hooks.read_feedback(feedback, g_fake_hooks.context);
    }
    memset(feedback, 0, sizeof(*feedback));
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t bridge_write_command(const platform_device_command_t *command, void *context)
{
    (void)context;
    g_last_command = *command;
    if (g_fake_hooks.write_command != 0)
    {
        return g_fake_hooks.write_command(command, g_fake_hooks.context);
    }
    return PLATFORM_DEVICE_RESULT_OK;
}

void platform_ports_fake_set_hooks(const platform_ports_fake_hooks_t *hooks)
{
    platform_device_test_hooks_t device_hooks = {0};

    if (hooks == 0)
    {
        platform_ports_fake_reset_hooks();
        return;
    }

    g_fake_hooks = *hooks;
    memset(&g_last_command, 0, sizeof(g_last_command));

    /* Install bridge adapters into device_layer so tasks that still call
       device_layer default API get routed through our hooks. */
    device_hooks.read_imu = bridge_read_imu;
    device_hooks.read_remote = bridge_read_remote;
    device_hooks.read_feedback = bridge_read_feedback;
    device_hooks.write_command = bridge_write_command;
    device_hooks.context = 0;
    platform_device_set_test_hooks(&device_hooks);
}

void platform_ports_fake_reset_hooks(void)
{
    g_fake_hooks = (platform_ports_fake_hooks_t){0};
    memset(&g_last_command, 0, sizeof(g_last_command));
    platform_device_reset_test_hooks();
}

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample)
{
    if (g_fake_hooks.read_imu != 0)
    {
        return g_fake_hooks.read_imu(sample, g_fake_hooks.context);
    }
    memset(sample, 0, sizeof(*sample));
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_remote_read(platform_rc_input_t *input)
{
    if (g_fake_hooks.read_remote != 0)
    {
        return g_fake_hooks.read_remote(input, g_fake_hooks.context);
    }
    memset(input, 0, sizeof(*input));
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd)
{
    platform_device_command_t wrapper = {0};
    wrapper.motors = *cmd;
    g_last_command = wrapper;
    if (g_fake_hooks.write_command != 0)
    {
        return g_fake_hooks.write_command(&wrapper, g_fake_hooks.context);
    }
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback)
{
    if (g_fake_hooks.read_feedback != 0)
    {
        return g_fake_hooks.read_feedback(feedback, g_fake_hooks.context);
    }
    memset(feedback, 0, sizeof(*feedback));
    return PLATFORM_DEVICE_RESULT_OK;
}

const platform_device_command_t *platform_ports_fake_last_command(void)
{
    return &g_last_command;
}
