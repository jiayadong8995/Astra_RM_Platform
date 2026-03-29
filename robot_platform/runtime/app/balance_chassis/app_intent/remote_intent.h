#ifndef BALANCE_CHASSIS_APP_INTENT_REMOTE_INTENT_H
#define BALANCE_CHASSIS_APP_INTENT_REMOTE_INTENT_H

#include "../../../control/contracts/device_input.h"
#include "../../../control/contracts/robot_intent.h"
#include "../../../control/contracts/robot_state.h"
#include "remote_intent_state.h"

void platform_remote_intent_state_init(platform_remote_intent_state_t *state);

void platform_remote_intent_state_apply_inputs(platform_remote_intent_state_t *state,
                                               const platform_rc_input_t *rc_input,
                                               const platform_robot_state_t *robot_state);

platform_robot_intent_t platform_remote_intent_build(const platform_remote_intent_state_t *state);

#endif
