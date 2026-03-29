#ifndef BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H

#include "../app_config/robot_def.h"
#include "../../../control/contracts/actuator_command.h"
#include "../../../control/contracts/device_feedback.h"
#include "../../../control/contracts/robot_state.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *robot_state_pub;
    Publisher_t *actuator_command_pub;
    Subscriber_t *ins_sub;
    Subscriber_t *cmd_sub;
    Subscriber_t *observe_sub;
    Subscriber_t *device_feedback_sub;
} Chassis_Runtime_Bus_t;

void chassis_runtime_bus_init(Chassis_Runtime_Bus_t *bus);

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus,
                                    INS_Data_t *ins,
                                    platform_device_feedback_t *feedback);

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus,
                                     INS_Data_t *ins,
                                     Chassis_Cmd_t *cmd,
                                     Chassis_Observe_t *observe,
                                     platform_device_feedback_t *feedback);

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus,
                                         const platform_robot_state_t *robot_state,
                                         const platform_actuator_command_t *actuator_command);

#endif
