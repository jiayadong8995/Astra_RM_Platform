#ifndef PLATFORM_CONTROL_EXECUTION_MOTOR_CONTROL_TASK_H
#define PLATFORM_CONTROL_EXECUTION_MOTOR_CONTROL_TASK_H

#include "../contracts/actuator_command.h"
#include "../contracts/device_feedback.h"
#include "../state/ins_state_message.h"
#include "actuator_topics.h"

typedef struct
{
    platform_actuator_bus_t runtime_bus;
    platform_ins_state_message_t ins_msg;
    platform_actuator_command_t actuator_msg;
    platform_device_feedback_t device_feedback;
    uint32_t systick;
} platform_motor_control_task_runtime_t;

void motor_control_task_init(platform_motor_control_task_runtime_t *runtime);
void motor_control_task_prepare(platform_motor_control_task_runtime_t *runtime);
void motor_control_task_step(platform_motor_control_task_runtime_t *runtime);
void motor_control_task(void);

#endif
