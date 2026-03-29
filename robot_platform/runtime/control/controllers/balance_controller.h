#ifndef PLATFORM_CONTROL_CONTROLLERS_BALANCE_CONTROLLER_H
#define PLATFORM_CONTROL_CONTROLLERS_BALANCE_CONTROLLER_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "../internal/runtime_state.h"
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
} platform_balance_controller_t;

typedef struct
{
    INS_Data_t ins;
    Chassis_Cmd_t cmd;
    Chassis_Observe_t observe;
    Actuator_Feedback_t feedback;
} platform_balance_controller_input_t;

typedef struct
{
    Chassis_State_t state;
    Leg_Output_t right_leg;
    Leg_Output_t left_leg;
    Actuator_Cmd_t actuator_cmd;
} platform_balance_controller_output_t;

void platform_balance_controller_init(platform_balance_controller_t *state);
void platform_balance_controller_apply_inputs(platform_balance_controller_t *state,
                                              const platform_balance_controller_input_t *inputs);
void platform_balance_controller_step(platform_balance_controller_t *state, const Actuator_Feedback_t *feedback);
void platform_balance_controller_build_outputs(const platform_balance_controller_t *state,
                                               platform_balance_controller_output_t *outputs);

#endif
