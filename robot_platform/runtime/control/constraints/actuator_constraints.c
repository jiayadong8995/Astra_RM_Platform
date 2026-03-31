#include "actuator_constraints.h"

#include "../internal/balance_params.h"
#include "../../module/lib/control/control_math.h"
#include "user_lib.h"

static uint32_t g_platform_actuator_constraint_oracle_flags;

static void platform_constraint_mark(uint32_t flag)
{
    g_platform_actuator_constraint_oracle_flags |= flag;
}

void platform_actuator_constraint_oracle_reset(void)
{
    g_platform_actuator_constraint_oracle_flags = 0U;
}

uint32_t platform_actuator_constraint_oracle_flags(void)
{
    return g_platform_actuator_constraint_oracle_flags;
}

void platform_constrain_leg_outputs(vmc_leg_t *right_leg, vmc_leg_t *left_leg)
{
    const float right_f0 = right_leg->F0;
    const float right_joint_1 = right_leg->torque_set[1];
    const float right_joint_0 = right_leg->torque_set[0];
    const float left_f0 = left_leg->F0;
    const float left_joint_1 = left_leg->torque_set[1];
    const float left_joint_0 = left_leg->torque_set[0];

    mySaturate(&right_leg->F0, -100.0f, 100.0f);
    mySaturate(&right_leg->torque_set[1], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&right_leg->torque_set[0], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&left_leg->F0, -100.0f, 100.0f);
    mySaturate(&left_leg->torque_set[1], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&left_leg->torque_set[0], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);

    if (right_f0 != right_leg->F0
        || right_joint_1 != right_leg->torque_set[1]
        || right_joint_0 != right_leg->torque_set[0]
        || left_f0 != left_leg->F0
        || left_joint_1 != left_leg->torque_set[1]
        || left_joint_0 != left_leg->torque_set[0])
    {
        platform_constraint_mark(PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_LEG);
    }
}

void platform_constrain_wheel_outputs(platform_balance_runtime_t *chassis)
{
    for (int i = 0; i < 2; i++)
    {
        const float wheel_torque = chassis->wheel_motor[i].torque_set;
        const int16_t wheel_current = chassis->wheel_motor[i].give_current;

        mySaturate(&chassis->wheel_motor[i].torque_set, -WHEEL_TORQUE_MAX, WHEEL_TORQUE_MAX);
        platform_int16_clamp(&chassis->wheel_motor[i].give_current, -8000, 8000);

        if (wheel_torque != chassis->wheel_motor[i].torque_set)
        {
            platform_constraint_mark(PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_WHEEL);
        }
        if (wheel_current != chassis->wheel_motor[i].give_current)
        {
            platform_constraint_mark(PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_CURRENT);
        }
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
