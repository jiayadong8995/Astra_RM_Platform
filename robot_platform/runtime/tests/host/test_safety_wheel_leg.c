#include <assert.h>
#include <string.h>

#include "chassis_topics.h"
#include "device_layer.h"
#include "test_support/balance_safety_harness.h"

typedef struct
{
    platform_rc_input_t remote;
    platform_device_feedback_t feedback;
} platform_test_wheel_leg_context_t;

static platform_device_result_t read_remote(platform_rc_input_t *input, void *context)
{
    platform_test_wheel_leg_context_t *wheel_leg_context = context;

    *input = wheel_leg_context->remote;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_feedback(platform_device_feedback_t *feedback, void *context)
{
    platform_test_wheel_leg_context_t *wheel_leg_context = context;

    *feedback = wheel_leg_context->feedback;
    return PLATFORM_DEVICE_RESULT_OK;
}

static void seed_context(platform_test_wheel_leg_context_t *context)
{
    memset(context, 0, sizeof(*context));
    context->remote.valid = true;
    context->remote.sample_time_us = 1000U;
    context->remote.switches[0] = 3U;
    context->feedback.actuator_feedback.valid = true;
    context->feedback.actuator_feedback.joints[0].position = 2.4f;
    context->feedback.actuator_feedback.joints[1].position = -2.4f;
    context->feedback.actuator_feedback.joints[2].position = 2.4f;
    context->feedback.actuator_feedback.joints[3].position = -2.4f;
    context->feedback.actuator_feedback.wheels[0].velocity = -120.0f;
    context->feedback.actuator_feedback.wheels[1].velocity = 120.0f;
}

int main(void)
{
    platform_balance_safety_harness_t harness = {0};
    platform_test_wheel_leg_context_t context = {0};
    platform_device_test_hooks_t hooks = {0};
    platform_robot_state_t robot_state = {0};
    platform_ins_state_message_t ins_msg = {0};
    platform_actuator_command_t observed = {0};

    seed_context(&context);
    robot_state.health.state_valid = true;

    hooks.read_remote = read_remote;
    hooks.read_feedback = read_feedback;
    hooks.context = &context;
    platform_device_set_test_hooks(&hooks);

    platform_balance_safety_harness_init(&harness);
    {
        Publisher_t *robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));

        assert(robot_state_pub != NULL);
        assert(PubPushMessage(robot_state_pub, &robot_state) > 0U);
    }

    ins_msg = harness.ins_msg;
    ins_msg.pitch = 3.0f;
    ins_msg.gyro[0] = 200.0f;
    platform_balance_safety_harness_seed_ins(&harness, &ins_msg);
    platform_balance_safety_harness_step_once(&harness);

    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(observed.start);
    assert(!observed.control_enable);
    assert(!observed.actuator_enable);
    assert(observed.motors.wheels[PLATFORM_WHEEL_LEFT].current_target == 0.0f);
    assert(observed.motors.wheels[PLATFORM_WHEEL_RIGHT].current_target == 0.0f);
    assert(observed.motors.joints[PLATFORM_JOINT_LEFT_FRONT].torque_target == 0.0f);
    assert(observed.motors.joints[PLATFORM_JOINT_RIGHT_FRONT].torque_target == 0.0f);

    platform_device_reset_test_hooks();
    return 0;
}
