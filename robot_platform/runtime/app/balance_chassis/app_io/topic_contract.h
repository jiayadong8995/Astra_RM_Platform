#ifndef BALANCE_CHASSIS_APP_IO_TOPIC_CONTRACT_H
#define BALANCE_CHASSIS_APP_IO_TOPIC_CONTRACT_H

#include "../app_config/robot_def.h"
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

typedef struct
{
    INS_Data_t ins;
    Chassis_Cmd_t cmd;
    Chassis_Observe_t observe;
    Actuator_Feedback_t feedback;
} Chassis_Bus_Input_t;

typedef struct
{
    Chassis_State_t state;
    Leg_Output_t right_leg;
    Leg_Output_t left_leg;
    Actuator_Cmd_t actuator_cmd;
} Chassis_Bus_Output_t;

typedef struct
{
    Publisher_t *cmd_pub;
    Subscriber_t *rc_sub;
    Subscriber_t *ins_sub;
    Subscriber_t *chassis_state_sub;
    Subscriber_t *leg_right_sub;
    Subscriber_t *leg_left_sub;
} Remote_Runtime_Bus_t;

#endif
