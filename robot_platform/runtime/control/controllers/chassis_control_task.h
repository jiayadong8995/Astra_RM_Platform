#ifndef PLATFORM_CONTROL_CONTROLLERS_CHASSIS_CONTROL_TASK_H
#define PLATFORM_CONTROL_CONTROLLERS_CHASSIS_CONTROL_TASK_H

#include "../../app/balance_chassis/app_io/chassis_topics.h"
#include "balance_controller.h"

typedef struct
{
    Chassis_Runtime_Bus_t runtime_bus;
    platform_balance_controller_t runtime_state;
    platform_balance_controller_input_t inputs;
    platform_balance_controller_output_t outputs;
} platform_chassis_task_runtime_t;

void chassis_task_init(platform_chassis_task_runtime_t *runtime);
void chassis_task_prepare(platform_chassis_task_runtime_t *runtime);
void chassis_task_step(platform_chassis_task_runtime_t *runtime);
void platform_chassis_control_task(void);

#endif
