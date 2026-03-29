#ifndef PLATFORM_CONTROL_EXECUTION_ACTUATOR_TOPICS_H
#define PLATFORM_CONTROL_EXECUTION_ACTUATOR_TOPICS_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "message_center.h"

typedef struct
{
    Subscriber_t *ins_sub;
    Subscriber_t *actuator_cmd_sub;
    Publisher_t *actuator_feedback_pub;
} platform_actuator_bus_t;

void platform_actuator_bus_init(platform_actuator_bus_t *bus);
void platform_actuator_bus_wait_ready(platform_actuator_bus_t *bus, INS_Data_t *ins_msg);
void platform_actuator_bus_pull_cmd(platform_actuator_bus_t *bus, Actuator_Cmd_t *actuator_msg);
void platform_actuator_bus_publish_feedback(platform_actuator_bus_t *bus, const Actuator_Feedback_t *feedback_msg);

#endif
