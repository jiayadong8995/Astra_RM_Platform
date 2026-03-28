#ifndef BALANCE_CHASSIS_APP_LOGIC_REMOTE_RUNTIME_H
#define BALANCE_CHASSIS_APP_LOGIC_REMOTE_RUNTIME_H

#include "../app_config/robot_def.h"

typedef struct
{
    float v_set;
    float x_set;
    float x_filter;
    float turn_set;
    float total_yaw;
    float leg_set;
    float myPithR;
    uint8_t start_flag;
    uint8_t jump_flag;
    uint8_t recover_flag;
    uint8_t last_recover_flag;
} Remote_Runtime_t;

#endif
