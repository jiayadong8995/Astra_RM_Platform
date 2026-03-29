#include "remote_intent.h"

#include <math.h>

#include "../app_config/app_params.h"
#include "../../../control/internal/balance_params.h"
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

void platform_remote_intent_state_init(platform_remote_intent_state_t *state)
{
    state->start_flag = 0;
    state->x_filter = 0.0f;
    state->x_set = state->x_filter;
    state->v_set = 0.0f;
    state->turn_set = 0.0f;
    state->leg_set = LEG_LENGTH_DEFAULT;
}

void platform_remote_intent_state_apply_inputs(platform_remote_intent_state_t *state,
                                               const platform_rc_input_t *rc_input,
                                               const platform_robot_state_t *robot_state)
{
    state->body_pitch = robot_state->body.pitch;
    state->x_filter = robot_state->chassis.x;
    state->total_yaw = robot_state->chassis.yaw_total;
    if (state->turn_set == 0.0f)
    {
        state->turn_set = robot_state->chassis.yaw_total;
    }

    state->last_recover_flag = state->recover_flag;
    if (!rc_input->valid)
    {
        state->start_flag = 0;
        state->recover_flag = 0;
        state->jump_flag = 0;
    }
    else if (app_rc_switch_is_mid(rc_input->switches[0]))
    {
        state->start_flag = 1;
        if (state->body_pitch > PITCH_RECOVER_THRESHOLD || state->body_pitch < -PITCH_RECOVER_THRESHOLD)
        {
            state->recover_flag = (rc_input->switches[1] == APP_RC_SW_MID) ? 0U : 1U;
        }
    }
    else if (app_rc_switch_is_down(rc_input->switches[0]))
    {
        state->start_flag = 0;
        state->recover_flag = 0;
        state->jump_flag = 0;
    }

    if (state->start_flag == 1)
    {
        if (app_rc_switch_is_mid(rc_input->switches[1]))
        {
            state->jump_flag = (rc_input->channels[3] == 660) ? 1U : 0U;
        }
        else if (app_rc_switch_is_down(rc_input->switches[1]))
        {
            state->jump_flag = 0;
        }

        state->leg_set = LEG_LENGTH_DEFAULT;
        platform_constrain_remote_leg_set(&state->leg_set,
                                          (robot_state->legs.left.length + robot_state->legs.right.length) / 2.0f,
                                          0.1f);
        state->turn_set = state->turn_set - rc_input->channels[2] * RC_TO_TURN;

        {
            float vx_speed_cmd = -rc_input->channels[1] * RC_TO_VX;
            platform_slew_to_target(vx_speed_cmd, &state->v_set, RC_SPEED_SLOPE);
            platform_float_clamp(&state->v_set, -RC_VX_MAX, RC_VX_MAX);
        }

        state->x_set = state->x_set + state->v_set * (float)REMOTE_TASK_PERIOD_MS / 1000.0f;
    }
    else
    {
        state->v_set = 0.0f;
        state->x_set = state->x_filter;
        state->turn_set = state->total_yaw;
    }

    platform_constrain_remote_turn(&state->turn_set, state->total_yaw, 0.3f);
    platform_float_clamp(&state->leg_set, LEG_LENGTH_MIN, LEG_LENGTH_MAX);
}

platform_robot_intent_t platform_remote_intent_build(const platform_remote_intent_state_t *state)
{
    platform_robot_intent_t intent = {
        .mode = PLATFORM_ROBOT_MODE_IDLE,
        .motion_target = {
            .vx = state->v_set,
            .x = state->x_set,
            .yaw_target = state->turn_set,
            .yaw_rate = 0.0f,
            .yaw_hold = true,
            .velocity_frame = PLATFORM_FRAME_BODY,
        },
        .posture_target = {
            .leg_length = state->leg_set,
            .body_pitch_ref = 0.0f,
            .stance_height = state->leg_set,
        },
        .behavior_request = {
            .jump_request = (state->jump_flag != 0U),
            .recover_request = (state->recover_flag != 0U),
            .stand_request = (state->start_flag != 0U),
            .emergency_stop = (state->start_flag == 0U),
        },
        .enable = {
            .start = (state->start_flag != 0U),
            .control_enable = (state->start_flag != 0U),
            .actuator_enable = (state->start_flag != 0U),
        },
    };

    if (state->start_flag != 0U)
    {
        intent.mode = (state->recover_flag != 0U) ? PLATFORM_ROBOT_MODE_RECOVER : PLATFORM_ROBOT_MODE_ACTIVE;
        if (state->jump_flag != 0U)
        {
            intent.mode = PLATFORM_ROBOT_MODE_JUMP;
        }
    }

    return intent;
}
