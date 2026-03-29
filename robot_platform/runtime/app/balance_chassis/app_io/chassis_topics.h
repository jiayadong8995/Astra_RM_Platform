#ifndef BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H

#include "../../../control/contracts/actuator_command.h"
#include "../../../control/contracts/device_feedback.h"
#include "../../../control/contracts/robot_intent.h"
#include "../../../control/contracts/robot_state.h"
#include "../../../control/state/chassis_observe_message.h"
#include "../../../control/state/ins_state_message.h"
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
                                    platform_ins_state_message_t *ins,
                                    platform_device_feedback_t *feedback);

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus,
                                     platform_ins_state_message_t *ins,
                                     platform_robot_intent_t *intent,
                                     platform_chassis_observe_message_t *observe,
                                     platform_device_feedback_t *feedback);

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus,
                                         const platform_robot_state_t *robot_state,
                                         const platform_actuator_command_t *actuator_command);

#endif
