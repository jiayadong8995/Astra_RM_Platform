#include "actuator_topics.h"

#include "cmsis_os.h"

void actuator_runtime_bus_init(Actuator_Runtime_Bus_t *bus)
{
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->actuator_cmd_sub = SubRegister("actuator_cmd", sizeof(Actuator_Cmd_t));
    bus->actuator_feedback_pub = PubRegister("actuator_feedback", sizeof(Actuator_Feedback_t));
}

void actuator_runtime_bus_wait_ready(Actuator_Runtime_Bus_t *bus, INS_Data_t *ins_msg)
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

void actuator_runtime_bus_pull_cmd(Actuator_Runtime_Bus_t *bus, Actuator_Cmd_t *actuator_msg)
{
    SubGetMessage(bus->actuator_cmd_sub, actuator_msg);
}

void actuator_runtime_bus_publish_feedback(Actuator_Runtime_Bus_t *bus, const Actuator_Feedback_t *feedback_msg)
{
    PubPushMessage(bus->actuator_feedback_pub, (void *)feedback_msg);
}
