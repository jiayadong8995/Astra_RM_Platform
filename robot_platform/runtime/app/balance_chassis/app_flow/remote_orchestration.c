#include "remote_orchestration.h"

#include <math.h>

#include "../app_config/app_params.h"
#include "remote_control.h"

static void slope_following(float *target, float *set, float acc);
static void local_saturate(float *in, float min, float max);

void remote_runtime_init(Remote_Runtime_t *runtime)
{
    runtime->start_flag = 0;
    runtime->x_filter = 0.0f;
    runtime->x_set = runtime->x_filter;
    runtime->v_set = 0.0f;
    runtime->turn_set = 0.0f;
    runtime->leg_set = LEG_LENGTH_DEFAULT;
}

void remote_runtime_apply_inputs(Remote_Runtime_t *runtime,
                                 const RC_Data_t *rc_data,
                                 const INS_Data_t *ins_msg,
                                 const Chassis_State_t *state_msg)
{
    runtime->myPithR = ins_msg->pitch;
    runtime->x_filter = state_msg->x_filter;
    runtime->total_yaw = state_msg->total_yaw;
    if (runtime->turn_set == 0.0f)
    {
        runtime->turn_set = state_msg->total_yaw;
    }

    runtime->last_recover_flag = runtime->recover_flag;
    if (!rc_data->online)
    {
        runtime->start_flag = 0;
        runtime->recover_flag = 0;
        runtime->jump_flag = 0;
    }
    else if (switch_is_mid(rc_data->sw[0]))
    {
        runtime->start_flag = 1;
        if (runtime->myPithR > PITCH_RECOVER_THRESHOLD || runtime->myPithR < -PITCH_RECOVER_THRESHOLD)
        {
            runtime->recover_flag = (rc_data->sw[1] == RC_SW_MID) ? 0U : 1U;
        }
    }
    else if (switch_is_down(rc_data->sw[0]))
    {
        runtime->start_flag = 0;
        runtime->recover_flag = 0;
        runtime->jump_flag = 0;
    }

    if (runtime->start_flag == 1)
    {
        if (switch_is_mid(rc_data->sw[1]))
        {
            runtime->jump_flag = (rc_data->ch[3] == 660) ? 1U : 0U;
        }
        else if (switch_is_down(rc_data->sw[1]))
        {
            runtime->jump_flag = 0;
        }

        runtime->leg_set = LEG_LENGTH_DEFAULT;
        runtime->turn_set = runtime->turn_set - rc_data->ch[2] * RC_TO_TURN;

        {
            float vx_speed_cmd = -rc_data->ch[1] * RC_TO_VX;
            slope_following(&vx_speed_cmd, &runtime->v_set, RC_SPEED_SLOPE);
            local_saturate(&runtime->v_set, -RC_VX_MAX, RC_VX_MAX);
        }

        runtime->x_set = runtime->x_set + runtime->v_set * (float)REMOTE_TASK_PERIOD_MS / 1000.0f;
    }
    else
    {
        runtime->v_set = 0.0f;
        runtime->x_set = runtime->x_filter;
        runtime->turn_set = runtime->total_yaw;
    }

    if (fabsf(runtime->turn_set - runtime->total_yaw) > 0.3f)
    {
        runtime->turn_set = ((runtime->turn_set - runtime->total_yaw) > 0.0f)
                          ? (runtime->total_yaw + 0.3f)
                          : (runtime->total_yaw - 0.3f);
    }

    local_saturate(&runtime->leg_set, LEG_LENGTH_MIN, LEG_LENGTH_MAX);
}

void remote_runtime_limit_leg_set(Remote_Runtime_t *runtime,
                                  const Leg_Output_t *right_msg,
                                  const Leg_Output_t *left_msg)
{
    float leg_length = (left_msg->leg_length + right_msg->leg_length) / 2.0f;
    float leg_cmd = runtime->leg_set - leg_length;
    if (fabsf(leg_cmd) > 0.1f)
    {
        runtime->leg_set = (leg_cmd > 0.0f) ? (leg_length + 0.1f) : (leg_length - 0.1f);
    }
}

Chassis_Cmd_t remote_runtime_build_cmd(const Remote_Runtime_t *runtime)
{
    Chassis_Cmd_t cmd = {
        .vx_cmd = runtime->v_set,
        .turn_cmd = runtime->turn_set,
        .leg_set = runtime->leg_set,
        .start_flag = runtime->start_flag,
        .jump_flag = runtime->jump_flag,
        .recover_flag = runtime->recover_flag,
    };
    return cmd;
}

static void slope_following(float *target, float *set, float acc)
{
    if (*target > *set)
    {
        *set = *set + acc;
        if (*set >= *target)
        {
            *set = *target;
        }
    }
    else if (*target < *set)
    {
        *set = *set - acc;
        if (*set <= *target)
        {
            *set = *target;
        }
    }
}

static void local_saturate(float *in, float min, float max)
{
    if (*in < min)
    {
        *in = min;
    }
    else if (*in > max)
    {
        *in = max;
    }
}
