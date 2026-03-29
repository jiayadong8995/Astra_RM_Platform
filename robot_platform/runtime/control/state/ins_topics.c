#include "ins_topics.h"

void platform_ins_bus_init(platform_ins_bus_t *bus)
{
    bus->ins_pub = PubRegister("ins_data", sizeof(INS_Data_t));
}

void platform_ins_bus_publish(platform_ins_bus_t *bus, const INS_Data_t *msg)
{
    PubPushMessage(bus->ins_pub, (void *)msg);
}
