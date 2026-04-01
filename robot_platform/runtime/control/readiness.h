#ifndef PLATFORM_CONTROL_READINESS_H
#define PLATFORM_CONTROL_READINESS_H

#include "contracts/device_feedback.h"
#include "state/ins_state_message.h"
#include "message_center.h"

/**
 * Wait until INS reports ready.
 *
 * Busy-waits on the given subscriber, polling ins_data messages until
 * ins_msg->ready becomes non-zero.  Used by observe and actuator tasks
 * to gate startup until the state estimator is converged.
 */
void platform_readiness_wait_ins(Subscriber_t *ins_sub,
                                 platform_ins_state_message_t *ins_msg);

/**
 * Wait until INS reports ready AND actuator feedback is valid.
 *
 * Busy-waits on both subscribers.  Used by the chassis control task to
 * gate startup until the full sensor + actuator chain is live.
 */
void platform_readiness_wait_ins_and_feedback(Subscriber_t *ins_sub,
                                              Subscriber_t *feedback_sub,
                                              platform_ins_state_message_t *ins_msg,
                                              platform_device_feedback_t *feedback);

#endif
