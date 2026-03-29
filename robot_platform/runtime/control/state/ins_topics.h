#ifndef PLATFORM_CONTROL_STATE_INS_TOPICS_H
#define PLATFORM_CONTROL_STATE_INS_TOPICS_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *ins_pub;
} platform_ins_bus_t;

void platform_ins_bus_init(platform_ins_bus_t *bus);
void platform_ins_bus_publish(platform_ins_bus_t *bus, const INS_Data_t *msg);

#endif
