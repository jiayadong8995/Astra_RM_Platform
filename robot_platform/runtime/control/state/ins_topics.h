#ifndef PLATFORM_CONTROL_STATE_INS_TOPICS_H
#define PLATFORM_CONTROL_STATE_INS_TOPICS_H

#include "message_center.h"
#include "ins_state_message.h"

typedef struct
{
    Publisher_t *ins_pub;
} platform_ins_bus_t;

void platform_ins_bus_init(platform_ins_bus_t *bus);
void platform_ins_bus_publish(platform_ins_bus_t *bus, const platform_ins_state_message_t *msg);

#endif
