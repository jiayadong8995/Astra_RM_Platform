#include "chassis_observer.h"

void platform_chassis_observer_init(platform_chassis_observer_t *runtime)
{
    runtime->v_filter = 0.0f;
    runtime->x_filter = 0.0f;
}

void platform_chassis_observer_apply_inputs(platform_chassis_observer_t *runtime,
                                            const platform_robot_intent_t *intent,
                                            const platform_device_feedback_t *feedback_msg,
                                            float dt_s)
{
    runtime->v_filter = -(feedback_msg->actuator_feedback.wheels[1].velocity
                        - feedback_msg->actuator_feedback.wheels[0].velocity) / 2.0f;
    runtime->x_filter = runtime->x_filter + runtime->v_filter * dt_s;

    if (intent->behavior_request.recover_request)
    {
        runtime->x_filter = 0.0f;
    }
}

platform_chassis_observe_message_t platform_chassis_observer_build_output(const platform_chassis_observer_t *runtime)
{
    platform_chassis_observe_message_t observe_msg = {
        .v_filter = runtime->v_filter,
        .x_filter = runtime->x_filter,
    };
    return observe_msg;
}
