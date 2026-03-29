#include "observe_topics.h"

#include "cmsis_os.h"

void platform_observe_bus_init(platform_observe_bus_t *bus)
{
    bus->observe_pub = PubRegister("chassis_observe", sizeof(Chassis_Observe_t));
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->cmd_sub = SubRegister("robot_intent", sizeof(platform_robot_intent_t));
    bus->feedback_sub = SubRegister("device_feedback", sizeof(platform_device_feedback_t));
}

void platform_observe_bus_wait_ready(platform_observe_bus_t *bus, INS_Data_t *ins_msg)
{
    while (ins_msg->ready == 0U)
    {
        SubGetMessage(bus->ins_sub, ins_msg);
        osDelay(1);
    }
}

void platform_observe_bus_pull_inputs(platform_observe_bus_t *bus,
                                      platform_robot_intent_t *intent,
                                      platform_device_feedback_t *feedback_msg)
{
    SubGetMessage(bus->cmd_sub, intent);
    SubGetMessage(bus->feedback_sub, feedback_msg);
}

void platform_observe_bus_publish(platform_observe_bus_t *bus, const Chassis_Observe_t *observe_msg)
{
    PubPushMessage(bus->observe_pub, (void *)observe_msg);
}
