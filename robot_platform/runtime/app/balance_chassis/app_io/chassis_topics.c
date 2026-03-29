#include "chassis_topics.h"

#include "cmsis_os.h"

void chassis_runtime_bus_init(Chassis_Runtime_Bus_t *bus)
{
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->cmd_sub = SubRegister("robot_intent", sizeof(platform_robot_intent_t));
    bus->observe_sub = SubRegister("chassis_observe", sizeof(Chassis_Observe_t));
    bus->device_feedback_sub = SubRegister("device_feedback", sizeof(platform_device_feedback_t));

    bus->robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));
    bus->actuator_command_pub = PubRegister("actuator_command", sizeof(platform_actuator_command_t));
}

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus,
                                    INS_Data_t *ins,
                                    platform_device_feedback_t *feedback)
{
    while (ins->ready == 0U || !feedback->actuator_feedback.valid)
    {
        platform_robot_intent_t intent = {0};
        Chassis_Observe_t observe = {0};
        chassis_runtime_bus_pull_inputs(bus, ins, &intent, &observe, feedback);
        osDelay(1);
    }
}

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus,
                                     INS_Data_t *ins,
                                     platform_robot_intent_t *intent,
                                     Chassis_Observe_t *observe,
                                     platform_device_feedback_t *feedback)
{
    SubGetMessage(bus->ins_sub, ins);
    SubGetMessage(bus->cmd_sub, intent);
    SubGetMessage(bus->observe_sub, observe);
    SubGetMessage(bus->device_feedback_sub, feedback);
}

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus,
                                         const platform_robot_state_t *robot_state,
                                         const platform_actuator_command_t *actuator_command)
{
    PubPushMessage(bus->robot_state_pub, (void *)robot_state);
    PubPushMessage(bus->actuator_command_pub, (void *)actuator_command);
}
