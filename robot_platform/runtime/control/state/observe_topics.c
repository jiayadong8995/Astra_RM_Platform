#include "observe_topics.h"

#include "../readiness.h"

void platform_observe_bus_init(platform_observe_bus_t *bus)
{
    bus->observe_pub = PubRegister("chassis_observe", sizeof(platform_chassis_observe_message_t));
    bus->ins_sub = SubRegister("ins_data", sizeof(platform_ins_state_message_t));
    bus->cmd_sub = SubRegister("robot_intent", sizeof(platform_robot_intent_t));
    bus->feedback_sub = SubRegister("device_feedback", sizeof(platform_device_feedback_t));
}

void platform_observe_bus_wait_ready(platform_observe_bus_t *bus, platform_ins_state_message_t *ins_msg)
{
    platform_readiness_wait_ins(bus->ins_sub, ins_msg);
}

void platform_observe_bus_pull_inputs(platform_observe_bus_t *bus,
                                      platform_robot_intent_t *intent,
                                      platform_device_feedback_t *feedback_msg)
{
    SubGetMessage(bus->cmd_sub, intent);
    SubGetMessage(bus->feedback_sub, feedback_msg);
}

void platform_observe_bus_publish(platform_observe_bus_t *bus,
                                  const platform_chassis_observe_message_t *observe_msg)
{
    PubPushMessage(bus->observe_pub, (void *)observe_msg);
}
