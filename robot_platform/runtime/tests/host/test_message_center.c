#include <assert.h>
#include <stdint.h>

#include "message_center.h"

typedef struct
{
    uint32_t counter;
    float duty_cycle;
} test_message_t;

int main(void)
{
    const test_message_t expected = {
        .counter = 7U,
        .duty_cycle = 0.25f,
    };
    test_message_t observed = {0};

    Publisher_t *publisher = PubRegister("message_center_bootstrap", sizeof(expected));
    Subscriber_t *subscriber = SubRegister("message_center_bootstrap", sizeof(expected));

    assert(publisher != 0);
    assert(subscriber != 0);
    assert(PubPushMessage(publisher, (void *)&expected) == 1U);
    assert(SubGetMessage(subscriber, &observed) == 1U);
    assert(observed.counter == expected.counter);
    assert(observed.duty_cycle == expected.duty_cycle);
    assert(SubGetMessage(subscriber, &observed) == 0U);

    return 0;
}
