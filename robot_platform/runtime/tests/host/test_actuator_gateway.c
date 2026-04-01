#include <assert.h>
#include <string.h>

#include "actuator_gateway.h"
#include "device_layer_stubs.h"

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
    platform_test_device_layer_stubs_reset();

    platform_actuator_gateway_init();

    assert(platform_test_device_layer_get_init_call_count() == 1U);
    assert(platform_test_device_layer_get_write_call_count() == 0U);
}

static void test_capture_feedback_forwards_result_and_payload(void)
{
    platform_device_feedback_t expected_feedback = {0};
    platform_device_feedback_t observed_feedback = {0};
    platform_device_result_t result;

    platform_test_device_layer_stubs_reset();
    expected_feedback.timestamp_us = 123U;
    expected_feedback.sequence = 456U;
    expected_feedback.backend_flags = 789U;
    expected_feedback.actuator_feedback.valid = true;
    expected_feedback.actuator_feedback.sample_time_us = 654U;
    expected_feedback.actuator_feedback.joints[0].id = 3U;
    expected_feedback.actuator_feedback.joints[0].kind = PLATFORM_MOTOR_KIND_JOINT;
    expected_feedback.actuator_feedback.joints[0].torque_est = 4.5f;
    expected_feedback.actuator_feedback.wheels[1].id = 1U;
    expected_feedback.actuator_feedback.wheels[1].kind = PLATFORM_MOTOR_KIND_WHEEL;
    expected_feedback.actuator_feedback.wheels[1].online = true;

    platform_test_device_layer_seed_feedback(&expected_feedback, PLATFORM_DEVICE_RESULT_TIMEOUT);

    result = platform_actuator_gateway_capture_feedback(&observed_feedback);

    assert(result == PLATFORM_DEVICE_RESULT_TIMEOUT);
    assert(memcmp(&observed_feedback, &expected_feedback, sizeof(observed_feedback)) == 0);
}

static void test_dispatch_maps_contract_motors_into_device_command(void)
{
    platform_actuator_command_t command = build_command_fixture();
    const platform_device_command_t *written_command;

    platform_test_device_layer_stubs_reset();

    platform_actuator_gateway_dispatch_command(&command, 999U);
    written_command = platform_test_device_layer_get_last_command();

    assert(platform_test_device_layer_get_write_call_count() == 1U);
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

    platform_test_device_layer_stubs_reset();
    command.control_enable = false;
    command.actuator_enable = true;
    assert(command.motors.joints[PLATFORM_JOINT_LEFT_FRONT].valid == true);
    assert(command.motors.joints[PLATFORM_JOINT_RIGHT_FRONT].valid == true);
    assert(command.motors.joints[PLATFORM_JOINT_RIGHT_REAR].valid == true);
    assert(command.motors.wheels[PLATFORM_WHEEL_LEFT].valid == true);

    platform_actuator_gateway_dispatch_command(&command, 1000U);
    written_command = platform_test_device_layer_get_last_command();

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
