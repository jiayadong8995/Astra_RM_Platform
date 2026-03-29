#include "actuator_constraints.h"

#include "../internal/balance_params.h"
#include "../../module/lib/control/control_math.h"
#include "user_lib.h"

void platform_constrain_leg_outputs(vmc_leg_t *right_leg, vmc_leg_t *left_leg)
{
    mySaturate(&right_leg->F0, -100.0f, 100.0f);
    mySaturate(&right_leg->torque_set[1], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&right_leg->torque_set[0], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&left_leg->F0, -100.0f, 100.0f);
    mySaturate(&left_leg->torque_set[1], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&left_leg->torque_set[0], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
}

void platform_constrain_wheel_outputs(platform_balance_runtime_t *chassis)
{
    for (int i = 0; i < 2; i++)
    {
        mySaturate(&chassis->wheel_motor[i].torque_set, -WHEEL_TORQUE_MAX, WHEEL_TORQUE_MAX);
        platform_int16_clamp(&chassis->wheel_motor[i].give_current, -8000, 8000);
    }
}

void platform_constrain_remote_turn(float *turn_set, float total_yaw, float max_delta)
{
    float delta = *turn_set - total_yaw;
    if (delta > max_delta)
    {
        *turn_set = total_yaw + max_delta;
    }
    else if (delta < -max_delta)
    {
        *turn_set = total_yaw - max_delta;
    }
}

void platform_constrain_remote_leg_set(float *leg_set, float current_leg_length, float max_delta)
{
    float leg_delta = *leg_set - current_leg_length;
    if (leg_delta > max_delta)
    {
        *leg_set = current_leg_length + max_delta;
    }
    else if (leg_delta < -max_delta)
    {
        *leg_set = current_leg_length - max_delta;
    }
}
