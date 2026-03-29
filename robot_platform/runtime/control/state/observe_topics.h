#ifndef PLATFORM_CONTROL_STATE_OBSERVE_TOPICS_H
#define PLATFORM_CONTROL_STATE_OBSERVE_TOPICS_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "../contracts/device_feedback.h"
#include "../contracts/robot_intent.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *observe_pub;
    Subscriber_t *ins_sub;
    Subscriber_t *cmd_sub;
    Subscriber_t *feedback_sub;
} platform_observe_bus_t;

void platform_observe_bus_init(platform_observe_bus_t *bus);
void platform_observe_bus_wait_ready(platform_observe_bus_t *bus, INS_Data_t *ins_msg);
void platform_observe_bus_pull_inputs(platform_observe_bus_t *bus,
                                      platform_robot_intent_t *intent,
                                      platform_device_feedback_t *feedback_msg);
void platform_observe_bus_publish(platform_observe_bus_t *bus, const Chassis_Observe_t *observe_msg);

#endif
