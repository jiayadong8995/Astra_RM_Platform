#ifndef BALANCE_CHASSIS_APP_FLOW_CHASSIS_ORCHESTRATION_H
#define BALANCE_CHASSIS_APP_FLOW_CHASSIS_ORCHESTRATION_H

#include "../app_io/topic_contract.h"
#include "../legacy/INS_task.h"
#include "../legacy/chassis_task.h"
#include "pid.h"
#include "VMC_calc.h"

typedef struct
{
    INS_t ins;
    chassis_t chassis;
    vmc_leg_t right_leg;
    vmc_leg_t left_leg;
    PidTypeDef leg_r_pid;
    PidTypeDef leg_l_pid;
    PidTypeDef tp_pid;
    PidTypeDef turn_pid;
    PidTypeDef roll_pid;
} Chassis_Runtime_State_t;

void chassis_runtime_state_init(Chassis_Runtime_State_t *state);

void chassis_runtime_apply_bus_inputs(Chassis_Runtime_State_t *state, const Chassis_Bus_Input_t *inputs);

void chassis_runtime_step(Chassis_Runtime_State_t *state, const Actuator_Feedback_t *feedback);

void chassis_runtime_build_bus_outputs(const Chassis_Runtime_State_t *state, Chassis_Bus_Output_t *outputs);

#endif
