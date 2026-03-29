#ifndef BALANCE_CHASSIS_APP_IO_REMOTE_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_REMOTE_TOPICS_H

#include "../../../control/contracts/robot_intent.h"
#include "../../../control/contracts/robot_state.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *intent_pub;
    Subscriber_t *robot_state_sub;
} platform_remote_intent_bus_t;

void platform_remote_intent_bus_init(platform_remote_intent_bus_t *bus);

void platform_remote_intent_bus_pull_inputs(platform_remote_intent_bus_t *bus,
                                            platform_robot_state_t *robot_state);

void platform_remote_intent_bus_publish(platform_remote_intent_bus_t *bus,
                                        const platform_robot_intent_t *intent);

#endif
