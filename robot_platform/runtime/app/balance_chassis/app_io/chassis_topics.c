#include "chassis_topics.h"

#include "cmsis_os.h"

void chassis_runtime_bus_init(Chassis_Runtime_Bus_t *bus)
{
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->cmd_sub = SubRegister("chassis_cmd", sizeof(Chassis_Cmd_t));
    bus->observe_sub = SubRegister("chassis_observe", sizeof(Chassis_Observe_t));
    bus->device_feedback_sub = SubRegister("device_feedback", sizeof(platform_device_feedback_t));

    bus->robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));
    bus->chassis_state_pub = PubRegister("chassis_state", sizeof(Chassis_State_t));
    bus->leg_right_pub = PubRegister("leg_right", sizeof(Leg_Output_t));
    bus->leg_left_pub = PubRegister("leg_left", sizeof(Leg_Output_t));
    bus->actuator_command_pub = PubRegister("actuator_command", sizeof(platform_actuator_command_t));
    bus->actuator_cmd_pub = PubRegister("actuator_cmd", sizeof(Actuator_Cmd_t));
}

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus,
                                    INS_Data_t *ins,
                                    platform_device_feedback_t *feedback)
{
    while (ins->ready == 0U || !feedback->actuator_feedback.valid)
    {
        Chassis_Cmd_t cmd = {0};
        Chassis_Observe_t observe = {0};
        chassis_runtime_bus_pull_inputs(bus, ins, &cmd, &observe, feedback);
        osDelay(1);
    }
}

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus,
                                     INS_Data_t *ins,
                                     Chassis_Cmd_t *cmd,
                                     Chassis_Observe_t *observe,
                                     platform_device_feedback_t *feedback)
{
    SubGetMessage(bus->ins_sub, ins);
    SubGetMessage(bus->cmd_sub, cmd);
    SubGetMessage(bus->observe_sub, observe);
    SubGetMessage(bus->device_feedback_sub, feedback);
}

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus,
                                         const platform_robot_state_t *robot_state,
                                         const Chassis_State_t *state,
                                         const Leg_Output_t *right_leg,
                                         const Leg_Output_t *left_leg,
                                         const platform_actuator_command_t *actuator_command,
                                         const Actuator_Cmd_t *actuator_cmd)
{
    PubPushMessage(bus->robot_state_pub, (void *)robot_state);
    PubPushMessage(bus->chassis_state_pub, (void *)state);
    PubPushMessage(bus->leg_right_pub, (void *)right_leg);
    PubPushMessage(bus->leg_left_pub, (void *)left_leg);
    PubPushMessage(bus->actuator_command_pub, (void *)actuator_command);
    PubPushMessage(bus->actuator_cmd_pub, (void *)actuator_cmd);
}
