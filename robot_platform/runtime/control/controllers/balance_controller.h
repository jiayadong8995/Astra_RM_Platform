#ifndef PLATFORM_CONTROL_CONTROLLERS_BALANCE_CONTROLLER_H
#define PLATFORM_CONTROL_CONTROLLERS_BALANCE_CONTROLLER_H

#include "../contracts/actuator_command.h"
#include "../contracts/device_feedback.h"
#include "../contracts/robot_intent.h"
#include "../contracts/robot_state.h"
#include "../internal/balance_runtime.h"
#include "../internal/ins_runtime.h"
#include "../state/chassis_observe_message.h"
#include "../state/ins_state_message.h"
#include "pid.h"
#include "VMC_calc.h"

typedef struct
{
    platform_ins_runtime_t ins;
    platform_balance_runtime_t chassis;
    platform_robot_state_t robot_state;
    platform_actuator_command_t actuator_command;
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
    platform_ins_state_message_t ins;
    platform_robot_intent_t intent;
    platform_chassis_observe_message_t observe;
    platform_device_feedback_t feedback;
} platform_balance_controller_input_t;

typedef struct
{
    platform_robot_state_t robot_state;
    platform_actuator_command_t actuator_command;
} platform_balance_controller_output_t;

void platform_balance_controller_init(platform_balance_controller_t *state);
void platform_balance_controller_apply_inputs(platform_balance_controller_t *state,
                                              const platform_balance_controller_input_t *inputs);
void platform_balance_controller_step(platform_balance_controller_t *state, const platform_device_feedback_t *feedback);
void platform_balance_controller_build_outputs(const platform_balance_controller_t *state,
                                               platform_balance_controller_output_t *outputs);

#endif
