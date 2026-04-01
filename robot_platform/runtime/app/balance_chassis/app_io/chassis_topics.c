#include "chassis_topics.h"

#include <stdio.h>

static uint8_t g_actuator_command_observed;
static uint32_t g_actuator_command_observation_count;
static platform_actuator_command_t g_first_actuator_command;
static platform_actuator_command_t g_latest_actuator_command;

void chassis_observation_on_publish(const platform_actuator_command_t *actuator_command)
{
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
