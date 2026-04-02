#include "observe_task.h"

#include "cmsis_os.h"
#include "app_params.h"
#include "../readiness.h"
#include "../topics.h"
#include "chassis_observer.h"
#include "chassis_observe_message.h"
#include "ins_state_message.h"

void observe_task_init(platform_observe_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_chassis_observer_init(&runtime->runtime);
    runtime->observe_pub = PubRegister(TOPIC_CHASSIS_OBSERVE, sizeof(platform_chassis_observe_message_t));
    runtime->ins_sub = SubRegister(TOPIC_INS_DATA, sizeof(platform_ins_state_message_t));
    runtime->cmd_sub = SubRegister(TOPIC_ROBOT_INTENT, sizeof(platform_robot_intent_t));
    runtime->feedback_sub = SubRegister(TOPIC_DEVICE_FEEDBACK, sizeof(platform_device_feedback_t));
}

void observe_task_prepare(platform_observe_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_readiness_wait_ins(runtime->ins_sub, &runtime->ins_msg);
}

void observe_task_step(platform_observe_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    SubGetMessage(runtime->cmd_sub, &runtime->intent);
    SubGetMessage(runtime->feedback_sub, &runtime->feedback_msg);
    platform_chassis_observer_apply_inputs(&runtime->runtime,
                                           &runtime->intent,
                                           &runtime->feedback_msg,
                                           (float)OBSERVE_TASK_PERIOD_MS / 1000.0f);
    runtime->observe_msg = platform_chassis_observer_build_output(&runtime->runtime);
    PubPushMessage(runtime->observe_pub, (void *)&runtime->observe_msg);
}

void Observe_task(void)
{
    platform_observe_task_runtime_t runtime = {0};

    observe_task_init(&runtime);
    observe_task_prepare(&runtime);

    while(1)
    {
        observe_task_step(&runtime);
        osDelay(OBSERVE_TASK_PERIOD_MS);
    }
}
