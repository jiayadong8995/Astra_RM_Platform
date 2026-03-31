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

void remote_task_init(platform_remote_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_remote_intent_state_init(&runtime->intent_state);
    platform_remote_intent_bus_init(&runtime->intent_bus);
}

void remote_task_step(platform_remote_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    (void)platform_device_read_default_remote(&runtime->rc_input);
    platform_remote_intent_bus_pull_inputs(&runtime->intent_bus, &runtime->robot_state);
    platform_remote_intent_state_apply_inputs(&runtime->intent_state, &runtime->rc_input, &runtime->robot_state);
    runtime->intent = platform_remote_intent_build(&runtime->intent_state);
    platform_remote_intent_bus_publish(&runtime->intent_bus, &runtime->intent);
}

void remote_task(void)
{
    platform_remote_task_runtime_t runtime = {0};

    remote_task_init(&runtime);
    while(1)
    {
        remote_task_step(&runtime);
        osDelay(REMOTE_TASK_PERIOD_MS);
    }
}
