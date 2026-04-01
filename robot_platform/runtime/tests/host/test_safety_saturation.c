#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "chassis_topics.h"
#include "device_layer.h"
#include "message_center.h"
#include "test_support/balance_safety_harness.h"
#include "../../control/constraints/actuator_constraints.h"

typedef struct
{
    platform_rc_input_t remote;
    platform_device_feedback_t feedback;
} platform_test_saturation_context_t;

static platform_device_result_t read_remote(platform_rc_input_t *input, void *context)
{
    platform_test_saturation_context_t *saturation_context = context;

    *input = saturation_context->remote;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_feedback(platform_device_feedback_t *feedback, void *context)
{
    platform_test_saturation_context_t *saturation_context = context;

    *feedback = saturation_context->feedback;
    return PLATFORM_DEVICE_RESULT_OK;
}

static void seed_robot_state(const platform_robot_state_t *robot_state)
{
    Publisher_t *robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));
    assert(robot_state_pub != NULL);
    assert(PubPushMessage(robot_state_pub, (void *)robot_state) > 0U);
}

static void seed_context(platform_test_saturation_context_t *context)
{
    memset(context, 0, sizeof(*context));
    context->remote.valid = true;
    context->remote.switches[0] = 3U;
    context->feedback.actuator_feedback.valid = true;
    context->feedback.actuator_feedback.wheels[0].velocity = -120.0f;
    context->feedback.actuator_feedback.wheels[1].velocity = 120.0f;
    context->feedback.actuator_feedback.joints[0].position = 2.4f;
    context->feedback.actuator_feedback.joints[1].position = -2.4f;
    context->feedback.actuator_feedback.joints[2].position = 2.4f;
    context->feedback.actuator_feedback.joints[3].position = -2.4f;
}

static void test_current_path_wheel_saturation_has_explicit_oracle(void)
{
    platform_balance_safety_harness_t harness = {0};
    platform_test_saturation_context_t context = {0};
    platform_device_test_hooks_t hooks = {0};
    platform_robot_state_t robot_state = {0};
    platform_ins_state_message_t ins_msg = {0};
    platform_actuator_command_t observed = {0};
    uint32_t oracle_flags;

    seed_context(&context);
    robot_state.health.state_valid = true;

    hooks.read_remote = read_remote;
    hooks.read_feedback = read_feedback;
    hooks.context = &context;
    platform_device_set_test_hooks(&hooks);

    platform_balance_safety_harness_init(&harness);
    seed_robot_state(&robot_state);
    ins_msg = harness.ins_msg;
    ins_msg.pitch = 3.0f;
    ins_msg.gyro[0] = 60.0f;
    platform_balance_safety_harness_seed_ins(&harness, &ins_msg);
    platform_actuator_constraint_oracle_reset();
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(observed.control_enable);
    assert(observed.actuator_enable);
    assert(fabsf(observed.motors.wheels[PLATFORM_WHEEL_LEFT].current_target) <= 8000.0f);
    assert(fabsf(observed.motors.wheels[PLATFORM_WHEEL_RIGHT].current_target) <= 8000.0f);
    oracle_flags = platform_actuator_constraint_oracle_flags();
    assert(oracle_flags != 0U);
}

static void test_wheel_saturation_sets_explicit_oracle(void)
{
    platform_balance_runtime_t chassis = {0};
    uint32_t oracle_flags;

    chassis.wheel_motor[0].torque_set = 5.5f;
    chassis.wheel_motor[0].give_current = 9000;
    chassis.wheel_motor[1].torque_set = -5.5f;
    chassis.wheel_motor[1].give_current = -9000;

    platform_actuator_constraint_oracle_reset();
    platform_constrain_wheel_outputs(&chassis);

    assert(fabsf(chassis.wheel_motor[0].torque_set) <= WHEEL_TORQUE_MAX);
    assert(fabsf(chassis.wheel_motor[1].torque_set) <= WHEEL_TORQUE_MAX);
    assert(abs(chassis.wheel_motor[0].give_current) <= 8000);
    assert(abs(chassis.wheel_motor[1].give_current) <= 8000);
    oracle_flags = platform_actuator_constraint_oracle_flags();
    assert((oracle_flags & PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_WHEEL) != 0U);
    assert((oracle_flags & PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_CURRENT) != 0U);
}

static void test_leg_saturation_sets_explicit_oracle(void)
{
    vmc_leg_t right_leg = {0};
    vmc_leg_t left_leg = {0};
    uint32_t oracle_flags;

    right_leg.F0 = 200.0f;
    right_leg.torque_set[0] = 5.5f;
    right_leg.torque_set[1] = -5.5f;
    left_leg.F0 = -200.0f;
    left_leg.torque_set[0] = -5.5f;
    left_leg.torque_set[1] = 5.5f;

    platform_actuator_constraint_oracle_reset();
    platform_constrain_leg_outputs(&right_leg, &left_leg);

    assert(fabsf(right_leg.torque_set[0]) <= JOINT_TORQUE_MAX);
    assert(fabsf(right_leg.torque_set[1]) <= JOINT_TORQUE_MAX);
    assert(fabsf(left_leg.torque_set[0]) <= JOINT_TORQUE_MAX);
    assert(fabsf(left_leg.torque_set[1]) <= JOINT_TORQUE_MAX);
    oracle_flags = platform_actuator_constraint_oracle_flags();
    assert((oracle_flags & PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_LEG) != 0U);
}

int main(void)
{
    test_current_path_wheel_saturation_has_explicit_oracle();
    platform_device_reset_test_hooks();
    test_wheel_saturation_sets_explicit_oracle();
    test_leg_saturation_sets_explicit_oracle();
    return 0;
}
