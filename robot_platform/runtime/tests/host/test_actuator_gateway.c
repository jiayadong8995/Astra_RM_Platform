#include <assert.h>
#include <string.h>

#include "actuator_gateway.h"
#include "ports_fake.h"

/* actuator_gateway_init still calls platform_device_init_default_profile
   which lives in device_layer.  Provide a minimal stub here. */
static uint32_t g_init_call_count;

platform_device_result_t platform_device_init_default_profile(void)
{
    g_init_call_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

/* Hook context for feedback seeding */
static platform_device_feedback_t g_seeded_feedback;
static platform_device_result_t g_feedback_result;

static platform_device_result_t hook_read_feedback(platform_device_feedback_t *feedback, void *context)
{
    (void)context;
    if (feedback == NULL) {
        return PLATFORM_DEVICE_RESULT_INVALID;
    }
    *feedback = g_seeded_feedback;
    return g_feedback_result;
}

/* Hook context for write counting */
static uint32_t g_write_call_count;

static platform_device_result_t hook_write_command(const platform_device_command_t *command, void *context)
{
    (void)context;
    (void)command;
    g_write_call_count += 1U;
    return PLATFORM_DEVICE_RESULT_OK;
}

static void reset_test_state(void)
{
    g_init_call_count = 0U;
    g_write_call_count = 0U;
    g_feedback_result = PLATFORM_DEVICE_RESULT_OK;
    memset(&g_seeded_feedback, 0, sizeof(g_seeded_feedback));
    platform_ports_fake_reset_hooks();
}

static void install_hooks(void)
{
    platform_ports_fake_hooks_t hooks = {0};
    hooks.read_feedback = hook_read_feedback;
    hooks.write_command = hook_write_command;
    platform_ports_fake_set_hooks(&hooks);
}

static platform_actuator_command_t build_command_fixture(void)
{
    platform_actuator_command_t command = {0};

    command.timestamp_us = 3210U;
    command.sequence = 17U;
    command.start = true;
    command.control_enable = true;
    command.actuator_enable = true;

    command.motors.joints[PLATFORM_JOINT_LEFT_FRONT] = (platform_motor_command_t){
        .control_mode = PLATFORM_MOTOR_CONTROL_TORQUE,
        .torque_target = 1.0f,
        .velocity_target = 2.0f,
        .position_target = 3.0f,
        .current_target = 4.0f,
        .kp = 5.0f,
        .kd = 6.0f,
        .valid = true,
    };
    command.motors.joints[PLATFORM_JOINT_LEFT_REAR] = (platform_motor_command_t){
        .control_mode = PLATFORM_MOTOR_CONTROL_TORQUE,
        .torque_target = 11.0f,
        .velocity_target = 12.0f,
        .position_target = 13.0f,
        .current_target = 14.0f,
        .kp = 15.0f,
        .kd = 16.0f,
        .valid = false,
    };
    command.motors.joints[PLATFORM_JOINT_RIGHT_FRONT] = (platform_motor_command_t){
        .control_mode = PLATFORM_MOTOR_CONTROL_TORQUE,
        .torque_target = 21.0f,
        .velocity_target = 22.0f,
        .position_target = 23.0f,
        .current_target = 24.0f,
        .kp = 25.0f,
        .kd = 26.0f,
        .valid = true,
    };
    command.motors.joints[PLATFORM_JOINT_RIGHT_REAR] = (platform_motor_command_t){
        .control_mode = PLATFORM_MOTOR_CONTROL_TORQUE,
        .torque_target = 31.0f,
        .velocity_target = 32.0f,
        .position_target = 33.0f,
        .current_target = 34.0f,
        .kp = 35.0f,
        .kd = 36.0f,
        .valid = true,
    };
    command.motors.wheels[PLATFORM_WHEEL_LEFT] = (platform_motor_command_t){
        .control_mode = PLATFORM_MOTOR_CONTROL_CURRENT,
        .torque_target = 41.0f,
        .velocity_target = 42.0f,
        .position_target = 43.0f,
        .current_target = 44.0f,
        .kp = 45.0f,
        .kd = 46.0f,
        .valid = true,
    };
    command.motors.wheels[PLATFORM_WHEEL_RIGHT] = (platform_motor_command_t){
        .control_mode = PLATFORM_MOTOR_CONTROL_CURRENT,
        .torque_target = 51.0f,
        .velocity_target = 52.0f,
        .position_target = 53.0f,
        .current_target = 54.0f,
        .kp = 55.0f,
        .kd = 56.0f,
        .valid = false,
    };

    return command;
}

static void assert_motor_command_matches(const platform_motor_command_t *actual,
                                         const platform_motor_command_t *expected,
                                         bool expected_valid)
{
    assert(actual->control_mode == expected->control_mode);
    assert(actual->torque_target == expected->torque_target);
    assert(actual->velocity_target == expected->velocity_target);
    assert(actual->position_target == expected->position_target);
    assert(actual->current_target == expected->current_target);
    assert(actual->kp == expected->kp);
    assert(actual->kd == expected->kd);
    assert(actual->valid == expected_valid);
}

static void test_init_calls_default_profile_once(void)
{
    reset_test_state();

    platform_actuator_gateway_init();

    assert(g_init_call_count == 1U);
    assert(g_write_call_count == 0U);
}

static void test_capture_feedback_forwards_result_and_payload(void)
{
    platform_device_feedback_t observed_feedback = {0};
    platform_device_result_t result;

    reset_test_state();
    install_hooks();

    g_seeded_feedback.timestamp_us = 123U;
    g_seeded_feedback.sequence = 456U;
    g_seeded_feedback.backend_flags = 789U;
    g_seeded_feedback.actuator_feedback.valid = true;
    g_seeded_feedback.actuator_feedback.sample_time_us = 654U;
    g_seeded_feedback.actuator_feedback.joints[0].id = 3U;
    g_seeded_feedback.actuator_feedback.joints[0].kind = PLATFORM_MOTOR_KIND_JOINT;
    g_seeded_feedback.actuator_feedback.joints[0].torque_est = 4.5f;
    g_seeded_feedback.actuator_feedback.wheels[1].id = 1U;
    g_seeded_feedback.actuator_feedback.wheels[1].kind = PLATFORM_MOTOR_KIND_WHEEL;
    g_seeded_feedback.actuator_feedback.wheels[1].online = true;
    g_feedback_result = PLATFORM_DEVICE_RESULT_TIMEOUT;

    result = platform_actuator_gateway_capture_feedback(&observed_feedback);

    assert(result == PLATFORM_DEVICE_RESULT_TIMEOUT);
    assert(memcmp(&observed_feedback, &g_seeded_feedback, sizeof(observed_feedback)) == 0);
}

static void test_dispatch_maps_contract_motors_into_device_command(void)
{
    platform_actuator_command_t command = build_command_fixture();
    const platform_device_command_t *written_command;

    reset_test_state();
    install_hooks();

    platform_actuator_gateway_dispatch_command(&command, 999U);
    written_command = platform_ports_fake_last_command();

    assert(g_write_call_count == 1U);
    assert_motor_command_matches(&written_command->motors.joints[PLATFORM_JOINT_LEFT_FRONT],
                                 &command.motors.joints[PLATFORM_JOINT_LEFT_FRONT],
                                 true);
    assert_motor_command_matches(&written_command->motors.joints[PLATFORM_JOINT_LEFT_REAR],
                                 &command.motors.joints[PLATFORM_JOINT_LEFT_REAR],
                                 false);
    assert_motor_command_matches(&written_command->motors.joints[PLATFORM_JOINT_RIGHT_FRONT],
                                 &command.motors.joints[PLATFORM_JOINT_RIGHT_FRONT],
                                 true);
    assert_motor_command_matches(&written_command->motors.joints[PLATFORM_JOINT_RIGHT_REAR],
                                 &command.motors.joints[PLATFORM_JOINT_RIGHT_REAR],
                                 true);
    assert_motor_command_matches(&written_command->motors.wheels[PLATFORM_WHEEL_LEFT],
                                 &command.motors.wheels[PLATFORM_WHEEL_LEFT],
                                 true);
    assert_motor_command_matches(&written_command->motors.wheels[PLATFORM_WHEEL_RIGHT],
                                 &command.motors.wheels[PLATFORM_WHEEL_RIGHT],
                                 false);
}

static void test_dispatch_clears_validity_when_actuator_or_control_is_disabled(void)
{
    platform_actuator_command_t command = build_command_fixture();
    const platform_device_command_t *written_command;

    reset_test_state();
    install_hooks();

    command.control_enable = false;
    command.actuator_enable = true;
    assert(command.motors.joints[PLATFORM_JOINT_LEFT_FRONT].valid == true);
    assert(command.motors.joints[PLATFORM_JOINT_RIGHT_FRONT].valid == true);
    assert(command.motors.joints[PLATFORM_JOINT_RIGHT_REAR].valid == true);
    assert(command.motors.wheels[PLATFORM_WHEEL_LEFT].valid == true);

    platform_actuator_gateway_dispatch_command(&command, 1000U);
    written_command = platform_ports_fake_last_command();

    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i) {
        assert(written_command->motors.joints[i].valid == false);
    }
    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i) {
        assert(written_command->motors.wheels[i].valid == false);
    }
}

int main(void)
{
    test_init_calls_default_profile_once();
    test_capture_feedback_forwards_result_and_payload();
    test_dispatch_maps_contract_motors_into_device_command();
    test_dispatch_clears_validity_when_actuator_or_control_is_disabled();

    return 0;
}
