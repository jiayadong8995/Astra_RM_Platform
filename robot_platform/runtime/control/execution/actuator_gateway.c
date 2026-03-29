#include "actuator_gateway.h"

static void platform_map_contract_command(const platform_actuator_command_t *actuator_msg,
                                          platform_device_command_t *device_command);

void platform_actuator_gateway_init(platform_actuator_gateway_t *runtime)
{
    (void)platform_device_layer_init_default(&runtime->devices);
}

platform_device_result_t platform_actuator_gateway_capture_feedback(const platform_actuator_gateway_t *runtime,
                                                                   platform_device_feedback_t *feedback_msg)
{
    return platform_device_layer_read_feedback((platform_device_layer_t *)&runtime->devices, feedback_msg);
}

void platform_actuator_gateway_build_legacy_feedback(const platform_device_feedback_t *feedback,
                                                     Actuator_Feedback_t *feedback_msg)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        feedback_msg->joint_pos[i] = feedback->actuator_feedback.joints[i].position;
    }

    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
    {
        feedback_msg->wheel_speed[i] = feedback->actuator_feedback.wheels[i].velocity;
        feedback_msg->wheel_angle[i] = feedback->actuator_feedback.wheels[i].position;
    }

    feedback_msg->ready = feedback->actuator_feedback.valid ? 1U : 0U;
}

void platform_actuator_gateway_dispatch_command(const platform_actuator_gateway_t *runtime,
                                                const platform_actuator_command_t *actuator_msg,
                                                uint32_t systick)
{
    platform_device_command_t device_command = {0};

    (void)systick;
    platform_map_contract_command(actuator_msg, &device_command);
    (void)platform_device_layer_write_command((platform_device_layer_t *)&runtime->devices, &device_command);
}

static void platform_map_contract_command(const platform_actuator_command_t *actuator_msg,
                                          platform_device_command_t *device_command)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        device_command->joints[i].device_id = i;
        device_command->joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
        device_command->joints[i].control_mode = actuator_msg->motors.left_leg_joint[0].control_mode;
        device_command->joints[i].valid = actuator_msg->control_enable && actuator_msg->actuator_enable;
    }

    device_command->joints[0].control_mode = actuator_msg->motors.left_leg_joint[0].control_mode;
    device_command->joints[0].torque_target = actuator_msg->motors.left_leg_joint[0].torque_target;
    device_command->joints[0].velocity_target = actuator_msg->motors.left_leg_joint[0].velocity_target;
    device_command->joints[0].position_target = actuator_msg->motors.left_leg_joint[0].position_target;
    device_command->joints[0].current_target = actuator_msg->motors.left_leg_joint[0].current_target;
    device_command->joints[0].kp = actuator_msg->motors.left_leg_joint[0].kp;
    device_command->joints[0].kd = actuator_msg->motors.left_leg_joint[0].kd;
    device_command->joints[0].valid = actuator_msg->motors.left_leg_joint[0].valid && actuator_msg->actuator_enable;

    device_command->joints[1].control_mode = actuator_msg->motors.left_leg_joint[1].control_mode;
    device_command->joints[1].torque_target = actuator_msg->motors.left_leg_joint[1].torque_target;
    device_command->joints[1].velocity_target = actuator_msg->motors.left_leg_joint[1].velocity_target;
    device_command->joints[1].position_target = actuator_msg->motors.left_leg_joint[1].position_target;
    device_command->joints[1].current_target = actuator_msg->motors.left_leg_joint[1].current_target;
    device_command->joints[1].kp = actuator_msg->motors.left_leg_joint[1].kp;
    device_command->joints[1].kd = actuator_msg->motors.left_leg_joint[1].kd;
    device_command->joints[1].valid = actuator_msg->motors.left_leg_joint[1].valid && actuator_msg->actuator_enable;

    device_command->joints[2].control_mode = actuator_msg->motors.right_leg_joint[0].control_mode;
    device_command->joints[2].torque_target = actuator_msg->motors.right_leg_joint[0].torque_target;
    device_command->joints[2].velocity_target = actuator_msg->motors.right_leg_joint[0].velocity_target;
    device_command->joints[2].position_target = actuator_msg->motors.right_leg_joint[0].position_target;
    device_command->joints[2].current_target = actuator_msg->motors.right_leg_joint[0].current_target;
    device_command->joints[2].kp = actuator_msg->motors.right_leg_joint[0].kp;
    device_command->joints[2].kd = actuator_msg->motors.right_leg_joint[0].kd;
    device_command->joints[2].valid = actuator_msg->motors.right_leg_joint[0].valid && actuator_msg->actuator_enable;

    device_command->joints[3].control_mode = actuator_msg->motors.right_leg_joint[1].control_mode;
    device_command->joints[3].torque_target = actuator_msg->motors.right_leg_joint[1].torque_target;
    device_command->joints[3].velocity_target = actuator_msg->motors.right_leg_joint[1].velocity_target;
    device_command->joints[3].position_target = actuator_msg->motors.right_leg_joint[1].position_target;
    device_command->joints[3].current_target = actuator_msg->motors.right_leg_joint[1].current_target;
    device_command->joints[3].kp = actuator_msg->motors.right_leg_joint[1].kp;
    device_command->joints[3].kd = actuator_msg->motors.right_leg_joint[1].kd;
    device_command->joints[3].valid = actuator_msg->motors.right_leg_joint[1].valid && actuator_msg->actuator_enable;

    device_command->wheels[0].device_id = 0;
    device_command->wheels[0].kind = PLATFORM_MOTOR_KIND_WHEEL;
    device_command->wheels[0].control_mode = actuator_msg->motors.left_wheel.control_mode;
    device_command->wheels[0].torque_target = actuator_msg->motors.left_wheel.torque_target;
    device_command->wheels[0].velocity_target = actuator_msg->motors.left_wheel.velocity_target;
    device_command->wheels[0].position_target = actuator_msg->motors.left_wheel.position_target;
    device_command->wheels[0].current_target = actuator_msg->motors.left_wheel.current_target;
    device_command->wheels[0].kp = actuator_msg->motors.left_wheel.kp;
    device_command->wheels[0].kd = actuator_msg->motors.left_wheel.kd;
    device_command->wheels[0].valid = actuator_msg->motors.left_wheel.valid && actuator_msg->actuator_enable;

    device_command->wheels[1].device_id = 1;
    device_command->wheels[1].kind = PLATFORM_MOTOR_KIND_WHEEL;
    device_command->wheels[1].control_mode = actuator_msg->motors.right_wheel.control_mode;
    device_command->wheels[1].torque_target = actuator_msg->motors.right_wheel.torque_target;
    device_command->wheels[1].velocity_target = actuator_msg->motors.right_wheel.velocity_target;
    device_command->wheels[1].position_target = actuator_msg->motors.right_wheel.position_target;
    device_command->wheels[1].current_target = actuator_msg->motors.right_wheel.current_target;
    device_command->wheels[1].kp = actuator_msg->motors.right_wheel.kp;
    device_command->wheels[1].kd = actuator_msg->motors.right_wheel.kd;
    device_command->wheels[1].valid = actuator_msg->motors.right_wheel.valid && actuator_msg->actuator_enable;
}
