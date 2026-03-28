#ifndef BALANCE_CHASSIS_IO_REMOTE_TOPICS_H
#define BALANCE_CHASSIS_IO_REMOTE_TOPICS_H

#include "../control/remote_runtime.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *cmd_pub;
    Subscriber_t *rc_sub;
    Subscriber_t *ins_sub;
    Subscriber_t *chassis_state_sub;
    Subscriber_t *leg_right_sub;
    Subscriber_t *leg_left_sub;
} Remote_Runtime_Bus_t;

void remote_runtime_bus_init(Remote_Runtime_Bus_t *bus);

void remote_runtime_bus_pull_inputs(Remote_Runtime_Bus_t *bus,
                                    RC_Data_t *rc_msg,
                                    INS_Data_t *ins_msg,
                                    Chassis_State_t *state_msg,
                                    Leg_Output_t *right_msg,
                                    Leg_Output_t *left_msg);

void remote_runtime_bus_publish_cmd(Remote_Runtime_Bus_t *bus, const Chassis_Cmd_t *cmd);

#endif
