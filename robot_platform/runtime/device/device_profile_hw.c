#include "device_profile.h"

#include "device_layer.h"

#include "imu/bmi088/BMI088driver.h"
#include "actuator/motor/dm4310/dm4310_drv.h"
#include "remote/dbus/remote_control.h"
#include "imu/bmi088_device.h"
#include "remote/dbus_remote_device.h"
#include "fdcan.h"
#include "spi.h"
#include "actuator/motor/motor_actuator_device.h"

static void bind_hw_imu(platform_device_layer_t *layer);
static void bind_hw_remote(platform_device_layer_t *layer);
static void bind_hw_motor(platform_device_layer_t *layer);

static const platform_bmi088_device_config_t g_platform_bmi088_hw = {
  .spi_handle = &hspi2,
  .sample_state = &BMI088,
  .calibrate = 1U,
  .init_fn = (void (*)(void *, uint8_t))BMI088_Init,
  .read_fn = (void (*)(void *))BMI088_Read,
};

static const platform_dbus_remote_device_config_t g_platform_remote_hw = {
  .acquire_fn = (const void *(*)(void))get_remote_control_point,
  .is_error_fn = RC_data_is_error,
};

static const platform_motor_actuator_device_config_t g_platform_motor_hw = {
  .joint_can_handle = &hfdcan1,
  .wheel_can_handle = &hfdcan2,
  .get_joint_state_fn = (void *(*)(uint8_t))get_joint_motor_state,
  .get_wheel_state_fn = (void *(*)(uint8_t))get_chassis_motor_measure_point,
  .joint_init_fn = (void (*)(void *, uint16_t, uint16_t))joint_motor_init,
  .enable_mode_fn = (int (*)(void *, uint16_t, uint16_t))enable_motor_mode,
  .mit_ctrl_fn = (void (*)(void *, uint16_t, float, float, float, float, float))mit_ctrl,
  .wheel_cmd_fn = (void (*)(void *, int16_t, int16_t, int16_t, int16_t))CAN_cmd_chassis,
};

static const platform_device_profile_t g_platform_device_profile_hw = {
  .name = "hw",
  .bind_imu = bind_hw_imu,
  .bind_remote = bind_hw_remote,
  .bind_motor = bind_hw_motor,
};

const platform_device_profile_t *platform_device_profile_hw(void)
{
  return &g_platform_device_profile_hw;
}

static void bind_hw_imu(platform_device_layer_t *layer)
{
  platform_bmi088_device_bind(&layer->imu, &g_platform_bmi088_hw);
}

static void bind_hw_remote(platform_device_layer_t *layer)
{
  platform_dbus_remote_device_bind(&layer->remote, &g_platform_remote_hw);
}

static void bind_hw_motor(platform_device_layer_t *layer)
{
  platform_motor_actuator_device_bind(&layer->motor, &g_platform_motor_hw);
}
