#ifndef PLATFORM_CONTROL_CONTROLLERS_CHASSIS_CONTROL_TASK_H
#define PLATFORM_CONTROL_CONTROLLERS_CHASSIS_CONTROL_TASK_H

#include "balance_controller.h"
#include "message_center.h"

typedef struct
{
    Publisher_t *robot_state_pub;
    Publisher_t *actuator_command_pub;
    Subscriber_t *ins_sub;
    Subscriber_t *cmd_sub;
    Subscriber_t *observe_sub;
    Subscriber_t *device_feedback_sub;
    platform_balance_controller_t runtime_state;
    platform_balance_controller_input_t inputs;
    platform_balance_controller_output_t outputs;
} platform_chassis_task_runtime_t;

void chassis_task_init(platform_chassis_task_runtime_t *runtime);
void chassis_task_prepare(platform_chassis_task_runtime_t *runtime);
void chassis_task_step(platform_chassis_task_runtime_t *runtime);
void platform_chassis_control_task(void);

#endif
