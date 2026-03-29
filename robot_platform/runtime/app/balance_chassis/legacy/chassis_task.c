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
#include "../app_config/robot_def.h"
#include "../app_io/chassis_topics.h"
#include "../../../control/controllers/balance_controller.h"

static Chassis_Runtime_Bus_t runtime_bus;
static platform_balance_controller_t runtime_state;

void Chassis_task(void)
{
    platform_balance_controller_input_t inputs = {0};
    platform_balance_controller_output_t outputs = {0};

    chassis_runtime_bus_init(&runtime_bus);
    chassis_runtime_bus_wait_ready(&runtime_bus, &inputs);

    platform_balance_controller_init(&runtime_state);
    platform_balance_controller_apply_inputs(&runtime_state, &inputs);

    osDelay(APP_CHASSIS_STARTUP_DELAY_MS);

	while(1)
	{	
        chassis_runtime_bus_pull_inputs(&runtime_bus, &inputs);
        platform_balance_controller_apply_inputs(&runtime_state, &inputs);
        platform_balance_controller_step(&runtime_state, &inputs.feedback);
        platform_balance_controller_build_outputs(&runtime_state, &outputs);
        chassis_runtime_bus_publish_outputs(&runtime_bus, &outputs);

		osDelay(CHASSIS_TASK_PERIOD_MS);
	}
}
