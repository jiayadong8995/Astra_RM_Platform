#include "legacy_seed_state.h"

static uint8_t begin_flag = 1;
static chassis_seed_state_t chassis_move = {0};

Joint_Motor_t *platform_joint_motor_state(uint8_t index)
{
    if (index >= 4U)
    {
        return &chassis_move.joint_motor[0];
    }
    return &chassis_move.joint_motor[index];
}

uint8_t platform_consume_begin_flag(void)
{
    uint8_t current = begin_flag;
    begin_flag = 0;
    return current;
}
