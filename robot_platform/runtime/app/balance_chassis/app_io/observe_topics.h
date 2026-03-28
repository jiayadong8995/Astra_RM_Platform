#ifndef BALANCE_CHASSIS_APP_IO_OBSERVE_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_OBSERVE_TOPICS_H

#include "topic_contract.h"

typedef struct
{
    Publisher_t *observe_pub;
    Subscriber_t *ins_sub;
    Subscriber_t *cmd_sub;
    Subscriber_t *feedback_sub;
} Observe_Runtime_Bus_t;

void observe_runtime_bus_init(Observe_Runtime_Bus_t *bus);

void observe_runtime_bus_wait_ready(Observe_Runtime_Bus_t *bus, INS_Data_t *ins_msg);

void observe_runtime_bus_pull_inputs(Observe_Runtime_Bus_t *bus,
                                     Chassis_Cmd_t *cmd_msg,
                                     Actuator_Feedback_t *feedback_msg);

void observe_runtime_bus_publish(Observe_Runtime_Bus_t *bus, const Chassis_Observe_t *observe_msg);

#endif
