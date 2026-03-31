#include "balance_safety_harness.h"

#include <string.h>

#include "../../../app/balance_chassis/app_io/chassis_topics.h"

void platform_balance_safety_harness_seed_ins(platform_balance_safety_harness_t *harness,
                                              const platform_ins_state_message_t *ins_msg)
{
    if (harness == NULL || harness->ins_pub == NULL || ins_msg == NULL)
    {
        return;
    }

    harness->ins_msg = *ins_msg;
    PubPushMessage(harness->ins_pub, &harness->ins_msg);
}

void platform_balance_safety_harness_init(platform_balance_safety_harness_t *harness)
{
    platform_ins_state_message_t ins_msg = {0};

    if (harness == NULL)
    {
        return;
    }

    memset(harness, 0, sizeof(*harness));
    chassis_runtime_bus_reset_observation();

    harness->ins_pub = PubRegister("ins_data", sizeof(platform_ins_state_message_t));
    remote_task_init(&harness->remote);
    observe_task_init(&harness->observe);
    motor_control_task_init(&harness->motor);
    chassis_task_init(&harness->chassis);

    ins_msg.ready = 1U;
    platform_balance_safety_harness_seed_ins(harness, &ins_msg);

    observe_task_prepare(&harness->observe);
    motor_control_task_prepare(&harness->motor);
    motor_control_task_step(&harness->motor);
    chassis_task_prepare(&harness->chassis);
}

void platform_balance_safety_harness_step_once(platform_balance_safety_harness_t *harness)
{
    if (harness == NULL)
    {
        return;
    }

    remote_task_step(&harness->remote);
    observe_task_step(&harness->observe);
    chassis_task_step(&harness->chassis);
    motor_control_task_step(&harness->motor);
}
