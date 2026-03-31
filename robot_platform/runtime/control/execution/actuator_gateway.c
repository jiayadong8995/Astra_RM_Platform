#include "actuator_gateway.h"

#define PLATFORM_DEVICE_COMMAND_FAULT_INVALID_MAPPING 0x1U

static void platform_map_contract_command(const platform_actuator_command_t *actuator_msg,
                                          platform_device_command_t *device_command);
static bool platform_profile_command_mapping_valid(const platform_actuator_command_t *actuator_msg);
static bool platform_command_dispatch_enabled(const platform_actuator_command_t *actuator_msg);

void platform_actuator_gateway_init(void)
{
    (void)platform_device_init_default_profile();
}

platform_device_result_t platform_actuator_gateway_capture_feedback(platform_device_feedback_t *feedback_msg)
{
    return platform_device_read_default_feedback(feedback_msg);
}

void platform_actuator_gateway_dispatch_command(const platform_actuator_command_t *actuator_msg,
                                                uint32_t systick)
{
    platform_device_command_t device_command = {0};

    (void)systick;
    platform_map_contract_command(actuator_msg, &device_command);
    (void)platform_device_write_default_command(&device_command);
}

static void platform_map_contract_command(const platform_actuator_command_t *actuator_msg,
                                          platform_device_command_t *device_command)
{
    const bool mapping_valid = platform_profile_command_mapping_valid(actuator_msg);
    const bool dispatch_enabled = platform_command_dispatch_enabled(actuator_msg) && mapping_valid;

    if (!mapping_valid)
    {
        device_command->backend_flags |= PLATFORM_DEVICE_COMMAND_FAULT_INVALID_MAPPING;
    }

    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        device_command->joints[i].device_id = i;
        device_command->joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
        device_command->joints[i].control_mode = actuator_msg->motors.left_leg_joint[0].control_mode;
        device_command->joints[i].valid = dispatch_enabled;
    }

    device_command->joints[0].control_mode = actuator_msg->motors.left_leg_joint[0].control_mode;
    device_command->joints[0].torque_target = actuator_msg->motors.left_leg_joint[0].torque_target;
    device_command->joints[0].velocity_target = actuator_msg->motors.left_leg_joint[0].velocity_target;
    device_command->joints[0].position_target = actuator_msg->motors.left_leg_joint[0].position_target;
    device_command->joints[0].current_target = actuator_msg->motors.left_leg_joint[0].current_target;
    device_command->joints[0].kp = actuator_msg->motors.left_leg_joint[0].kp;
    device_command->joints[0].kd = actuator_msg->motors.left_leg_joint[0].kd;
    device_command->joints[0].valid = actuator_msg->motors.left_leg_joint[0].valid && dispatch_enabled;

    device_command->joints[1].control_mode = actuator_msg->motors.left_leg_joint[1].control_mode;
    device_command->joints[1].torque_target = actuator_msg->motors.left_leg_joint[1].torque_target;
    device_command->joints[1].velocity_target = actuator_msg->motors.left_leg_joint[1].velocity_target;
    device_command->joints[1].position_target = actuator_msg->motors.left_leg_joint[1].position_target;
    device_command->joints[1].current_target = actuator_msg->motors.left_leg_joint[1].current_target;
    device_command->joints[1].kp = actuator_msg->motors.left_leg_joint[1].kp;
    device_command->joints[1].kd = actuator_msg->motors.left_leg_joint[1].kd;
    device_command->joints[1].valid = actuator_msg->motors.left_leg_joint[1].valid && dispatch_enabled;

    device_command->joints[2].control_mode = actuator_msg->motors.right_leg_joint[0].control_mode;
    device_command->joints[2].torque_target = actuator_msg->motors.right_leg_joint[0].torque_target;
    device_command->joints[2].velocity_target = actuator_msg->motors.right_leg_joint[0].velocity_target;
    device_command->joints[2].position_target = actuator_msg->motors.right_leg_joint[0].position_target;
    device_command->joints[2].current_target = actuator_msg->motors.right_leg_joint[0].current_target;
    device_command->joints[2].kp = actuator_msg->motors.right_leg_joint[0].kp;
    device_command->joints[2].kd = actuator_msg->motors.right_leg_joint[0].kd;
    device_command->joints[2].valid = actuator_msg->motors.right_leg_joint[0].valid && dispatch_enabled;

    device_command->joints[3].control_mode = actuator_msg->motors.right_leg_joint[1].control_mode;
    device_command->joints[3].torque_target = actuator_msg->motors.right_leg_joint[1].torque_target;
    device_command->joints[3].velocity_target = actuator_msg->motors.right_leg_joint[1].velocity_target;
    device_command->joints[3].position_target = actuator_msg->motors.right_leg_joint[1].position_target;
    device_command->joints[3].current_target = actuator_msg->motors.right_leg_joint[1].current_target;
    device_command->joints[3].kp = actuator_msg->motors.right_leg_joint[1].kp;
    device_command->joints[3].kd = actuator_msg->motors.right_leg_joint[1].kd;
    device_command->joints[3].valid = actuator_msg->motors.right_leg_joint[1].valid && dispatch_enabled;

    device_command->wheels[0].device_id = 0;
    device_command->wheels[0].kind = PLATFORM_MOTOR_KIND_WHEEL;
    device_command->wheels[0].control_mode = actuator_msg->motors.left_wheel.control_mode;
    device_command->wheels[0].torque_target = actuator_msg->motors.left_wheel.torque_target;
    device_command->wheels[0].velocity_target = actuator_msg->motors.left_wheel.velocity_target;
    device_command->wheels[0].position_target = actuator_msg->motors.left_wheel.position_target;
    device_command->wheels[0].current_target = actuator_msg->motors.left_wheel.current_target;
    device_command->wheels[0].kp = actuator_msg->motors.left_wheel.kp;
    device_command->wheels[0].kd = actuator_msg->motors.left_wheel.kd;
    device_command->wheels[0].valid = actuator_msg->motors.left_wheel.valid && dispatch_enabled;

    device_command->wheels[1].device_id = 1;
    device_command->wheels[1].kind = PLATFORM_MOTOR_KIND_WHEEL;
    device_command->wheels[1].control_mode = actuator_msg->motors.right_wheel.control_mode;
    device_command->wheels[1].torque_target = actuator_msg->motors.right_wheel.torque_target;
    device_command->wheels[1].velocity_target = actuator_msg->motors.right_wheel.velocity_target;
    device_command->wheels[1].position_target = actuator_msg->motors.right_wheel.position_target;
    device_command->wheels[1].current_target = actuator_msg->motors.right_wheel.current_target;
    device_command->wheels[1].kp = actuator_msg->motors.right_wheel.kp;
    device_command->wheels[1].kd = actuator_msg->motors.right_wheel.kd;
    device_command->wheels[1].valid = actuator_msg->motors.right_wheel.valid && dispatch_enabled;
}

static bool platform_profile_command_mapping_valid(const platform_actuator_command_t *actuator_msg)
{
    if (actuator_msg->motors.left_leg_joint[0].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.left_leg_joint[1].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.right_leg_joint[0].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.right_leg_joint[1].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.left_wheel.control_mode != PLATFORM_MOTOR_CONTROL_CURRENT)
    {
        return false;
    }
    if (actuator_msg->motors.right_wheel.control_mode != PLATFORM_MOTOR_CONTROL_CURRENT)
    {
        return false;
    }

    return true;
}

static bool platform_command_dispatch_enabled(const platform_actuator_command_t *actuator_msg)
{
    return actuator_msg->control_enable && actuator_msg->actuator_enable;
}
