#ifndef PLATFORM_MOTOR_ACTUATOR_DEVICE_H
#define PLATFORM_MOTOR_ACTUATOR_DEVICE_H

#include <stdint.h>

#include "motor_device.h"

typedef struct {
  void *joint_can_handle;
  void *wheel_can_handle;
  void *(*get_joint_state_fn)(uint8_t index);
  void *(*get_wheel_state_fn)(uint8_t index);
  void (*joint_init_fn)(void *motor, uint16_t id, uint16_t mode);
  int (*enable_mode_fn)(void *hcan, uint16_t motor_id, uint16_t mode_id);
  void (*mit_ctrl_fn)(void *hcan, uint16_t motor_id, float pos, float vel, float kp, float kd, float torq);
  void (*wheel_cmd_fn)(void *hcan, int16_t motor1, int16_t motor2, int16_t rev1, int16_t rev2);
} platform_motor_actuator_device_config_t;

void platform_motor_actuator_device_bind(platform_motor_device_t *device,
                                         const platform_motor_actuator_device_config_t *config);

#endif
