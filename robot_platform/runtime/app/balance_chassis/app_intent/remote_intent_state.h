#ifndef BALANCE_CHASSIS_APP_INTENT_REMOTE_INTENT_STATE_H
#define BALANCE_CHASSIS_APP_INTENT_REMOTE_INTENT_STATE_H

#include <stdint.h>

typedef struct
{
    float v_set;
    float x_set;
    float x_filter;
    float turn_set;
    float total_yaw;
    float leg_set;
    float body_pitch;
    uint8_t start_flag;
    uint8_t jump_flag;
    uint8_t recover_flag;
    uint8_t last_recover_flag;
} platform_remote_intent_state_t;

#endif
