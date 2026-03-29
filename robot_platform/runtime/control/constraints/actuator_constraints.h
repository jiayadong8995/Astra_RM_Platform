#ifndef PLATFORM_CONTROL_CONSTRAINTS_ACTUATOR_CONSTRAINTS_H
#define PLATFORM_CONTROL_CONSTRAINTS_ACTUATOR_CONSTRAINTS_H

#include "../internal/runtime_state.h"
#include "VMC_calc.h"

void platform_constrain_leg_outputs(vmc_leg_t *right_leg, vmc_leg_t *left_leg);
void platform_constrain_wheel_outputs(chassis_t *chassis);
void platform_constrain_remote_turn(float *turn_set, float total_yaw, float max_delta);
void platform_constrain_remote_leg_set(float *leg_set, float current_leg_length, float max_delta);

#endif
