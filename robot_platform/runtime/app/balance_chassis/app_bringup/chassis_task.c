/**
  *********************************************************************
  * @file      chassis_task.c/h
  * @brief     该任务计算轮腿模型
  *			   得出输出电流和力矩后发送给电机控制线程
  * @note       
  * @history
  *
  @verbatim
  =====================================================================

  =====================================================================
  @endverbatim
  *********************************************************************
  */
	
	
#include "chassis_task.h"
#include "fdcan.h"
#include "cmsis_os.h"
#include "can_bsp.h"
#include "../app_config/app_params.h"
#include "../app_io/chassis_topics.h"
#include "../../../control/controllers/balance_controller.h"

void chassis_task_init(platform_chassis_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    chassis_runtime_bus_init(&runtime->runtime_bus);
    platform_balance_controller_init(&runtime->runtime_state);
}

void chassis_task_prepare(platform_chassis_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    chassis_runtime_bus_wait_ready(&runtime->runtime_bus, &runtime->inputs.ins, &runtime->inputs.feedback);
    platform_balance_controller_apply_inputs(&runtime->runtime_state, &runtime->inputs);
    osDelay(APP_CHASSIS_STARTUP_DELAY_MS);
}

void chassis_task_step(platform_chassis_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    chassis_runtime_bus_pull_inputs(&runtime->runtime_bus,
                                    &runtime->inputs.ins,
                                    &runtime->inputs.intent,
                                    &runtime->inputs.observe,
                                    &runtime->inputs.feedback);
    platform_balance_controller_apply_inputs(&runtime->runtime_state, &runtime->inputs);
    platform_balance_controller_step(&runtime->runtime_state, &runtime->inputs.feedback);
    platform_balance_controller_build_outputs(&runtime->runtime_state, &runtime->outputs);
    chassis_runtime_bus_publish_outputs(&runtime->runtime_bus,
                                       &runtime->outputs.robot_state,
                                       &runtime->outputs.actuator_command);
}

void Chassis_task(void)
{
    platform_chassis_task_runtime_t runtime = {0};

    chassis_task_init(&runtime);
    chassis_task_prepare(&runtime);

    while(1)
    {
        chassis_task_step(&runtime);
        osDelay(CHASSIS_TASK_PERIOD_MS);
    }
}
