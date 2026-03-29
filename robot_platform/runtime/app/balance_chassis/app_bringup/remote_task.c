/**
  *********************************************************************
  * @file      remote_task.c/h
  * @brief     该任务是读取并处理遥控数据，
	*            将遥控数据转化为期望的速度、期望的转角、期望的腿长等
  * @note       
  * @history
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  *********************************************************************
  */
	
#include "remote_task.h"
#include "cmsis_os.h"
#include "../app_config/app_params.h"
#include "../app_flow/remote_orchestration.h"
#include "../app_flow/remote_runtime.h"
#include "../app_io/remote_topics.h"
#include "../../../device/device_layer.h"

void remote_task(void)
{	
    Remote_Runtime_t cmd_state = {0};
    Remote_Runtime_Bus_t runtime_bus = {0};
    platform_rc_input_t rc_input = {0};
    platform_robot_state_t robot_state = {0};
    platform_robot_intent_t intent = {0};

    remote_runtime_init(&cmd_state);
    remote_runtime_bus_init(&runtime_bus);
	while(1)
	{	
        (void)platform_device_read_default_remote(&rc_input);
        remote_runtime_bus_pull_inputs(&runtime_bus, &robot_state);
        remote_runtime_apply_inputs(&cmd_state, &rc_input, &robot_state);
        intent = remote_runtime_build_intent(&cmd_state);
        remote_runtime_bus_publish_intent(&runtime_bus, &intent);

		osDelay(REMOTE_TASK_PERIOD_MS);
	}
}
