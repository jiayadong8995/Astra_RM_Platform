#include "observe_task.h"

#include "cmsis_os.h"
#include "../../app/balance_chassis/app_config/app_params.h"
#include "../../app/balance_chassis/app_config/robot_def.h"
#include "chassis_observer.h"
#include "observe_topics.h"

void Observe_task(void)
{
    platform_chassis_observer_t runtime = {0};
    platform_observe_bus_t runtime_bus = {0};
    INS_Data_t ins_msg = {0};
    Chassis_Cmd_t cmd_msg = {0};
    Actuator_Feedback_t feedback_msg = {0};
    Chassis_Observe_t observe_msg = {0};

    platform_chassis_observer_init(&runtime);
    platform_observe_bus_init(&runtime_bus);
    platform_observe_bus_wait_ready(&runtime_bus, &ins_msg);

    while(1)
    {
        platform_observe_bus_pull_inputs(&runtime_bus, &cmd_msg, &feedback_msg);
        platform_chassis_observer_apply_inputs(&runtime, &cmd_msg, &feedback_msg, (float)OBSERVE_TASK_PERIOD_MS / 1000.0f);
        observe_msg = platform_chassis_observer_build_output(&runtime);
        platform_observe_bus_publish(&runtime_bus, &observe_msg);

        osDelay(OBSERVE_TASK_PERIOD_MS);
    }
}
