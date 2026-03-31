#include <assert.h>
#include <string.h>

#include "chassis_topics.h"
#include "device_layer.h"
#include "test_support/balance_safety_harness.h"
#include "../../control/state/ins_state_estimator.h"

typedef struct
{
    platform_rc_input_t remote;
    platform_device_feedback_t feedback;
    platform_device_command_t last_command;
    uint32_t write_count;
} platform_test_sensor_context_t;

static platform_device_result_t read_remote(platform_rc_input_t *input, void *context)
{
    platform_test_sensor_context_t *sensor_context = context;

    *input = sensor_context->remote;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t read_feedback(platform_device_feedback_t *feedback, void *context)
{
    platform_test_sensor_context_t *sensor_context = context;

    *feedback = sensor_context->feedback;
    return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t write_command(const platform_device_command_t *command, void *context)
{
    platform_test_sensor_context_t *sensor_context = context;

    sensor_context->last_command = *command;
    sensor_context->write_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

static void seed_context(platform_test_sensor_context_t *context)
{
    memset(context, 0, sizeof(*context));
    context->remote.valid = true;
    context->remote.switches[0] = 3U;
    context->feedback.actuator_feedback.valid = true;
}

static void test_pre_ready_and_post_ready_sensor_faults(void)
{
    platform_balance_safety_harness_t harness = {0};
    platform_test_sensor_context_t context = {0};
    platform_device_test_hooks_t hooks = {0};
    platform_ins_state_message_t ins_msg = {0};
    platform_actuator_command_t observed = {0};

    seed_context(&context);
    hooks.read_remote = read_remote;
    hooks.read_feedback = read_feedback;
    hooks.write_command = write_command;
    hooks.context = &context;
    platform_device_set_test_hooks(&hooks);

    platform_balance_safety_harness_init(&harness);
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(observed.control_enable);
    assert(observed.actuator_enable);

    ins_msg = harness.ins_msg;
    ins_msg.ready = 0U;
    platform_balance_safety_harness_seed_ins(&harness, &ins_msg);
    platform_balance_safety_harness_step_once(&harness);
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(!observed.control_enable);
    assert(!observed.actuator_enable);
    assert(!observed.motors.left_wheel.valid);
    assert(!observed.motors.right_leg_joint[0].valid);

    ins_msg.ready = 1U;
    platform_balance_safety_harness_seed_ins(&harness, &ins_msg);
    context.feedback.actuator_feedback.valid = false;
    platform_balance_safety_harness_step_once(&harness);
    platform_balance_safety_harness_step_once(&harness);
    assert(chassis_runtime_bus_get_latest_observation(&observed));
    assert(!observed.control_enable);
    assert(!observed.actuator_enable);
    assert(!observed.motors.right_wheel.valid);
    assert(!observed.motors.left_leg_joint[0].valid);
}

static void test_estimator_warmup_and_degrade_behavior(void)
{
    platform_ins_state_estimator_t state = {0};
    platform_ins_state_message_t msg = {0};
    const float accel[3] = {0.0f, 0.0f, 9.81f};
    const float gyro[3] = {0.0f, 0.0f, 0.0f};
    int ready_tick = -1;

    platform_ins_state_estimator_init(&state);
    platform_ins_state_estimator_build_msg(&state, &msg);
    assert(msg.ready == 0U);

    for (int i = 0; i < 20; ++i)
    {
        platform_ins_state_estimator_apply_sample(&state, 0.001f, accel, gyro);
    }
    platform_ins_state_estimator_build_msg(&state, &msg);
    assert(msg.ready == 0U);

    for (int i = 21; i <= 4005; ++i)
    {
        platform_ins_state_estimator_apply_sample(&state, 0.001f, accel, gyro);
        platform_ins_state_estimator_build_msg(&state, &msg);
        if (msg.ready == 1U)
        {
            ready_tick = i;
            break;
        }
    }
    assert(ready_tick > 20);
    assert(ready_tick >= 21 && ready_tick <= 4005);

    platform_ins_state_estimator_apply_sample(&state, 0.0f, accel, gyro);
    platform_ins_state_estimator_build_msg(&state, &msg);
    assert(msg.ready == 0U);
}

int main(void)
{
    test_pre_ready_and_post_ready_sensor_faults();
    test_estimator_warmup_and_degrade_behavior();

    platform_device_reset_test_hooks();
    return 0;
}
