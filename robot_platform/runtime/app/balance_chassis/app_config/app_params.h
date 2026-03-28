#ifndef BALANCE_CHASSIS_APP_CONFIG_APP_PARAMS_H
#define BALANCE_CHASSIS_APP_CONFIG_APP_PARAMS_H

#include "cmsis_os.h"

#define INS_TASK_PERIOD_MS               1U
#define CHASSIS_TASK_PERIOD_MS           2U
#define OBSERVE_TASK_PERIOD_MS           3U
#define REMOTE_TASK_PERIOD_MS            10U

#define APP_DEFAULT_TASK_STACK_BYTES     512U

#define APP_INS_TASK_STACK_BYTES         2048U
#define APP_CHASSIS_TASK_STACK_BYTES     4096U
#define APP_MOTOR_CONTROL_STACK_BYTES    2048U
#define APP_OBSERVE_TASK_STACK_BYTES     2048U
#define APP_RC_INPUT_TASK_STACK_BYTES    1024U
#define APP_REMOTE_TASK_STACK_BYTES      2048U

#define APP_DEFAULT_TASK_PRIORITY        osPriorityNormal
#define APP_INS_TASK_PRIORITY            osPriorityRealtime
#define APP_CHASSIS_TASK_PRIORITY        osPriorityAboveNormal
#define APP_MOTOR_CONTROL_TASK_PRIORITY  osPriorityAboveNormal
#define APP_OBSERVE_TASK_PRIORITY        osPriorityHigh
#define APP_RC_INPUT_TASK_PRIORITY       osPriorityAboveNormal
#define APP_REMOTE_TASK_PRIORITY         osPriorityAboveNormal

#define APP_CHASSIS_STARTUP_DELAY_MS     6U

#endif
