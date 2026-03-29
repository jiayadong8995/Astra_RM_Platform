#ifndef BALANCE_CHASSIS_APP_IO_REMOTE_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_REMOTE_TOPICS_H

#include "../app_config/robot_def.h"
#include "../../../control/contracts/robot_state.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *cmd_pub;
    Subscriber_t *robot_state_sub;
} Remote_Runtime_Bus_t;

void remote_runtime_bus_init(Remote_Runtime_Bus_t *bus);

void remote_runtime_bus_pull_inputs(Remote_Runtime_Bus_t *bus,
                                    platform_robot_state_t *robot_state);

void remote_runtime_bus_publish_cmd(Remote_Runtime_Bus_t *bus, const Chassis_Cmd_t *cmd);

#endif
