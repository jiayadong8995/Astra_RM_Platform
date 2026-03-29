#ifndef PLATFORM_CONTROL_EXECUTION_ACTUATOR_GATEWAY_H
#define PLATFORM_CONTROL_EXECUTION_ACTUATOR_GATEWAY_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "dm4310_drv.h"

typedef struct
{
    Joint_Motor_t *joint_motor[4];
    chassis_motor_measure_t *wheel_motor[2];
} platform_actuator_gateway_t;

void platform_actuator_gateway_init(platform_actuator_gateway_t *runtime);
void platform_actuator_gateway_capture_feedback(const platform_actuator_gateway_t *runtime, Actuator_Feedback_t *feedback_msg);
void platform_actuator_gateway_dispatch_command(const platform_actuator_gateway_t *runtime,
                                                const Actuator_Cmd_t *actuator_msg,
                                                uint32_t systick);

#endif
