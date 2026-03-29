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
#include "../app_intent/remote_intent.h"
#include "../app_intent/remote_intent_state.h"
#include "../app_io/remote_topics.h"
#include "../../../device/device_layer.h"

void remote_task(void)
{	
    platform_remote_intent_state_t intent_state = {0};
    platform_remote_intent_bus_t intent_bus = {0};
    platform_rc_input_t rc_input = {0};
    platform_robot_state_t robot_state = {0};
    platform_robot_intent_t intent = {0};

    platform_remote_intent_state_init(&intent_state);
    platform_remote_intent_bus_init(&intent_bus);
	while(1)
	{	
        (void)platform_device_read_default_remote(&rc_input);
        platform_remote_intent_bus_pull_inputs(&intent_bus, &robot_state);
        platform_remote_intent_state_apply_inputs(&intent_state, &rc_input, &robot_state);
        intent = platform_remote_intent_build(&intent_state);
        platform_remote_intent_bus_publish(&intent_bus, &intent);

		osDelay(REMOTE_TASK_PERIOD_MS);
	}
}
