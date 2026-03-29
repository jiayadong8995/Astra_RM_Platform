#include "device_layer.h"

#include "device_backend.h"

static void platform_map_device_command_to_motor_set(const platform_device_command_t *command,
                                                     platform_motor_command_set_t *motor_set);
static void platform_copy_feedback_to_input(const platform_device_feedback_t *feedback,
                                            platform_device_input_t *input);
static platform_device_result_t platform_default_layer_ensure_ready(void);

static platform_device_layer_t g_platform_default_layer;
static bool g_platform_default_layer_ready;

void platform_device_layer_bind_default(platform_device_layer_t *layer)
{
  platform_device_backend_bind_default(layer);
  layer->input_sequence = 0U;
  layer->feedback_sequence = 0U;
  layer->command_sequence = 0U;
}

platform_device_result_t platform_device_layer_init(platform_device_layer_t *layer)
{
  platform_device_result_t result = PLATFORM_DEVICE_RESULT_OK;

  if (layer->imu.ops.init != 0)
  {
    result = layer->imu.ops.init(&layer->imu);
    if (result != PLATFORM_DEVICE_RESULT_OK)
    {
      return result;
    }
  }

  if (layer->remote.ops.init != 0)
  {
    result = layer->remote.ops.init(&layer->remote);
    if (result != PLATFORM_DEVICE_RESULT_OK)
    {
      return result;
    }
  }

  if (layer->motor.ops.init != 0)
  {
    result = layer->motor.ops.init(&layer->motor);
    if (result != PLATFORM_DEVICE_RESULT_OK)
    {
      return result;
    }
  }

  return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_device_layer_read_input(platform_device_layer_t *layer,
                                                          platform_device_input_t *input)
{
  platform_device_result_t result = PLATFORM_DEVICE_RESULT_OK;
  platform_device_feedback_t feedback = {0};

  input->sequence = ++layer->input_sequence;

  if (layer->imu.ops.read_sample != 0)
  {
    result = layer->imu.ops.read_sample(&layer->imu, &input->imu_sample);
    if (result != PLATFORM_DEVICE_RESULT_OK && result != PLATFORM_DEVICE_RESULT_UNAVAILABLE)
    {
      return result;
    }
  }

  if (layer->remote.ops.read_input != 0)
  {
    result = layer->remote.ops.read_input(&layer->remote, &input->rc_input);
    if (result != PLATFORM_DEVICE_RESULT_OK && result != PLATFORM_DEVICE_RESULT_UNAVAILABLE)
    {
      return result;
    }
  }

  if (layer->motor.ops.read_motor_feedback != 0)
  {
    result = layer->motor.ops.read_motor_feedback(&layer->motor, &feedback);
    if (result != PLATFORM_DEVICE_RESULT_OK && result != PLATFORM_DEVICE_RESULT_UNAVAILABLE)
    {
      return result;
    }
    platform_copy_feedback_to_input(&feedback, input);
  }

  return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_device_layer_read_remote(platform_device_layer_t *layer,
                                                           platform_rc_input_t *input)
{
  if (layer->remote.ops.read_input == 0)
  {
    return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
  }

  return layer->remote.ops.read_input(&layer->remote, input);
}

platform_device_result_t platform_device_layer_read_imu(platform_device_layer_t *layer,
                                                        platform_imu_sample_t *sample)
{
  if (layer->imu.ops.read_sample == 0)
  {
    return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
  }

  return layer->imu.ops.read_sample(&layer->imu, sample);
}

platform_device_result_t platform_device_layer_write_command(platform_device_layer_t *layer,
                                                             const platform_device_command_t *command)
{
  platform_motor_command_set_t motor_set = {0};

  if (layer->motor.ops.write_motor_command == 0)
  {
    return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
  }

  layer->command_sequence++;
  platform_map_device_command_to_motor_set(command, &motor_set);
  return layer->motor.ops.write_motor_command(&layer->motor, &motor_set);
}

platform_device_result_t platform_device_layer_read_feedback(platform_device_layer_t *layer,
                                                             platform_device_feedback_t *feedback)
{
  if (layer->motor.ops.read_motor_feedback == 0)
  {
    return PLATFORM_DEVICE_RESULT_UNSUPPORTED;
  }

  feedback->sequence = ++layer->feedback_sequence;
  return layer->motor.ops.read_motor_feedback(&layer->motor, feedback);
}

platform_device_result_t platform_device_read_default_remote(platform_rc_input_t *input)
{
  platform_device_result_t result = platform_default_layer_ensure_ready();

  if (result != PLATFORM_DEVICE_RESULT_OK)
  {
    return result;
  }

  return platform_device_layer_read_remote(&g_platform_default_layer, input);
}

platform_device_result_t platform_device_read_default_imu(platform_imu_sample_t *sample)
{
  platform_device_result_t result = platform_default_layer_ensure_ready();

  if (result != PLATFORM_DEVICE_RESULT_OK)
  {
    return result;
  }

  return platform_device_layer_read_imu(&g_platform_default_layer, sample);
}

static void platform_map_device_command_to_motor_set(const platform_device_command_t *command,
                                                     platform_motor_command_set_t *motor_set)
{
  motor_set->left_leg_joint[0].control_mode = command->joints[0].control_mode;
  motor_set->left_leg_joint[0].torque_target = command->joints[0].torque_target;
  motor_set->left_leg_joint[0].velocity_target = command->joints[0].velocity_target;
  motor_set->left_leg_joint[0].position_target = command->joints[0].position_target;
  motor_set->left_leg_joint[0].current_target = command->joints[0].current_target;
  motor_set->left_leg_joint[0].kp = command->joints[0].kp;
  motor_set->left_leg_joint[0].kd = command->joints[0].kd;
  motor_set->left_leg_joint[0].valid = command->joints[0].valid;

  motor_set->left_leg_joint[1].control_mode = command->joints[1].control_mode;
  motor_set->left_leg_joint[1].torque_target = command->joints[1].torque_target;
  motor_set->left_leg_joint[1].velocity_target = command->joints[1].velocity_target;
  motor_set->left_leg_joint[1].position_target = command->joints[1].position_target;
  motor_set->left_leg_joint[1].current_target = command->joints[1].current_target;
  motor_set->left_leg_joint[1].kp = command->joints[1].kp;
  motor_set->left_leg_joint[1].kd = command->joints[1].kd;
  motor_set->left_leg_joint[1].valid = command->joints[1].valid;

  motor_set->right_leg_joint[0].control_mode = command->joints[2].control_mode;
  motor_set->right_leg_joint[0].torque_target = command->joints[2].torque_target;
  motor_set->right_leg_joint[0].velocity_target = command->joints[2].velocity_target;
  motor_set->right_leg_joint[0].position_target = command->joints[2].position_target;
  motor_set->right_leg_joint[0].current_target = command->joints[2].current_target;
  motor_set->right_leg_joint[0].kp = command->joints[2].kp;
  motor_set->right_leg_joint[0].kd = command->joints[2].kd;
  motor_set->right_leg_joint[0].valid = command->joints[2].valid;

  motor_set->right_leg_joint[1].control_mode = command->joints[3].control_mode;
  motor_set->right_leg_joint[1].torque_target = command->joints[3].torque_target;
  motor_set->right_leg_joint[1].velocity_target = command->joints[3].velocity_target;
  motor_set->right_leg_joint[1].position_target = command->joints[3].position_target;
  motor_set->right_leg_joint[1].current_target = command->joints[3].current_target;
  motor_set->right_leg_joint[1].kp = command->joints[3].kp;
  motor_set->right_leg_joint[1].kd = command->joints[3].kd;
  motor_set->right_leg_joint[1].valid = command->joints[3].valid;

  motor_set->left_wheel.control_mode = command->wheels[0].control_mode;
  motor_set->left_wheel.torque_target = command->wheels[0].torque_target;
  motor_set->left_wheel.velocity_target = command->wheels[0].velocity_target;
  motor_set->left_wheel.position_target = command->wheels[0].position_target;
  motor_set->left_wheel.current_target = command->wheels[0].current_target;
  motor_set->left_wheel.kp = command->wheels[0].kp;
  motor_set->left_wheel.kd = command->wheels[0].kd;
  motor_set->left_wheel.valid = command->wheels[0].valid;

  motor_set->right_wheel.control_mode = command->wheels[1].control_mode;
  motor_set->right_wheel.torque_target = command->wheels[1].torque_target;
  motor_set->right_wheel.velocity_target = command->wheels[1].velocity_target;
  motor_set->right_wheel.position_target = command->wheels[1].position_target;
  motor_set->right_wheel.current_target = command->wheels[1].current_target;
  motor_set->right_wheel.kp = command->wheels[1].kp;
  motor_set->right_wheel.kd = command->wheels[1].kd;
  motor_set->right_wheel.valid = command->wheels[1].valid;
}

static void platform_copy_feedback_to_input(const platform_device_feedback_t *feedback,
                                            platform_device_input_t *input)
{
  input->actuator_feedback = feedback->actuator_feedback;
}

static platform_device_result_t platform_default_layer_ensure_ready(void)
{
  if (!g_platform_default_layer_ready)
  {
    platform_device_layer_bind_default(&g_platform_default_layer);
    if (platform_device_layer_init(&g_platform_default_layer) != PLATFORM_DEVICE_RESULT_OK)
    {
      return PLATFORM_DEVICE_RESULT_INVALID;
    }
    g_platform_default_layer_ready = true;
  }

  return PLATFORM_DEVICE_RESULT_OK;
}
