#include <assert.h>
#include <string.h>

#include "actuator_gateway.h"
#include "chassis_topics.h"
#include "device_layer.h"
#include "message_center.h"
#include "test_support/balance_safety_harness.h"

typedef struct
{
    platform_rc_input_t remote;
    platform_device_feedback_t feedback;
    platform_device_command_t last_command;
    uint32_t write_count;
} platform_test_mapping_context_t;

static platform_device_result_t read_remote(platform_rc_input_t *input, void *context)
{
    platform_test_mapping_context_t *mapping_context = context;

    *input = mapping_context->remote;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_feedback(platform_device_feedback_t *feedback, void *context)
{
    platform_test_mapping_context_t *mapping_context = context;

    *feedback = mapping_context->feedback;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t write_command(const platform_device_command_t *command, void *context)
{
    platform_test_mapping_context_t *mapping_context = context;

    mapping_context->last_command = *command;
    mapping_context->write_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

static void seed_context(platform_test_mapping_context_t *context)
{
    memset(context, 0, sizeof(*context));
    context->remote.valid = true;
    context->remote.switches[0] = 3U;
    context->feedback.actuator_feedback.valid = true;
}

static void seed_robot_state(const platform_robot_state_t *robot_state)
{
    Publisher_t *robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));

    assert(robot_state_pub != NULL);
    assert(PubPushMessage(robot_state_pub, (void *)robot_state) > 0U);
}

static platform_actuator_command_t observe_started_command(platform_test_mapping_context_t *context)
{
    platform_balance_safety_harness_t harness = {0};
    platform_device_test_hooks_t hooks = {0};
    platform_actuator_command_t observed = {0};
    platform_robot_state_t robot_state = {0};
    uint32_t baseline_writes;

    hooks.read_remote = read_remote;
    hooks.read_feedback = read_feedback;
    hooks.write_command = write_command;
    hooks.context = context;
    platform_device_set_test_hooks(&hooks);

    platform_balance_safety_harness_init(&harness);
    robot_state.health.state_valid = true;
    seed_robot_state(&robot_state);
    baseline_writes = context->write_count;
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(observed.start);
    assert(observed.control_enable);
    assert(observed.actuator_enable);
    assert(context->write_count == (baseline_writes + 1U));

    return observed;
}

static void assert_all_outputs_disabled(const platform_device_command_t *command)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        assert(!command->joints[i].valid);
    }
    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
    {
        assert(!command->wheels[i].valid);
    }
}

int main(void)
{
    platform_test_mapping_context_t context = {0};
    platform_actuator_command_t observed = {0};
    platform_actuator_command_t invalid_command = {0};
    uint32_t baseline_writes;

    seed_context(&context);
    observed = observe_started_command(&context);
    baseline_writes = context.write_count;

    invalid_command = observed;
    invalid_command.motors.left_wheel.control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    invalid_command.motors.left_wheel.valid = true;
    platform_actuator_gateway_dispatch_command(&invalid_command, 0U);

    assert(context.write_count == (baseline_writes + 1U));
    assert((context.last_command.backend_flags & 0x1U) != 0U);
    assert_all_outputs_disabled(&context.last_command);

    baseline_writes = context.write_count;
    invalid_command = observed;
    invalid_command.motors.right_leg_joint[0].control_mode = PLATFORM_MOTOR_CONTROL_CURRENT;
    invalid_command.motors.right_leg_joint[0].valid = true;
    platform_actuator_gateway_dispatch_command(&invalid_command, 0U);

    assert(context.write_count == (baseline_writes + 1U));
    assert((context.last_command.backend_flags & 0x1U) != 0U);
    assert_all_outputs_disabled(&context.last_command);

    platform_device_reset_test_hooks();
    return 0;
}
