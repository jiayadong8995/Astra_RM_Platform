#ifndef BALANCE_CHASSIS_APP_FLOW_REMOTE_ORCHESTRATION_H
#define BALANCE_CHASSIS_APP_FLOW_REMOTE_ORCHESTRATION_H

#include "remote_runtime.h"

void remote_runtime_init(Remote_Runtime_t *runtime);

void remote_runtime_apply_inputs(Remote_Runtime_t *runtime,
                                 const RC_Data_t *rc_data,
                                 const INS_Data_t *ins_msg,
                                 const Chassis_State_t *state_msg);

void remote_runtime_limit_leg_set(Remote_Runtime_t *runtime,
                                  const Leg_Output_t *right_msg,
                                  const Leg_Output_t *left_msg);

Chassis_Cmd_t remote_runtime_build_cmd(const Remote_Runtime_t *runtime);

#endif
