#include "observe_runtime_helpers.h"

void observe_runtime_init(Observe_Runtime_t *runtime)
{
    runtime->v_filter = 0.0f;
    runtime->x_filter = 0.0f;
}

void observe_runtime_apply_inputs(Observe_Runtime_t *runtime,
                                  const Chassis_Cmd_t *cmd_msg,
                                  const Actuator_Feedback_t *feedback_msg,
                                  float dt_s)
{
    runtime->v_filter = -(feedback_msg->wheel_speed[1] - feedback_msg->wheel_speed[0]) / 2.0f;
    runtime->x_filter = runtime->x_filter + runtime->v_filter * dt_s;

    if (cmd_msg->recover_flag != 0U)
    {
        runtime->x_filter = 0.0f;
    }
}

Chassis_Observe_t observe_runtime_build_output(const Observe_Runtime_t *runtime)
{
    Chassis_Observe_t observe_msg = {
        .v_filter = runtime->v_filter,
        .x_filter = runtime->x_filter,
    };
    return observe_msg;
}
