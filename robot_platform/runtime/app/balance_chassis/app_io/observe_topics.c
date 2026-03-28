#include "observe_topics.h"

#include "cmsis_os.h"

void observe_runtime_bus_init(Observe_Runtime_Bus_t *bus)
{
    bus->observe_pub = PubRegister("chassis_observe", sizeof(Chassis_Observe_t));
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->cmd_sub = SubRegister("chassis_cmd", sizeof(Chassis_Cmd_t));
    bus->feedback_sub = SubRegister("actuator_feedback", sizeof(Actuator_Feedback_t));
}

void observe_runtime_bus_wait_ready(Observe_Runtime_Bus_t *bus, INS_Data_t *ins_msg)
{
    while (ins_msg->ready == 0U)
    {
        SubGetMessage(bus->ins_sub, ins_msg);
        osDelay(1);
    }
}

void observe_runtime_bus_pull_inputs(Observe_Runtime_Bus_t *bus,
                                     Chassis_Cmd_t *cmd_msg,
                                     Actuator_Feedback_t *feedback_msg)
{
    SubGetMessage(bus->cmd_sub, cmd_msg);
    SubGetMessage(bus->feedback_sub, feedback_msg);
}

void observe_runtime_bus_publish(Observe_Runtime_Bus_t *bus, const Chassis_Observe_t *observe_msg)
{
    PubPushMessage(bus->observe_pub, (void *)observe_msg);
}
