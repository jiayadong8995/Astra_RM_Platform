#ifndef PLATFORM_CONTROL_TOPICS_H
#define PLATFORM_CONTROL_TOPICS_H

/**
 * Centralized topic name constants for the message_center pub/sub bus.
 *
 * Every task that registers a publisher or subscriber should use these
 * constants instead of bare string literals so that topic names are
 * defined in exactly one place.
 */

#define TOPIC_INS_DATA        "ins_data"
#define TOPIC_ROBOT_INTENT    "robot_intent"
#define TOPIC_CHASSIS_OBSERVE "chassis_observe"
#define TOPIC_DEVICE_FEEDBACK "device_feedback"
#define TOPIC_ROBOT_STATE     "robot_state"
#define TOPIC_ACTUATOR_CMD    "actuator_command"

#endif
