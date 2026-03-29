#include "chassis_observer.h"

void platform_chassis_observer_init(platform_chassis_observer_t *runtime)
{
    runtime->v_filter = 0.0f;
    runtime->x_filter = 0.0f;
}

void platform_chassis_observer_apply_inputs(platform_chassis_observer_t *runtime,
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

Chassis_Observe_t platform_chassis_observer_build_output(const platform_chassis_observer_t *runtime)
{
    Chassis_Observe_t observe_msg = {
        .v_filter = runtime->v_filter,
        .x_filter = runtime->x_filter,
    };
    return observe_msg;
}
