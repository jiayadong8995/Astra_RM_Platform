#ifndef PLATFORM_CONTROL_STATE_OBSERVE_TASK_H
#define PLATFORM_CONTROL_STATE_OBSERVE_TASK_H

#include "../contracts/device_feedback.h"
#include "../contracts/robot_intent.h"
#include "chassis_observe_message.h"
#include "chassis_observer.h"
#include "ins_state_message.h"
#include "message_center.h"

typedef struct
{
    platform_chassis_observer_t runtime;
    Publisher_t *observe_pub;
    Subscriber_t *ins_sub;
    Subscriber_t *cmd_sub;
    Subscriber_t *feedback_sub;
    platform_ins_state_message_t ins_msg;
    platform_robot_intent_t intent;
    platform_device_feedback_t feedback_msg;
    platform_chassis_observe_message_t observe_msg;
} platform_observe_task_runtime_t;

void observe_task_init(platform_observe_task_runtime_t *runtime);
void observe_task_prepare(platform_observe_task_runtime_t *runtime);
void observe_task_step(platform_observe_task_runtime_t *runtime);
void Observe_task(void);

#endif
