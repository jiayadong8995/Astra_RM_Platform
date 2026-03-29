#include "actuator_topics.h"

#include "cmsis_os.h"

void platform_actuator_bus_init(platform_actuator_bus_t *bus)
{
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->actuator_cmd_sub = SubRegister("actuator_command", sizeof(platform_actuator_command_t));
    bus->device_feedback_pub = PubRegister("device_feedback", sizeof(platform_device_feedback_t));
    bus->actuator_feedback_pub = PubRegister("actuator_feedback", sizeof(Actuator_Feedback_t));
}

void platform_actuator_bus_wait_ready(platform_actuator_bus_t *bus, INS_Data_t *ins_msg)
{
    while (ins_msg->ready == 0U)
    {
        if (SubGetMessage(bus->ins_sub, ins_msg))
        {
            if (ins_msg->ready != 0U)
            {
                break;
            }
        }
        osDelay(1);
    }
}

void platform_actuator_bus_pull_cmd(platform_actuator_bus_t *bus, platform_actuator_command_t *actuator_msg)
{
    SubGetMessage(bus->actuator_cmd_sub, actuator_msg);
}

void platform_actuator_bus_publish_feedback(platform_actuator_bus_t *bus,
                                            const platform_device_feedback_t *device_feedback,
                                            const Actuator_Feedback_t *feedback_msg)
{
    PubPushMessage(bus->device_feedback_pub, (void *)device_feedback);
    PubPushMessage(bus->actuator_feedback_pub, (void *)feedback_msg);
}
