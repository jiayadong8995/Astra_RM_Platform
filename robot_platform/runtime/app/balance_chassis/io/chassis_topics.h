#ifndef BALANCE_CHASSIS_IO_CHASSIS_TOPICS_H
#define BALANCE_CHASSIS_IO_CHASSIS_TOPICS_H

#include "../chassis_task.h"
#include "../INS_task.h"
#include "../robot_def.h"
#include "VMC_calc.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *chassis_state_pub;
    Publisher_t *leg_right_pub;
    Publisher_t *leg_left_pub;
    Publisher_t *actuator_cmd_pub;
    Subscriber_t *ins_sub;
    Subscriber_t *cmd_sub;
    Subscriber_t *observe_sub;
    Subscriber_t *actuator_feedback_sub;
} Chassis_Runtime_Bus_t;

void chassis_runtime_bus_init(Chassis_Runtime_Bus_t *bus);

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus,
                                    chassis_t *chassis,
                                    INS_t *ins,
                                    INS_Data_t *ins_msg,
                                    Chassis_Cmd_t *cmd_msg,
                                    Chassis_Observe_t *observe_msg,
                                    Actuator_Feedback_t *feedback_msg);

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus,
                                     chassis_t *chassis,
                                     INS_t *ins,
                                     INS_Data_t *ins_msg,
                                     Chassis_Cmd_t *cmd_msg,
                                     Chassis_Observe_t *observe_msg,
                                     Actuator_Feedback_t *feedback_msg);

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus,
                                         const chassis_t *chassis,
                                         const vmc_leg_t *right_leg,
                                         const vmc_leg_t *left_leg);

#endif
