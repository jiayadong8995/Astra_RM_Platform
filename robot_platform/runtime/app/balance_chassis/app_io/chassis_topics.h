#ifndef BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_CHASSIS_TOPICS_H

#include "../app_config/robot_def.h"
#include "../../../control/controllers/balance_controller.h"
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

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus, platform_balance_controller_input_t *inputs);

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus, platform_balance_controller_input_t *inputs);

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus, const platform_balance_controller_output_t *outputs);

#endif
