#ifndef BALANCE_CHASSIS_APP_FLOW_REMOTE_ORCHESTRATION_H
#define BALANCE_CHASSIS_APP_FLOW_REMOTE_ORCHESTRATION_H

#include "../../../control/contracts/device_input.h"
#include "../../../control/contracts/robot_intent.h"
#include "../../../control/contracts/robot_state.h"
#include "remote_runtime.h"

void remote_runtime_init(Remote_Runtime_t *runtime);

void remote_runtime_apply_inputs(Remote_Runtime_t *runtime,
                                 const platform_rc_input_t *rc_input,
                                 const platform_robot_state_t *robot_state);

platform_robot_intent_t remote_runtime_build_intent(const Remote_Runtime_t *runtime);
Chassis_Cmd_t remote_runtime_build_cmd_from_intent(const platform_robot_intent_t *intent);

#endif
