#ifndef PLATFORM_CONTROL_STATE_CHASSIS_OBSERVER_H
#define PLATFORM_CONTROL_STATE_CHASSIS_OBSERVER_H

#include "../contracts/device_feedback.h"
#include "../contracts/robot_intent.h"
#include "chassis_observe_message.h"

typedef struct
{
    float v_filter;
    float x_filter;
} platform_chassis_observer_t;

void platform_chassis_observer_init(platform_chassis_observer_t *runtime);
void platform_chassis_observer_apply_inputs(platform_chassis_observer_t *runtime,
                                            const platform_robot_intent_t *intent,
                                            const platform_device_feedback_t *feedback_msg,
                                            float dt_s);
platform_chassis_observe_message_t platform_chassis_observer_build_output(const platform_chassis_observer_t *runtime);

#endif
