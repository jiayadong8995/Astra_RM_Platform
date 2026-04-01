#include "chassis_topics.h"

#include <stdio.h>

#include "../../../control/readiness.h"

static uint8_t g_actuator_command_observed;
static uint32_t g_actuator_command_observation_count;
static platform_actuator_command_t g_first_actuator_command;
static platform_actuator_command_t g_latest_actuator_command;

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
    platform_readiness_wait_ins_and_feedback(bus->ins_sub, bus->device_feedback_sub, ins, feedback);
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
    g_latest_actuator_command = *actuator_command;
    g_actuator_command_observation_count += 1U;
    if (g_actuator_command_observed == 0U)
    {
        g_actuator_command_observed = 1U;
        g_first_actuator_command = *actuator_command;
        printf("[RuntimeOutput] topic=actuator_command sample_count=%lu start=%u control_enable=%u actuator_enable=%u\n",
               (unsigned long)g_actuator_command_observation_count,
               actuator_command->start ? 1U : 0U,
               actuator_command->control_enable ? 1U : 0U,
               actuator_command->actuator_enable ? 1U : 0U);
        fflush(stdout);
    }
}

void chassis_runtime_bus_reset_observation(void)
{
    g_actuator_command_observed = 0U;
    g_actuator_command_observation_count = 0U;
    g_first_actuator_command = (platform_actuator_command_t){0};
    g_latest_actuator_command = (platform_actuator_command_t){0};
}

uint32_t chassis_runtime_bus_observation_count(void)
{
    return g_actuator_command_observation_count;
}

bool chassis_runtime_bus_get_first_observation(platform_actuator_command_t *actuator_command)
{
    if (actuator_command == NULL || g_actuator_command_observation_count == 0U)
    {
        return false;
    }

    *actuator_command = g_first_actuator_command;
    return true;
}

bool chassis_runtime_bus_get_latest_observation(platform_actuator_command_t *actuator_command)
{
    if (actuator_command == NULL || g_actuator_command_observation_count == 0U)
    {
        return false;
    }

    *actuator_command = g_latest_actuator_command;
    return true;
}
