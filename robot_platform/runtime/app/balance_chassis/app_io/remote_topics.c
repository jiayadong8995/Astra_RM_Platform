#include "remote_topics.h"

void remote_runtime_bus_init(Remote_Runtime_Bus_t *bus)
{
    bus->robot_state_sub = SubRegister("robot_state", sizeof(platform_robot_state_t));
    bus->cmd_pub = PubRegister("robot_intent", sizeof(platform_robot_intent_t));
}

void remote_runtime_bus_pull_inputs(Remote_Runtime_Bus_t *bus,
                                    platform_robot_state_t *robot_state)
{
    SubGetMessage(bus->robot_state_sub, robot_state);
}

void remote_runtime_bus_publish_intent(Remote_Runtime_Bus_t *bus, const platform_robot_intent_t *intent)
{
    PubPushMessage(bus->cmd_pub, (void *)intent);
}
