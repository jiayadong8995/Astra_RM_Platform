#ifndef BALANCE_CHASSIS_APP_FLOW_OBSERVE_RUNTIME_H
#define BALANCE_CHASSIS_APP_FLOW_OBSERVE_RUNTIME_H

#include "../app_config/robot_def.h"

typedef struct
{
    float v_filter;
    float x_filter;
} Observe_Runtime_t;

#endif
