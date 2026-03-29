#include "remote_topics.h"

void platform_remote_intent_bus_init(platform_remote_intent_bus_t *bus)
{
    bus->robot_state_sub = SubRegister("robot_state", sizeof(platform_robot_state_t));
    bus->intent_pub = PubRegister("robot_intent", sizeof(platform_robot_intent_t));
}

void platform_remote_intent_bus_pull_inputs(platform_remote_intent_bus_t *bus,
                                            platform_robot_state_t *robot_state)
{
    SubGetMessage(bus->robot_state_sub, robot_state);
}

void platform_remote_intent_bus_publish(platform_remote_intent_bus_t *bus,
                                        const platform_robot_intent_t *intent)
{
    PubPushMessage(bus->intent_pub, (void *)intent);
}
