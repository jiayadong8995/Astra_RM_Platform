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
#include "../app_config/robot_def.h"
#include "../app_flow/chassis_orchestration.h"
#include "../app_io/chassis_topics.h"

static Chassis_Runtime_Bus_t runtime_bus;
static Chassis_Runtime_State_t runtime_state;
static uint32_t CHASSIS_TIME = CHASSIS_TASK_PERIOD;

void Chassis_task(void)
{
    Chassis_Bus_Input_t inputs = {0};
    Chassis_Bus_Output_t outputs = {0};

    chassis_runtime_bus_init(&runtime_bus);
    chassis_runtime_bus_wait_ready(&runtime_bus, &inputs);

    chassis_runtime_state_init(&runtime_state);
    chassis_runtime_apply_bus_inputs(&runtime_state, &inputs);

    osDelay(6);

	while(1)
	{	
        chassis_runtime_bus_pull_inputs(&runtime_bus, &inputs);
        chassis_runtime_apply_bus_inputs(&runtime_state, &inputs);
        chassis_runtime_step(&runtime_state, &inputs.feedback);
        chassis_runtime_build_bus_outputs(&runtime_state, &outputs);
        chassis_runtime_bus_publish_outputs(&runtime_bus, &outputs);

		osDelay(CHASSIS_TIME);
	}
}
