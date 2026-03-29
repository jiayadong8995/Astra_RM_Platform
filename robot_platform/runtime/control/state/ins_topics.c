#include "ins_topics.h"

void platform_ins_bus_init(platform_ins_bus_t *bus)
{
    bus->ins_pub = PubRegister("ins_data", sizeof(platform_ins_state_message_t));
}

void platform_ins_bus_publish(platform_ins_bus_t *bus, const platform_ins_state_message_t *msg)
{
    PubPushMessage(bus->ins_pub, (void *)msg);
}
