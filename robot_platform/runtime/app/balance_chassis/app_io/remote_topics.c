#include "remote_topics.h"

void remote_runtime_bus_init(Remote_Runtime_Bus_t *bus)
{
    bus->robot_state_sub = SubRegister("robot_state", sizeof(platform_robot_state_t));
    bus->cmd_pub = PubRegister("chassis_cmd", sizeof(Chassis_Cmd_t));
}

void remote_runtime_bus_pull_inputs(Remote_Runtime_Bus_t *bus,
                                    platform_robot_state_t *robot_state)
{
    SubGetMessage(bus->robot_state_sub, robot_state);
}

void remote_runtime_bus_publish_cmd(Remote_Runtime_Bus_t *bus, const Chassis_Cmd_t *cmd)
{
    PubPushMessage(bus->cmd_pub, (void *)cmd);
}
