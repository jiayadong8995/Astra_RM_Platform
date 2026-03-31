#ifndef PLATFORM_CONTROL_CONSTRAINTS_ACTUATOR_CONSTRAINTS_H
#define PLATFORM_CONTROL_CONSTRAINTS_ACTUATOR_CONSTRAINTS_H

#include "../internal/balance_runtime.h"
#include "VMC_calc.h"

#define PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_WHEEL  (1U << 0)
#define PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_CURRENT (1U << 1)
#define PLATFORM_ACTUATOR_CONSTRAINT_ORACLE_LEG    (1U << 2)

void platform_constrain_leg_outputs(vmc_leg_t *right_leg, vmc_leg_t *left_leg);
void platform_constrain_wheel_outputs(platform_balance_runtime_t *chassis);
void platform_constrain_remote_turn(float *turn_set, float total_yaw, float max_delta);
void platform_constrain_remote_leg_set(float *leg_set, float current_leg_length, float max_delta);
void platform_actuator_constraint_oracle_reset(void);
uint32_t platform_actuator_constraint_oracle_flags(void);

#endif
