#include "actuator_gateway.h"

static void platform_map_legacy_command(const Actuator_Cmd_t *actuator_msg,
                                        platform_device_command_t *device_command);
static void platform_map_feedback_snapshot(const platform_actuator_feedback_snapshot_t *snapshot,
                                           Actuator_Feedback_t *feedback_msg);

void platform_actuator_gateway_init(platform_actuator_gateway_t *runtime)
{
    platform_device_layer_bind_default(&runtime->devices);
    (void)platform_device_layer_init(&runtime->devices);
}

void platform_actuator_gateway_capture_feedback(const platform_actuator_gateway_t *runtime, Actuator_Feedback_t *feedback_msg)
{
    platform_device_feedback_t feedback = {0};

    if (platform_device_layer_read_feedback((platform_device_layer_t *)&runtime->devices, &feedback)
        != PLATFORM_DEVICE_RESULT_OK)
    {
        return;
    }

    platform_map_feedback_snapshot(&feedback.actuator_feedback, feedback_msg);
}

void platform_actuator_gateway_dispatch_command(const platform_actuator_gateway_t *runtime,
                                                const Actuator_Cmd_t *actuator_msg,
                                                uint32_t systick)
{
    platform_device_command_t device_command = {0};

    (void)systick;
    platform_map_legacy_command(actuator_msg, &device_command);
    (void)platform_device_layer_write_command((platform_device_layer_t *)&runtime->devices, &device_command);
}

static void platform_map_legacy_command(const Actuator_Cmd_t *actuator_msg,
                                        platform_device_command_t *device_command)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        device_command->joints[i].device_id = i;
        device_command->joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
        device_command->joints[i].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
        device_command->joints[i].torque_target = actuator_msg->joint_torque[i];
        device_command->joints[i].valid = (actuator_msg->start_flag != 0U);
    }

    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
    {
        device_command->wheels[i].device_id = i;
        device_command->wheels[i].kind = PLATFORM_MOTOR_KIND_WHEEL;
        device_command->wheels[i].control_mode = PLATFORM_MOTOR_CONTROL_CURRENT;
        device_command->wheels[i].current_target = (float)actuator_msg->wheel_current[i];
        device_command->wheels[i].valid = (actuator_msg->start_flag != 0U);
    }
}

static void platform_map_feedback_snapshot(const platform_actuator_feedback_snapshot_t *snapshot,
                                           Actuator_Feedback_t *feedback_msg)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        feedback_msg->joint_pos[i] = snapshot->joints[i].position;
    }

    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
    {
        feedback_msg->wheel_speed[i] = snapshot->wheels[i].velocity;
        feedback_msg->wheel_angle[i] = snapshot->wheels[i].position;
    }

    feedback_msg->ready = snapshot->valid ? 1U : 0U;
}
