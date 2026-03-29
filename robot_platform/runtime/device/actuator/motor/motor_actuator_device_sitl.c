#include "motor_actuator_device.h"

static platform_device_result_t platform_motor_actuator_init(platform_motor_device_t *device);
static platform_device_result_t platform_motor_actuator_write(platform_motor_device_t *device,
                                                              const platform_motor_command_set_t *command_set);
static platform_device_result_t platform_motor_actuator_read(platform_motor_device_t *device,
                                                             platform_device_feedback_t *feedback);

void platform_motor_actuator_device_bind(platform_motor_device_t *device,
                                         const platform_motor_actuator_device_config_t *config)
{
  (void)config;
  device->name = "motor_actuator_sitl_stub";
  device->context = 0;
  device->ops.init = platform_motor_actuator_init;
  device->ops.write_motor_command = platform_motor_actuator_write;
  device->ops.read_motor_feedback = platform_motor_actuator_read;
}

static platform_device_result_t platform_motor_actuator_init(platform_motor_device_t *device)
{
  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_write(platform_motor_device_t *device,
                                                              const platform_motor_command_set_t *command_set)
{
  (void)device;
  (void)command_set;
  return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
}

static platform_device_result_t platform_motor_actuator_read(platform_motor_device_t *device,
                                                             platform_device_feedback_t *feedback)
{
  (void)device;

  for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
  {
    feedback->actuator_feedback.joints[i].id = i;
    feedback->actuator_feedback.joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
    feedback->actuator_feedback.joints[i].online = false;
  }

  for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
  {
    feedback->actuator_feedback.wheels[i].id = i;
    feedback->actuator_feedback.wheels[i].kind = PLATFORM_MOTOR_KIND_WHEEL;
    feedback->actuator_feedback.wheels[i].online = false;
  }

  feedback->actuator_feedback.sample_time_us = 0U;
  feedback->actuator_feedback.valid = false;
  return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
}
