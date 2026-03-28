/**
  *********************************************************************
  * @file      observe_task.c/h
  * @brief     该任务是对机体运动速度估计，用于抑制打滑
  * @note       
  * @history
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  *********************************************************************
  */
	
#include "observe_task.h"
#include "cmsis_os.h"
#include "../app_config/app_params.h"
#include "../app_config/robot_def.h"
#include "../app_io/observe_topics.h"
#include "../app_flow/observe_orchestration.h"
#include "../app_flow/observe_runtime.h"

void Observe_task(void)
{
    Observe_Runtime_t runtime = {0};
    Observe_Runtime_Bus_t runtime_bus = {0};
    INS_Data_t ins_msg = {0};
    Chassis_Cmd_t cmd_msg = {0};
    Actuator_Feedback_t feedback_msg = {0};
    Chassis_Observe_t observe_msg = {0};

    observe_runtime_init(&runtime);
    observe_runtime_bus_init(&runtime_bus);
    observe_runtime_bus_wait_ready(&runtime_bus, &ins_msg);

	while(1)
	{
        observe_runtime_bus_pull_inputs(&runtime_bus, &cmd_msg, &feedback_msg);
        observe_runtime_apply_inputs(&runtime, &cmd_msg, &feedback_msg, (float)OBSERVE_TASK_PERIOD_MS / 1000.0f);
        observe_msg = observe_runtime_build_output(&runtime);
        observe_runtime_bus_publish(&runtime_bus, &observe_msg);

		osDelay(OBSERVE_TASK_PERIOD_MS);
	}
}
