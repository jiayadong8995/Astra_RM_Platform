#include "chassis_topics.h"

#include <stdio.h>

#include "cmsis_os.h"

static uint8_t g_actuator_command_observed;

void chassis_runtime_bus_init(Chassis_Runtime_Bus_t *bus)
{
    bus->ins_sub = SubRegister("ins_data", sizeof(platform_ins_state_message_t));
    bus->cmd_sub = SubRegister("robot_intent", sizeof(platform_robot_intent_t));
    bus->observe_sub = SubRegister("chassis_observe", sizeof(platform_chassis_observe_message_t));
    bus->device_feedback_sub = SubRegister("device_feedback", sizeof(platform_device_feedback_t));

    bus->robot_state_pub = PubRegister("robot_state", sizeof(platform_robot_state_t));
    bus->actuator_command_pub = PubRegister("actuator_command", sizeof(platform_actuator_command_t));
}

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus,
                                    platform_ins_state_message_t *ins,
                                    platform_device_feedback_t *feedback)
{
    while (ins->ready == 0U || !feedback->actuator_feedback.valid)
    {
        platform_robot_intent_t intent = {0};
        platform_chassis_observe_message_t observe = {0};
        chassis_runtime_bus_pull_inputs(bus, ins, &intent, &observe, feedback);
        osDelay(1);
    }
}

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus,
                                     platform_ins_state_message_t *ins,
                                     platform_robot_intent_t *intent,
                                     platform_chassis_observe_message_t *observe,
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
    if (g_actuator_command_observed == 0U)
    {
        g_actuator_command_observed = 1U;
        printf("[RuntimeOutput] topic=actuator_command start=%u control_enable=%u actuator_enable=%u\n",
               actuator_command->start ? 1U : 0U,
               actuator_command->control_enable ? 1U : 0U,
               actuator_command->actuator_enable ? 1U : 0U);
        fflush(stdout);
    }
}
