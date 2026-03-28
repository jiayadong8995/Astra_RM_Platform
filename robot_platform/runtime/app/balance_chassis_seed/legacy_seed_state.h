#ifndef LEGACY_SEED_STATE_H
#define LEGACY_SEED_STATE_H

#include "dm4310_drv.h"

typedef struct
{
    Joint_Motor_t joint_motor[4];
    chassis_motor_t wheel_motor[2];
} chassis_seed_state_t;

Joint_Motor_t *platform_joint_motor_state(uint8_t index);
uint8_t platform_consume_begin_flag(void);

#endif
