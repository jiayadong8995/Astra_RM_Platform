#include <assert.h>
#include <string.h>

#include "chassis_topics.h"
#include "device_layer.h"
#include "message_center.h"
#include "test_support/balance_safety_harness.h"

typedef struct
{
    platform_rc_input_t remote;
    platform_device_feedback_t feedback;
} platform_test_arming_context_t;

static platform_device_result_t read_remote(platform_rc_input_t *input, void *context)
{
    platform_test_arming_context_t *arming_context = context;

    *input = arming_context->remote;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_feedback(platform_device_feedback_t *feedback, void *context)
{
    platform_test_arming_context_t *arming_context = context;

    *feedback = arming_context->feedback;
    return PLATFORM_DEVICE_RESULT_OK;
}

static void seed_robot_state(const platform_robot_state_t *robot_state)
{
    Publisher_t *robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));
    assert(robot_state_pub != NULL);
    assert(PubPushMessage(robot_state_pub, (void *)robot_state) > 0U);
}

static void seed_context(platform_test_arming_context_t *context)
{
    memset(context, 0, sizeof(*context));
    context->remote.valid = true;
    context->remote.switches[0] = 3U;
    context->feedback.actuator_feedback.valid = true;
}

static void assert_output_blocked(const platform_actuator_command_t *observed)
{
    assert(!observed->control_enable);
    assert(!observed->actuator_enable);
    assert(!observed->motors.wheels[PLATFORM_WHEEL_LEFT].valid);
    assert(!observed->motors.wheels[PLATFORM_WHEEL_RIGHT].valid);
    assert(!observed->motors.joints[PLATFORM_JOINT_LEFT_FRONT].valid);
    assert(!observed->motors.joints[PLATFORM_JOINT_RIGHT_FRONT].valid);
}

static void test_invalid_transitions_stay_blocked(void)
{
    platform_balance_safety_harness_t harness = {0};
    platform_test_arming_context_t context = {0};
    platform_device_test_hooks_t hooks = {0};
    platform_robot_state_t robot_state = {0};
    platform_actuator_command_t observed = {0};

    seed_context(&context);
    context.remote.switches[1] = 2U;
    robot_state.body.pitch = PITCH_RECOVER_THRESHOLD + 0.05f;
    robot_state.health.state_valid = true;

    hooks.read_remote = read_remote;
    hooks.read_feedback = read_feedback;
    hooks.context = &context;
    platform_device_set_test_hooks(&hooks);

    platform_balance_safety_harness_init(&harness);
    seed_robot_state(&robot_state);
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(observed.start);
    assert_output_blocked(&observed);

    context.remote.switches[1] = 3U;
    context.remote.channels[3] = 660;
    robot_state.body.pitch = 0.0f;
    robot_state.health.state_valid = true;
    seed_robot_state(&robot_state);
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(observed.start);
    assert_output_blocked(&observed);
}

int main(void)
{
    test_invalid_transitions_stay_blocked();
    platform_device_reset_test_hooks();
    return 0;
}
