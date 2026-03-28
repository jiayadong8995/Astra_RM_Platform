#ifndef BALANCE_CHASSIS_APP_IO_INS_TOPICS_H
#define BALANCE_CHASSIS_APP_IO_INS_TOPICS_H

#include "topic_contract.h"

typedef struct
{
    Publisher_t *ins_pub;
} INS_Runtime_Bus_t;

void ins_runtime_bus_init(INS_Runtime_Bus_t *bus);

void ins_runtime_bus_publish(INS_Runtime_Bus_t *bus, const INS_Data_t *msg);

#endif
