#ifndef PLATFORM_DEVICE_LAYER_H
#define PLATFORM_DEVICE_LAYER_H

#include "actuator/actuator_device.h"
#include "actuator/motor/motor_device.h"
#include "device_types.h"
#include "imu/imu_device.h"
#include "remote/remote_device.h"

typedef struct {
  platform_imu_device_t imu;
  platform_remote_device_t remote;
  platform_motor_device_t motor;
  platform_device_backend_profile_t backend_profile;
  uint32_t input_sequence;
  uint32_t feedback_sequence;
  uint32_t command_sequence;
} platform_device_layer_t;

platform_device_result_t platform_device_layer_init(platform_device_layer_t *layer);
platform_device_result_t platform_device_layer_init_profile(platform_device_layer_t *layer,
                                                            platform_device_backend_profile_t profile);

platform_device_result_t platform_device_layer_read_input(platform_device_layer_t *layer,
                                                          platform_device_input_t *input);

platform_device_result_t platform_device_layer_read_remote(platform_device_layer_t *layer,
                                                           platform_rc_input_t *input);

platform_device_result_t platform_device_layer_read_imu(platform_device_layer_t *layer,
                                                        platform_imu_sample_t *sample);

platform_device_result_t platform_device_layer_write_command(platform_device_layer_t *layer,
                                                             const platform_device_command_t *command);

platform_device_result_t platform_device_layer_read_feedback(platform_device_layer_t *layer,
                                                             platform_device_feedback_t *feedback);

platform_device_result_t platform_device_configure_default_profile(platform_device_backend_profile_t profile);
platform_device_result_t platform_device_init_default_profile(void);
platform_device_result_t platform_device_read_default_remote(platform_rc_input_t *input);
platform_device_result_t platform_device_read_default_imu(platform_imu_sample_t *sample);
platform_device_result_t platform_device_read_default_feedback(platform_device_feedback_t *feedback);
platform_device_result_t platform_device_write_default_command(const platform_device_command_t *command);

#endif
