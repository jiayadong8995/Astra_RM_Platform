#include <assert.h>
#include <string.h>

#include "device_layer.h"

typedef struct
{
    platform_rc_input_t remote;
    platform_imu_sample_t imu;
    platform_device_feedback_t feedback;
    platform_device_command_t last_command;
    uint32_t init_count;
    uint32_t write_count;
} platform_test_device_seam_context_t;

static platform_device_result_t init_default_profile(void *context)
{
    platform_test_device_seam_context_t *seam_context = context;

    seam_context->init_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_remote(platform_rc_input_t *input, void *context)
{
    platform_test_device_seam_context_t *seam_context = context;

    *input = seam_context->remote;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_imu(platform_imu_sample_t *sample, void *context)
{
    platform_test_device_seam_context_t *seam_context = context;

    *sample = seam_context->imu;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_feedback(platform_device_feedback_t *feedback, void *context)
{
    platform_test_device_seam_context_t *seam_context = context;

    *feedback = seam_context->feedback;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t write_command(const platform_device_command_t *command, void *context)
{
    platform_test_device_seam_context_t *seam_context = context;

    seam_context->last_command = *command;
    seam_context->write_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

int main(void)
{
    platform_test_device_seam_context_t context = {0};
    platform_device_test_hooks_t hooks = {0};
    platform_rc_input_t remote = {0};
    platform_imu_sample_t imu = {0};
    platform_device_feedback_t feedback = {0};
    platform_device_command_t command = {0};

    context.remote.valid = true;
    context.remote.channels[0] = 321;
    context.imu.valid = true;
    context.imu.gyro[1] = 4.5f;
    context.feedback.actuator_feedback.valid = true;
    context.feedback.actuator_feedback.wheels[1].online = true;

    hooks.init_default_profile = init_default_profile;
    hooks.read_remote = read_remote;
    hooks.read_imu = read_imu;
    hooks.read_feedback = read_feedback;
    hooks.write_command = write_command;
    hooks.context = &context;
    platform_device_set_test_hooks(&hooks);

    assert(platform_device_init_default_profile() == PLATFORM_DEVICE_RESULT_OK);
    assert(context.init_count == 1U);

    assert(platform_device_read_default_remote(&remote) == PLATFORM_DEVICE_RESULT_OK);
    assert(memcmp(&remote, &context.remote, sizeof(remote)) == 0);

    assert(platform_device_read_default_imu(&imu) == PLATFORM_DEVICE_RESULT_OK);
    assert(memcmp(&imu, &context.imu, sizeof(imu)) == 0);

    assert(platform_device_read_default_feedback(&feedback) == PLATFORM_DEVICE_RESULT_OK);
    assert(memcmp(&feedback, &context.feedback, sizeof(feedback)) == 0);

    command.joints[0].valid = true;
    command.wheels[0].control_mode = PLATFORM_MOTOR_CONTROL_CURRENT;
    assert(platform_device_write_default_command(&command) == PLATFORM_DEVICE_RESULT_OK);
    assert(context.write_count == 1U);
    assert(memcmp(&context.last_command, &command, sizeof(command)) == 0);

    platform_device_reset_test_hooks();
    return 0;
}
