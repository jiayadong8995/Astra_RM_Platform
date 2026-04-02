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
#include "remote_intent.h"
#include "remote_intent_state.h"
#include "../../../control/topics.h"
#include "../../../bsp/ports.h"
#include <string.h>

void remote_task_init(platform_remote_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_remote_intent_state_init(&runtime->intent_state);
    runtime->robot_state_sub = SubRegister(TOPIC_ROBOT_STATE, sizeof(platform_robot_state_t));
    runtime->intent_pub = PubRegister(TOPIC_ROBOT_INTENT, sizeof(platform_robot_intent_t));
}

void remote_task_step(platform_remote_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    runtime->rc_result = platform_remote_read(&runtime->rc_input);
    if (runtime->rc_result != PLATFORM_DEVICE_RESULT_OK)
    {
        memset(&runtime->rc_input, 0, sizeof(runtime->rc_input));
        runtime->rc_input.valid = false;
    }
    SubGetMessage(runtime->robot_state_sub, &runtime->robot_state);
    platform_remote_intent_state_apply_inputs(&runtime->intent_state, &runtime->rc_input, &runtime->robot_state);
    runtime->intent = platform_remote_intent_build(&runtime->intent_state);
    PubPushMessage(runtime->intent_pub, (void *)&runtime->intent);
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
