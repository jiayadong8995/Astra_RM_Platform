#include "actuator_runtime.h"

#include "fdcan.h"

void actuator_runtime_init(Actuator_Runtime_t *runtime)
{
    runtime->joint_motor[0] = get_joint_motor_state(0);
    runtime->joint_motor[1] = get_joint_motor_state(1);
    runtime->joint_motor[2] = get_joint_motor_state(2);
    runtime->joint_motor[3] = get_joint_motor_state(3);
    runtime->wheel_motor[0] = get_chassis_motor_measure_point(0);
    runtime->wheel_motor[1] = get_chassis_motor_measure_point(1);

    joint_motor_init(runtime->joint_motor[0], 1, MIT_MODE);
    joint_motor_init(runtime->joint_motor[1], 2, MIT_MODE);
    joint_motor_init(runtime->joint_motor[2], 3, MIT_MODE);
    joint_motor_init(runtime->joint_motor[3], 4, MIT_MODE);

    enable_motor_mode(&hfdcan1, runtime->joint_motor[0]->para.id, runtime->joint_motor[0]->mode);
    enable_motor_mode(&hfdcan1, runtime->joint_motor[1]->para.id, runtime->joint_motor[1]->mode);
    enable_motor_mode(&hfdcan1, runtime->joint_motor[2]->para.id, runtime->joint_motor[2]->mode);
    enable_motor_mode(&hfdcan1, runtime->joint_motor[3]->para.id, runtime->joint_motor[3]->mode);
}

void actuator_runtime_capture_feedback(const Actuator_Runtime_t *runtime, Actuator_Feedback_t *feedback_msg)
{
    feedback_msg->joint_pos[0] = runtime->joint_motor[0]->para.pos;
    feedback_msg->joint_pos[1] = runtime->joint_motor[1]->para.pos;
    feedback_msg->joint_pos[2] = runtime->joint_motor[2]->para.pos;
    feedback_msg->joint_pos[3] = runtime->joint_motor[3]->para.pos;
    feedback_msg->wheel_speed[0] = runtime->wheel_motor[0]->speed_rpm * M3508_RPM_TO_RADS * WHEEL_RADIUS;
    feedback_msg->wheel_speed[1] = runtime->wheel_motor[1]->speed_rpm * M3508_RPM_TO_RADS * WHEEL_RADIUS;
    feedback_msg->wheel_angle[0] = runtime->wheel_motor[0]->total_angle / WHEEL_GEAR_RATIO * WHEEL_RADIUS;
    feedback_msg->wheel_angle[1] = runtime->wheel_motor[1]->total_angle / WHEEL_GEAR_RATIO * WHEEL_RADIUS;
    feedback_msg->ready = 1U;
}

void actuator_runtime_dispatch_command(const Actuator_Runtime_t *runtime, const Actuator_Cmd_t *actuator_msg, uint32_t systick)
{
    if (actuator_msg->start_flag == 0U)
    {
        if ((systick % 2U) == 0U)
        {
            mit_ctrl(&hfdcan1, runtime->joint_motor[0]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            mit_ctrl(&hfdcan1, runtime->joint_motor[1]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            CAN_cmd_chassis(&hfdcan2, 0, 0, 0, 0);
        }
        else
        {
            mit_ctrl(&hfdcan1, runtime->joint_motor[2]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            mit_ctrl(&hfdcan1, runtime->joint_motor[3]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        }
        return;
    }

    if ((systick % 2U) == 0U)
    {
        mit_ctrl(&hfdcan1, runtime->joint_motor[0]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, actuator_msg->joint_torque[0]);
        mit_ctrl(&hfdcan1, runtime->joint_motor[1]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, actuator_msg->joint_torque[1]);
        CAN_cmd_chassis(&hfdcan2, actuator_msg->wheel_current[0], actuator_msg->wheel_current[1], 0, 0);
    }
    else
    {
        mit_ctrl(&hfdcan1, runtime->joint_motor[2]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, actuator_msg->joint_torque[2]);
        mit_ctrl(&hfdcan1, runtime->joint_motor[3]->para.id, 0.0f, 0.0f, 0.0f, 0.0f, actuator_msg->joint_torque[3]);
    }
}
