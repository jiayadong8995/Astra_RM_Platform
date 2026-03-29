#include "remote_orchestration.h"

#include <math.h>

#include "../app_config/app_params.h"
#include "../../../control/constraints/actuator_constraints.h"
#include "../../../module/lib/control/control_math.h"

#define APP_RC_SW_UP   1U
#define APP_RC_SW_MID  3U
#define APP_RC_SW_DOWN 2U

static uint8_t app_rc_switch_is_mid(uint8_t sw)
{
    return (sw == APP_RC_SW_MID) ? 1U : 0U;
}

static uint8_t app_rc_switch_is_down(uint8_t sw)
{
    return (sw == APP_RC_SW_DOWN) ? 1U : 0U;
}

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
                                 const platform_rc_input_t *rc_input,
                                 const platform_robot_state_t *robot_state)
{
    runtime->myPithR = robot_state->body.pitch;
    runtime->x_filter = robot_state->chassis.x;
    runtime->total_yaw = robot_state->chassis.yaw_total;
    if (runtime->turn_set == 0.0f)
    {
        runtime->turn_set = robot_state->chassis.yaw_total;
    }

    runtime->last_recover_flag = runtime->recover_flag;
    if (!rc_input->valid)
    {
        runtime->start_flag = 0;
        runtime->recover_flag = 0;
        runtime->jump_flag = 0;
    }
    else if (app_rc_switch_is_mid(rc_input->switches[0]))
    {
        runtime->start_flag = 1;
        if (runtime->myPithR > PITCH_RECOVER_THRESHOLD || runtime->myPithR < -PITCH_RECOVER_THRESHOLD)
        {
            runtime->recover_flag = (rc_input->switches[1] == APP_RC_SW_MID) ? 0U : 1U;
        }
    }
    else if (app_rc_switch_is_down(rc_input->switches[0]))
    {
        runtime->start_flag = 0;
        runtime->recover_flag = 0;
        runtime->jump_flag = 0;
    }

    if (runtime->start_flag == 1)
    {
        if (app_rc_switch_is_mid(rc_input->switches[1]))
        {
            runtime->jump_flag = (rc_input->channels[3] == 660) ? 1U : 0U;
        }
        else if (app_rc_switch_is_down(rc_input->switches[1]))
        {
            runtime->jump_flag = 0;
        }

        runtime->leg_set = LEG_LENGTH_DEFAULT;
        platform_constrain_remote_leg_set(&runtime->leg_set,
                                          (robot_state->legs.left.length + robot_state->legs.right.length) / 2.0f,
                                          0.1f);
        runtime->turn_set = runtime->turn_set - rc_input->channels[2] * RC_TO_TURN;

        {
            float vx_speed_cmd = -rc_input->channels[1] * RC_TO_VX;
            platform_slew_to_target(vx_speed_cmd, &runtime->v_set, RC_SPEED_SLOPE);
            platform_float_clamp(&runtime->v_set, -RC_VX_MAX, RC_VX_MAX);
        }

        runtime->x_set = runtime->x_set + runtime->v_set * (float)REMOTE_TASK_PERIOD_MS / 1000.0f;
    }
    else
    {
        runtime->v_set = 0.0f;
        runtime->x_set = runtime->x_filter;
        runtime->turn_set = runtime->total_yaw;
    }

    platform_constrain_remote_turn(&runtime->turn_set, runtime->total_yaw, 0.3f);
    platform_float_clamp(&runtime->leg_set, LEG_LENGTH_MIN, LEG_LENGTH_MAX);
}

platform_robot_intent_t remote_runtime_build_intent(const Remote_Runtime_t *runtime)
{
    platform_robot_intent_t intent = {
        .mode = PLATFORM_ROBOT_MODE_IDLE,
        .motion_target = {
            .vx = runtime->v_set,
            .x = runtime->x_set,
            .yaw_target = runtime->turn_set,
            .yaw_rate = 0.0f,
            .yaw_hold = true,
            .velocity_frame = PLATFORM_FRAME_BODY,
        },
        .posture_target = {
            .leg_length = runtime->leg_set,
            .body_pitch_ref = 0.0f,
            .stance_height = runtime->leg_set,
        },
        .behavior_request = {
            .jump_request = (runtime->jump_flag != 0U),
            .recover_request = (runtime->recover_flag != 0U),
            .stand_request = (runtime->start_flag != 0U),
            .emergency_stop = (runtime->start_flag == 0U),
        },
        .enable = {
            .start = (runtime->start_flag != 0U),
            .control_enable = (runtime->start_flag != 0U),
            .actuator_enable = (runtime->start_flag != 0U),
        },
    };

    if (runtime->start_flag != 0U)
    {
        intent.mode = (runtime->recover_flag != 0U) ? PLATFORM_ROBOT_MODE_RECOVER : PLATFORM_ROBOT_MODE_ACTIVE;
        if (runtime->jump_flag != 0U)
        {
            intent.mode = PLATFORM_ROBOT_MODE_JUMP;
        }
    }

    return intent;
}
