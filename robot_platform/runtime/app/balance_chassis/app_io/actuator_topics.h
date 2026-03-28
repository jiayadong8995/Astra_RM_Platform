#ifndef BALANCE_CHASSIS_APP_IO_ACTUATOR_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_ACTUATOR_TOPICS_H

#include "topic_contract.h"

typedef struct
{
    Subscriber_t *ins_sub;
    Subscriber_t *actuator_cmd_sub;
    Publisher_t *actuator_feedback_pub;
} Actuator_Runtime_Bus_t;

void actuator_runtime_bus_init(Actuator_Runtime_Bus_t *bus);

void actuator_runtime_bus_wait_ready(Actuator_Runtime_Bus_t *bus, INS_Data_t *ins_msg);

void actuator_runtime_bus_pull_cmd(Actuator_Runtime_Bus_t *bus, Actuator_Cmd_t *actuator_msg);

void actuator_runtime_bus_publish_feedback(Actuator_Runtime_Bus_t *bus, const Actuator_Feedback_t *feedback_msg);

#endif
