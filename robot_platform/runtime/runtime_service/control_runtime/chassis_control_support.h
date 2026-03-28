#ifndef BALANCE_CHASSIS_RUNTIME_SERVICE_CONTROL_RUNTIME_CHASSIS_CONTROL_SUPPORT_H
#define BALANCE_CHASSIS_RUNTIME_SERVICE_CONTROL_RUNTIME_CHASSIS_CONTROL_SUPPORT_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "../../app/balance_chassis/app_config/runtime_state.h"
#include "pid.h"
#include "VMC_calc.h"

void chassis_apply_feedback_snapshot(chassis_t *chassis,
                                     vmc_leg_t *vmc_r,
                                     vmc_leg_t *vmc_l,
                                     const INS_t *ins,
                                     const Actuator_Feedback_t *feedback);

void chassis_compute_turn_and_leg_compensation(chassis_t *chassis,
                                               const INS_t *ins,
                                               const PidTypeDef *turn_pid,
                                               const PidTypeDef *roll_pid,
                                               PidTypeDef *tp_pid);

void chassis_compute_lqr_outputs(chassis_t *chassis,
                                 vmc_leg_t *vmcr,
                                 vmc_leg_t *vmcl,
                                 const float *LQR_KR,
                                 const float *LQR_KL);

void chassis_mix_wheel_torque(chassis_t *chassis);

void chassis_apply_jump_logic(chassis_t *chassis,
                              vmc_leg_t *vmcr,
                              vmc_leg_t *vmcl,
                              PidTypeDef *legr,
                              PidTypeDef *legl);

void chassis_apply_ground_detection(chassis_t *chassis,
                                    vmc_leg_t *vmcr,
                                    vmc_leg_t *vmcl,
                                    INS_t *ins);

void chassis_saturate_outputs(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl);

#endif
