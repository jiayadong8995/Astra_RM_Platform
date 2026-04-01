#include "observe_task.h"

#include "cmsis_os.h"
#include "../control_config/control_task_params.h"
#include "chassis_observer.h"
#include "chassis_observe_message.h"
#include "ins_state_message.h"
#include "observe_topics.h"

void observe_task_init(platform_observe_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_chassis_observer_init(&runtime->runtime);
    platform_observe_bus_init(&runtime->runtime_bus);
}

void observe_task_prepare(platform_observe_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_observe_bus_wait_ready(&runtime->runtime_bus, &runtime->ins_msg);
}

void observe_task_step(platform_observe_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_observe_bus_pull_inputs(&runtime->runtime_bus, &runtime->intent, &runtime->feedback_msg);
    platform_chassis_observer_apply_inputs(&runtime->runtime,
                                           &runtime->intent,
                                           &runtime->feedback_msg,
                                           (float)OBSERVE_TASK_PERIOD_MS / 1000.0f);
    runtime->observe_msg = platform_chassis_observer_build_output(&runtime->runtime);
    platform_observe_bus_publish(&runtime->runtime_bus, &runtime->observe_msg);
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
