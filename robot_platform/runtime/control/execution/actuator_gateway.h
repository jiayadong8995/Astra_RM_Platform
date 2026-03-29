#ifndef PLATFORM_CONTROL_EXECUTION_ACTUATOR_GATEWAY_H
#define PLATFORM_CONTROL_EXECUTION_ACTUATOR_GATEWAY_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "../contracts/actuator_command.h"
#include "../contracts/device_feedback.h"
#include "../../device/device_layer.h"

void platform_actuator_gateway_init(void);
platform_device_result_t platform_actuator_gateway_capture_feedback(platform_device_feedback_t *feedback_msg);
void platform_actuator_gateway_build_legacy_feedback(const platform_device_feedback_t *feedback,
                                                     Actuator_Feedback_t *feedback_msg);
void platform_actuator_gateway_dispatch_command(const platform_actuator_command_t *actuator_msg,
                                                uint32_t systick);

#endif
