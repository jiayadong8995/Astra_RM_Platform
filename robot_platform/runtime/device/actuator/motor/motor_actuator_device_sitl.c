#include "motor_actuator_device.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
  int sock_fd;
  struct sockaddr_in sim_addr;
  bool initialized;
} platform_motor_actuator_sitl_context_t;

static platform_motor_actuator_sitl_context_t g_sitl_context;

static platform_device_result_t platform_motor_actuator_init(platform_motor_device_t *device);
static platform_device_result_t platform_motor_actuator_write(platform_motor_device_t *device,
                                                              const platform_motor_command_set_t *command_set);
static platform_device_result_t platform_motor_actuator_read(platform_motor_device_t *device,
                                                             platform_device_feedback_t *feedback);
static platform_device_result_t platform_motor_actuator_ensure_socket(platform_motor_device_t *device);
static platform_device_result_t platform_motor_actuator_send_joint_commands(
    platform_motor_actuator_sitl_context_t *context,
    const platform_motor_command_set_t *command_set);
static platform_device_result_t platform_motor_actuator_send_wheel_commands(
    platform_motor_actuator_sitl_context_t *context,
    const platform_motor_command_set_t *command_set);

void platform_motor_actuator_device_bind(platform_motor_device_t *device,
                                         const platform_motor_actuator_device_config_t *config)
{
  (void)config;
  device->name = "motor_actuator_sitl_stub";
  memset(&g_sitl_context, 0, sizeof(g_sitl_context));
  g_sitl_context.sock_fd = -1;
  device->context = &g_sitl_context;
  device->ops.init = platform_motor_actuator_init;
  device->ops.write_motor_command = platform_motor_actuator_write;
  device->ops.read_motor_feedback = platform_motor_actuator_read;
}

static platform_device_result_t platform_motor_actuator_init(platform_motor_device_t *device)
{
  platform_device_result_t result = platform_motor_actuator_ensure_socket(device);
  if (result != PLATFORM_DEVICE_RESULT_OK && result != PLATFORM_DEVICE_RESULT_UNAVAILABLE)
  {
    return result;
  }

  /* Host-side binding tests only need profile init to succeed; command dispatch
   * still attempts to establish the UDP socket before first use.
   */
  device->stamp.valid = (result == PLATFORM_DEVICE_RESULT_OK);
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_ensure_socket(platform_motor_device_t *device)
{
  platform_motor_actuator_sitl_context_t *context = (platform_motor_actuator_sitl_context_t *)device->context;
  if (context == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }
  if (context->initialized)
  {
    return PLATFORM_DEVICE_RESULT_OK;
  }

  context->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (context->sock_fd < 0)
  {
    return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
  }

  memset(&context->sim_addr, 0, sizeof(context->sim_addr));
  context->sim_addr.sin_family = AF_INET;
  context->sim_addr.sin_port = htons(9003);
  context->sim_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  context->initialized = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_send_joint_commands(
    platform_motor_actuator_sitl_context_t *context,
    const platform_motor_command_set_t *command_set)
{
  const platform_motor_command_t *joint_commands[4] = {
      &command_set->left_leg_joint[0],
      &command_set->left_leg_joint[1],
      &command_set->right_leg_joint[0],
      &command_set->right_leg_joint[1],
  };

  for (uint32_t i = 0; i < 4U; ++i)
  {
    struct __attribute__((packed)) {
      uint32_t type;
      uint32_t id;
      float position;
      float velocity;
      float kp;
      float kd;
      float torque;
    } packet = {
        .type = 1U,
        .id = i + 1U,
        .position = joint_commands[i]->position_target,
        .velocity = joint_commands[i]->velocity_target,
        .kp = joint_commands[i]->kp,
        .kd = joint_commands[i]->kd,
        .torque = joint_commands[i]->torque_target,
    };

    ssize_t sent = sendto(context->sock_fd,
                          &packet,
                          sizeof(packet),
                          0,
                          (struct sockaddr *)&context->sim_addr,
                          sizeof(context->sim_addr));
    if (sent != (ssize_t)sizeof(packet))
    {
      return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
    }
  }

  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_send_wheel_commands(
    platform_motor_actuator_sitl_context_t *context,
    const platform_motor_command_set_t *command_set)
{
  struct __attribute__((packed)) {
    uint32_t type;
    float motor1_current;
    float motor2_current;
  } packet = {
      .type = 2U,
      .motor1_current = command_set->left_wheel.current_target,
      .motor2_current = command_set->right_wheel.current_target,
  };

  ssize_t sent = sendto(context->sock_fd,
                        &packet,
                        sizeof(packet),
                        0,
                        (struct sockaddr *)&context->sim_addr,
                        sizeof(context->sim_addr));
  if (sent != (ssize_t)sizeof(packet))
  {
    return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
  }

  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_write(platform_motor_device_t *device,
                                                              const platform_motor_command_set_t *command_set)
{
  platform_motor_actuator_sitl_context_t *context = (platform_motor_actuator_sitl_context_t *)device->context;
  platform_device_result_t result = platform_motor_actuator_ensure_socket(device);
  if (result != PLATFORM_DEVICE_RESULT_OK || context == 0)
  {
    return result;
  }

  result = platform_motor_actuator_send_joint_commands(context, command_set);
  if (result != PLATFORM_DEVICE_RESULT_OK)
  {
    device->stamp.valid = false;
    return result;
  }

  result = platform_motor_actuator_send_wheel_commands(context, command_set);
  if (result != PLATFORM_DEVICE_RESULT_OK)
  {
    device->stamp.valid = false;
    return result;
  }

  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_read(platform_motor_device_t *device,
                                                             platform_device_feedback_t *feedback)
{
  device->stamp.valid = true;

  for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
  {
    feedback->actuator_feedback.joints[i].id = i;
    feedback->actuator_feedback.joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
    feedback->actuator_feedback.joints[i].online = true;
  }

  for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
  {
    feedback->actuator_feedback.wheels[i].id = i;
    feedback->actuator_feedback.wheels[i].kind = PLATFORM_MOTOR_KIND_WHEEL;
    feedback->actuator_feedback.wheels[i].online = true;
  }

  feedback->actuator_feedback.sample_time_us = 1000U;
  feedback->actuator_feedback.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}
