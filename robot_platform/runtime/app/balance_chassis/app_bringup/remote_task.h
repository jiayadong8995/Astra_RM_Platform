#ifndef __REMOTE_TASK_H
#define __REMOTE_TASK_H

#include "../../../control/contracts/device_input.h"
#include "../../../control/contracts/robot_intent.h"
#include "../../../control/contracts/robot_state.h"
#include "../../../bsp/device_types.h"
#include "../app_intent/remote_intent_state.h"
#include "message_center.h"

typedef struct
{
    platform_remote_intent_state_t intent_state;
    Publisher_t *intent_pub;
    Subscriber_t *robot_state_sub;
    platform_rc_input_t rc_input;
    platform_device_result_t rc_result;
    platform_robot_state_t robot_state;
    platform_robot_intent_t intent;
} platform_remote_task_runtime_t;

void remote_task_init(platform_remote_task_runtime_t *runtime);
void remote_task_step(platform_remote_task_runtime_t *runtime);
void remote_task(void);

#endif
