#include "motor_node.h"

#include "dm4310_drv.h"
#include "fdcan.h"

static const platform_motor_actuator_device_config_t g_platform_motor_node = {
    .joint_can_handle = &hfdcan1,
    .wheel_can_handle = &hfdcan2,
    .get_joint_state_fn = (void *(*)(uint8_t))get_joint_motor_state,
    .get_wheel_state_fn = (void *(*)(uint8_t))get_chassis_motor_measure_point,
    .joint_init_fn = (void (*)(void *, uint16_t, uint16_t))joint_motor_init,
    .enable_mode_fn = (int (*)(void *, uint16_t, uint16_t))enable_motor_mode,
    .mit_ctrl_fn = (void (*)(void *, uint16_t, float, float, float, float, float))mit_ctrl,
    .wheel_cmd_fn = (void (*)(void *, int16_t, int16_t, int16_t, int16_t))CAN_cmd_chassis,
};

const platform_motor_actuator_device_config_t *platform_motor_node_default(void)
{
    return &g_platform_motor_node;
}
