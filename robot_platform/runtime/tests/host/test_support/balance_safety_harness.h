#ifndef PLATFORM_TEST_BALANCE_SAFETY_HARNESS_H
#define PLATFORM_TEST_BALANCE_SAFETY_HARNESS_H

#include "../../../app/balance_chassis/app_bringup/remote_task.h"
#include "../../../control/controllers/chassis_control_task.h"
#include "../../../control/execution/motor_control_task.h"
#include "../../../control/state/ins_state_message.h"
#include "../../../control/state/observe_task.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *ins_pub;
    platform_ins_state_message_t ins_msg;
    platform_remote_task_runtime_t remote;
    platform_observe_task_runtime_t observe;
    platform_chassis_task_runtime_t chassis;
    platform_motor_control_task_runtime_t motor;
} platform_balance_safety_harness_t;

void platform_balance_safety_harness_init(platform_balance_safety_harness_t *harness);
void platform_balance_safety_harness_seed_ins(platform_balance_safety_harness_t *harness,
                                              const platform_ins_state_message_t *ins_msg);
void platform_balance_safety_harness_step_once(platform_balance_safety_harness_t *harness);

#endif
