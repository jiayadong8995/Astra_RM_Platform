#include "readiness.h"

#include "cmsis_os.h"

void platform_readiness_wait_ins(Subscriber_t *ins_sub,
                                 platform_ins_state_message_t *ins_msg)
{
    while (ins_msg->ready == 0U)
    {
        SubGetMessage(ins_sub, ins_msg);
        osDelay(1);
    }
}

void platform_readiness_wait_ins_and_feedback(Subscriber_t *ins_sub,
                                              Subscriber_t *feedback_sub,
                                              platform_ins_state_message_t *ins_msg,
                                              platform_device_feedback_t *feedback)
{
    while (ins_msg->ready == 0U || !feedback->actuator_feedback.valid)
    {
        SubGetMessage(ins_sub, ins_msg);
        SubGetMessage(feedback_sub, feedback);
        osDelay(1);
    }
}
