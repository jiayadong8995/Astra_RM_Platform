#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "chassis_topics.h"
#include "device_layer.h"
#include "message_center.h"
#include "test_support/balance_safety_harness.h"

typedef struct
{
    platform_rc_input_t remote;
    platform_imu_sample_t imu;
    platform_device_feedback_t feedback;
    platform_device_command_t last_command;
    platform_device_result_t remote_result;
    uint32_t write_count;
} platform_test_device_hook_context_t;

static platform_device_result_t read_remote(platform_rc_input_t *input, void *context)
{
    platform_test_device_hook_context_t *hook_context = context;

    if (hook_context->remote_result != PLATFORM_DEVICE_RESULT_OK)
    {
        return hook_context->remote_result;
    }

    *input = hook_context->remote;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_imu(platform_imu_sample_t *sample, void *context)
{
    platform_test_device_hook_context_t *hook_context = context;

    *sample = hook_context->imu;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_feedback(platform_device_feedback_t *feedback, void *context)
{
    platform_test_device_hook_context_t *hook_context = context;

    *feedback = hook_context->feedback;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t write_command(const platform_device_command_t *command, void *context)
{
    platform_test_device_hook_context_t *hook_context = context;

    hook_context->last_command = *command;
    hook_context->write_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

static void seed_hook_context(platform_test_device_hook_context_t *context)
{
    memset(context, 0, sizeof(*context));
    context->remote.valid = true;
    context->remote.sample_time_us = 1000U;
    context->remote.switches[0] = 3U;
    context->feedback.actuator_feedback.valid = true;
    context->feedback.actuator_feedback.sample_time_us = 2000U;
    context->feedback.actuator_feedback.joints[0].online = true;
    context->feedback.actuator_feedback.wheels[0].online = true;
    context->imu.valid = true;
    context->imu.sample_time_us = 3000U;
    context->remote_result = PLATFORM_DEVICE_RESULT_OK;
}

static void seed_robot_state(const platform_robot_state_t *robot_state)
{
    Publisher_t *robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));

    assert(robot_state_pub != NULL);
    assert(PubPushMessage(robot_state_pub, (void *)robot_state) > 0U);
}

static void assert_dispatch_matches_observed_command(const platform_actuator_command_t *observed,
                                                     const platform_device_command_t *written)
{
    const bool dispatch_enabled = observed->control_enable && observed->actuator_enable;

    assert(written->joints[0].valid == (observed->motors.left_leg_joint[0].valid && dispatch_enabled));
    assert(written->joints[1].valid == (observed->motors.left_leg_joint[1].valid && dispatch_enabled));
    assert(written->joints[2].valid == (observed->motors.right_leg_joint[0].valid && dispatch_enabled));
    assert(written->joints[3].valid == (observed->motors.right_leg_joint[1].valid && dispatch_enabled));
    assert(written->wheels[0].valid == (observed->motors.left_wheel.valid && dispatch_enabled));
    assert(written->wheels[1].valid == (observed->motors.right_wheel.valid && dispatch_enabled));
    assert(written->joints[0].control_mode == observed->motors.left_leg_joint[0].control_mode);
    assert(written->wheels[0].control_mode == observed->motors.left_wheel.control_mode);
    assert(written->wheels[1].control_mode == observed->motors.right_wheel.control_mode);
}

int main(void)
{
    platform_balance_safety_harness_t harness = {0};
    platform_test_device_hook_context_t hook_context = {0};
    platform_device_test_hooks_t hooks = {0};
    platform_actuator_command_t first_command = {0};
    platform_actuator_command_t latest_command = {0};
    platform_robot_state_t robot_state = {0};
    uint32_t baseline_write_count;

    seed_hook_context(&hook_context);
    hooks.read_remote = read_remote;
    hooks.read_imu = read_imu;
    hooks.read_feedback = read_feedback;
    hooks.write_command = write_command;
    hooks.context = &hook_context;
    platform_device_set_test_hooks(&hooks);

    platform_balance_safety_harness_init(&harness);
    robot_state.health.state_valid = true;
    seed_robot_state(&robot_state);
    baseline_write_count = hook_context.write_count;
    assert(chassis_runtime_bus_observation_count() == 0U);

    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_observation_count() == 1U);
    assert(hook_context.write_count == (baseline_write_count + 1U));
    assert(chassis_runtime_bus_get_first_observation(&first_command));
    assert(first_command.start);
    assert(first_command.control_enable);
    assert(first_command.actuator_enable);
    assert_dispatch_matches_observed_command(&first_command, &hook_context.last_command);

    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_observation_count() == 2U);
    assert(hook_context.write_count == (baseline_write_count + 2U));
    assert(chassis_runtime_bus_get_latest_observation(&latest_command));
    assert(latest_command.control_enable);
    assert(latest_command.actuator_enable);
    assert_dispatch_matches_observed_command(&latest_command, &hook_context.last_command);

    hook_context.remote_result = PLATFORM_DEVICE_RESULT_UNAVAILABLE;
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&latest_command));
    assert(!latest_command.start);
    assert(!latest_command.control_enable);
    assert(!latest_command.actuator_enable);
    assert(latest_command.motors.left_leg_joint[0].torque_target == 0.0f);
    assert(latest_command.motors.right_leg_joint[0].torque_target == 0.0f);

    hook_context.remote_result = PLATFORM_DEVICE_RESULT_OK;
    hook_context.remote.sample_time_us = 2000U;
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&latest_command));
    assert(latest_command.start);
    assert(latest_command.control_enable);
    assert(latest_command.actuator_enable);

    platform_balance_safety_harness_step_once(&harness);
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&latest_command));
    assert(!latest_command.start);
    assert(!latest_command.control_enable);
    assert(!latest_command.actuator_enable);
    assert(latest_command.motors.left_wheel.current_target == 0.0f);
    assert(latest_command.motors.right_wheel.current_target == 0.0f);

    platform_device_reset_test_hooks();
    return 0;
}
