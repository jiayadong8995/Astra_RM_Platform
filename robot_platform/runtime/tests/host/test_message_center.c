#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "actuator_command.h"
#include "device_feedback.h"
#include "device_input.h"
#include "message_center.h"
#include "robot_state.h"

_Static_assert(sizeof(platform_device_input_t) > UINT8_MAX,
               "platform_device_input_t must stay above the legacy uint8_t size ceiling");

static void fill_bytes(void *payload, size_t payload_size, uint8_t seed)
{
    uint8_t *bytes = (uint8_t *)payload;

    for (size_t i = 0; i < payload_size; ++i) {
        bytes[i] = (uint8_t)(seed + (uint8_t)i);
    }
}

static void expect_publish_round_trip(const char *topic_name,
                                      size_t topic_size,
                                      void *expected_payload,
                                      void *observed_payload)
{
    Publisher_t *publisher = PubRegister((char *)topic_name, topic_size);
    Subscriber_t *subscriber = SubRegister((char *)topic_name, topic_size);

    assert(publisher != NULL);
    assert(subscriber != NULL);
    assert(PubPushMessage(publisher, expected_payload) == 1U);
    assert(SubGetMessage(subscriber, observed_payload) == 1U);
    assert(memcmp(observed_payload, expected_payload, topic_size) == 0);
    assert(SubGetMessage(subscriber, observed_payload) == 0U);
}

static void test_duplicate_publisher_mismatched_size_is_rejected(void)
{
    Publisher_t *publisher = PubRegister("topic_pub_mismatch", sizeof(platform_robot_state_t));

    assert(publisher != NULL);
    assert(PubRegister("topic_pub_mismatch", sizeof(platform_actuator_command_t)) == NULL);
}

static void test_subscriber_mismatched_size_is_rejected(void)
{
    Publisher_t *publisher = PubRegister("topic_sub_mismatch", sizeof(platform_robot_state_t));

    assert(publisher != NULL);
    assert(SubRegister("topic_sub_mismatch", sizeof(platform_actuator_command_t)) == NULL);
}

static void test_large_runtime_contracts_round_trip(void)
{
    platform_robot_state_t robot_state_expected = {0};
    platform_robot_state_t robot_state_observed = {0};
    platform_actuator_command_t actuator_expected = {0};
    platform_actuator_command_t actuator_observed = {0};
    platform_device_feedback_t feedback_expected = {0};
    platform_device_feedback_t feedback_observed = {0};
    platform_device_input_t input_expected = {0};
    platform_device_input_t input_observed = {0};

    fill_bytes(&robot_state_expected, sizeof(robot_state_expected), 0x11U);
    fill_bytes(&actuator_expected, sizeof(actuator_expected), 0x33U);
    fill_bytes(&feedback_expected, sizeof(feedback_expected), 0x55U);
    fill_bytes(&input_expected, sizeof(input_expected), 0x77U);

    expect_publish_round_trip("topic_robot_state",
                              sizeof(robot_state_expected),
                              &robot_state_expected,
                              &robot_state_observed);
    expect_publish_round_trip("topic_actuator_command",
                              sizeof(actuator_expected),
                              &actuator_expected,
                              &actuator_observed);
    expect_publish_round_trip("topic_device_feedback",
                              sizeof(feedback_expected),
                              &feedback_expected,
                              &feedback_observed);
    expect_publish_round_trip("topic_device_input",
                              sizeof(input_expected),
                              &input_expected,
                              &input_observed);
}

static void test_null_publish_inputs_fail(void)
{
    Publisher_t *publisher = PubRegister("topic_null_publish", sizeof(platform_robot_state_t));
    platform_robot_state_t payload = {0};

    assert(publisher != NULL);
    assert(PubPushMessage(NULL, &payload) == 0U);
    assert(PubPushMessage(publisher, NULL) == 0U);
}

int main(void)
{
    test_duplicate_publisher_mismatched_size_is_rejected();
    test_subscriber_mismatched_size_is_rejected();
    test_large_runtime_contracts_round_trip();
    test_null_publish_inputs_fail();

    return 0;
}
