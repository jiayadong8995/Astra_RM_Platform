#ifndef PLATFORM_CONTROL_STATE_INS_STATE_ESTIMATOR_H
#define PLATFORM_CONTROL_STATE_INS_STATE_ESTIMATOR_H

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "../../app/balance_chassis/app_config/runtime_state.h"
#include "../contracts/robot_state.h"
#include "mahony_filter.h"

typedef struct
{
    INS_t ins;
    struct MAHONY_FILTER_t mahony;
    Axis3f gyro;
    Axis3f accel;
    float gravity[3];
    uint32_t dwt_count;
    float ins_time;
} platform_ins_state_estimator_t;

void platform_ins_state_estimator_init(platform_ins_state_estimator_t *state);
void platform_ins_state_estimator_apply_sample(platform_ins_state_estimator_t *state,
                                               float dt,
                                               const float accel[3],
                                               const float gyro[3]);
void platform_ins_state_estimator_build_msg(const platform_ins_state_estimator_t *state, INS_Data_t *msg);
void platform_ins_state_estimator_fill_robot_state(const platform_ins_state_estimator_t *state,
                                                   platform_robot_state_t *robot_state);

#endif
