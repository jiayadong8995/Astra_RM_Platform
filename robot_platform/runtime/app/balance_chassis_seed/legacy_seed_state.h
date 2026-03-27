#ifndef LEGACY_SEED_STATE_H
#define LEGACY_SEED_STATE_H

#include "dm4310_drv.h"

typedef struct
{
    Joint_Motor_t joint_motor[4];
    chassis_motor_t wheel_motor[2];
} chassis_seed_state_t;

extern int Begin_flag;
extern chassis_seed_state_t chassis_move;

#endif
