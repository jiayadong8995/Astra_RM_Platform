#include "motor_actuator_device.h"

#include "dm4310/dm4310_drv.h"
#include "fdcan.h"

typedef struct {
  Joint_Motor_t *joints[PLATFORM_JOINT_MOTOR_COUNT];
  chassis_motor_measure_t *wheels[PLATFORM_WHEEL_MOTOR_COUNT];
  const platform_motor_actuator_device_config_t *config;
} platform_motor_actuator_context_t;

static platform_device_result_t platform_motor_actuator_init(platform_motor_device_t *device);
static platform_device_result_t platform_motor_actuator_write(platform_motor_device_t *device,
                                                              const platform_motor_command_set_t *command_set);
static platform_device_result_t platform_motor_actuator_read(platform_motor_device_t *device,
                                                             platform_device_feedback_t *feedback);

static platform_motor_actuator_context_t g_platform_motor_actuator_context;

void platform_motor_actuator_device_bind(platform_motor_device_t *device,
                                         const platform_motor_actuator_device_config_t *config)
{
  g_platform_motor_actuator_context.config = config;
  device->name = "motor_actuator";
  device->context = &g_platform_motor_actuator_context;
  device->ops.init = platform_motor_actuator_init;
  device->ops.write_motor_command = platform_motor_actuator_write;
  device->ops.read_motor_feedback = platform_motor_actuator_read;
}

static platform_device_result_t platform_motor_actuator_init(platform_motor_device_t *device)
{
  platform_motor_actuator_context_t *context = (platform_motor_actuator_context_t *)device->context;
  const platform_motor_actuator_device_config_t *config = context->config;

  if (config == 0 || config->get_joint_state_fn == 0 || config->get_wheel_state_fn == 0
      || config->joint_init_fn == 0 || config->enable_mode_fn == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }

  for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
  {
    context->joints[i] = (Joint_Motor_t *)config->get_joint_state_fn(i);
    config->joint_init_fn(context->joints[i], (uint16_t)(i + 1U), MIT_MODE);
    config->enable_mode_fn(config->joint_can_handle, context->joints[i]->para.id, context->joints[i]->mode);
  }

  for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
  {
    context->wheels[i] = (chassis_motor_measure_t *)config->get_wheel_state_fn(i);
  }

  device->stamp.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_write(platform_motor_device_t *device,
                                                              const platform_motor_command_set_t *command_set)
{
  platform_motor_actuator_context_t *context = (platform_motor_actuator_context_t *)device->context;
  const platform_motor_actuator_device_config_t *config = context->config;

  if (config == 0 || config->mit_ctrl_fn == 0 || config->wheel_cmd_fn == 0)
  {
    return PLATFORM_DEVICE_RESULT_INVALID;
  }

  config->mit_ctrl_fn(config->joint_can_handle, context->joints[0]->para.id, 0.0f, 0.0f,
                      command_set->left_leg_joint[0].kp, command_set->left_leg_joint[0].kd,
                      command_set->left_leg_joint[0].torque_target);
  config->mit_ctrl_fn(config->joint_can_handle, context->joints[1]->para.id, 0.0f, 0.0f,
                      command_set->left_leg_joint[1].kp, command_set->left_leg_joint[1].kd,
                      command_set->left_leg_joint[1].torque_target);
  config->mit_ctrl_fn(config->joint_can_handle, context->joints[2]->para.id, 0.0f, 0.0f,
                      command_set->right_leg_joint[0].kp, command_set->right_leg_joint[0].kd,
                      command_set->right_leg_joint[0].torque_target);
  config->mit_ctrl_fn(config->joint_can_handle, context->joints[3]->para.id, 0.0f, 0.0f,
                      command_set->right_leg_joint[1].kp, command_set->right_leg_joint[1].kd,
                      command_set->right_leg_joint[1].torque_target);

  config->wheel_cmd_fn(config->wheel_can_handle,
                       (int16_t)command_set->left_wheel.current_target,
                       (int16_t)command_set->right_wheel.current_target,
                       0,
                       0);

  return PLATFORM_DEVICE_RESULT_OK;
}

static platform_device_result_t platform_motor_actuator_read(platform_motor_device_t *device,
                                                             platform_device_feedback_t *feedback)
{
  platform_motor_actuator_context_t *context = (platform_motor_actuator_context_t *)device->context;

  for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
  {
    feedback->actuator_feedback.joints[i].id = i;
    feedback->actuator_feedback.joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
    feedback->actuator_feedback.joints[i].position = context->joints[i]->para.pos;
    feedback->actuator_feedback.joints[i].velocity = context->joints[i]->para.vel;
    feedback->actuator_feedback.joints[i].torque_est = context->joints[i]->para.tor;
    feedback->actuator_feedback.joints[i].temperature = context->joints[i]->para.Tmos;
    feedback->actuator_feedback.joints[i].online = true;
  }

  for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
  {
    feedback->actuator_feedback.wheels[i].id = i;
    feedback->actuator_feedback.wheels[i].kind = PLATFORM_MOTOR_KIND_WHEEL;
    feedback->actuator_feedback.wheels[i].position = context->wheels[i]->total_angle;
    feedback->actuator_feedback.wheels[i].velocity = context->wheels[i]->speed_rpm;
    feedback->actuator_feedback.wheels[i].torque_est = context->wheels[i]->given_current;
    feedback->actuator_feedback.wheels[i].temperature = context->wheels[i]->temperate;
    feedback->actuator_feedback.wheels[i].online = true;
  }

  feedback->actuator_feedback.valid = true;
  return PLATFORM_DEVICE_RESULT_OK;
}
