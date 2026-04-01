#include "actuator_gateway.h"

#define PLATFORM_DEVICE_COMMAND_FAULT_INVALID_MAPPING 0x1U

static bool platform_profile_command_mapping_valid(const platform_actuator_command_t *actuator_msg);
static bool platform_command_dispatch_enabled(const platform_actuator_command_t *actuator_msg);

void platform_actuator_gateway_init(void)
{
    /* Port implementations now self-init on first call — nothing to do here. */
}

platform_device_result_t platform_actuator_gateway_capture_feedback(platform_device_feedback_t *feedback_msg)
{
    return platform_motor_read_feedback(feedback_msg);
}

void platform_actuator_gateway_dispatch_command(const platform_actuator_command_t *actuator_msg,
                                                uint32_t systick)
{
    platform_actuator_command_t device_command = *actuator_msg;

    (void)systick;

    const bool mapping_valid = platform_profile_command_mapping_valid(actuator_msg);
    const bool dispatch_enabled = platform_command_dispatch_enabled(actuator_msg) && mapping_valid;

    if (!mapping_valid)
    {
        device_command.motors.joints[PLATFORM_JOINT_LEFT_FRONT].valid = false;
        device_command.motors.joints[PLATFORM_JOINT_LEFT_REAR].valid = false;
        device_command.motors.joints[PLATFORM_JOINT_RIGHT_FRONT].valid = false;
        device_command.motors.joints[PLATFORM_JOINT_RIGHT_REAR].valid = false;
        device_command.motors.wheels[PLATFORM_WHEEL_LEFT].valid = false;
        device_command.motors.wheels[PLATFORM_WHEEL_RIGHT].valid = false;
    }

    if (!dispatch_enabled)
    {
        for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
        {
            device_command.motors.joints[i].valid = false;
        }
        for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
        {
            device_command.motors.wheels[i].valid = false;
        }
    }

    (void)platform_motor_write_command(&device_command.motors);
}

static bool platform_profile_command_mapping_valid(const platform_actuator_command_t *actuator_msg)
{
    if (actuator_msg->motors.joints[PLATFORM_JOINT_LEFT_FRONT].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.joints[PLATFORM_JOINT_LEFT_REAR].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.joints[PLATFORM_JOINT_RIGHT_FRONT].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.joints[PLATFORM_JOINT_RIGHT_REAR].control_mode != PLATFORM_MOTOR_CONTROL_TORQUE)
    {
        return false;
    }
    if (actuator_msg->motors.wheels[PLATFORM_WHEEL_LEFT].control_mode != PLATFORM_MOTOR_CONTROL_CURRENT)
    {
        return false;
    }
    if (actuator_msg->motors.wheels[PLATFORM_WHEEL_RIGHT].control_mode != PLATFORM_MOTOR_CONTROL_CURRENT)
    {
        return false;
    }

    return true;
}

static bool platform_command_dispatch_enabled(const platform_actuator_command_t *actuator_msg)
{
    return actuator_msg->control_enable && actuator_msg->actuator_enable;
}
