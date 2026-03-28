#ifndef BALANCE_CHASSIS_RUNTIME_SERVICE_ACTUATOR_RUNTIME_H
#define BALANCE_CHASSIS_RUNTIME_SERVICE_ACTUATOR_RUNTIME_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "dm4310_drv.h"

typedef struct
{
    Joint_Motor_t *joint_motor[4];
    chassis_motor_measure_t *wheel_motor[2];
} Actuator_Runtime_t;

void actuator_runtime_init(Actuator_Runtime_t *runtime);

void actuator_runtime_capture_feedback(const Actuator_Runtime_t *runtime, Actuator_Feedback_t *feedback_msg);

void actuator_runtime_dispatch_command(const Actuator_Runtime_t *runtime, const Actuator_Cmd_t *actuator_msg, uint32_t systick);

#endif
