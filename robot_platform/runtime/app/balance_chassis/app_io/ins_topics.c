#include "ins_topics.h"

void ins_runtime_bus_init(INS_Runtime_Bus_t *bus)
{
    bus->ins_pub = PubRegister("ins_data", sizeof(INS_Data_t));
}

void ins_runtime_bus_publish(INS_Runtime_Bus_t *bus, const INS_Data_t *msg)
{
    PubPushMessage(bus->ins_pub, (void *)msg);
}
