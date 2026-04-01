#ifndef PLATFORM_CONTROL_CONFIG_CONTROL_TASK_PARAMS_H
#define PLATFORM_CONTROL_CONFIG_CONTROL_TASK_PARAMS_H

/* Control-task timing and priority parameters.
 * Values are grounded in the balance_chassis proving path.
 * Uses CONTROL_ prefix to signal control-layer ownership.
 */

#include "cmsis_os.h"

/* Task stack sizes (bytes) */
#define CONTROL_INS_TASK_STACK_BYTES            2048U
#define CONTROL_CHASSIS_TASK_STACK_BYTES         4096U
#define CONTROL_MOTOR_CONTROL_STACK_BYTES        2048U
#define CONTROL_OBSERVE_TASK_STACK_BYTES         2048U

/* Task priorities */
#define CONTROL_INS_TASK_PRIORITY               osPriorityRealtime
#define CONTROL_CHASSIS_TASK_PRIORITY            osPriorityAboveNormal
#define CONTROL_MOTOR_CONTROL_TASK_PRIORITY      osPriorityAboveNormal
#define CONTROL_OBSERVE_TASK_PRIORITY            osPriorityHigh

/* Task periods (ms) */
#define CONTROL_INS_TASK_PERIOD_MS              1U
#define CONTROL_CHASSIS_TASK_PERIOD_MS           2U
#define CONTROL_MOTOR_CONTROL_TASK_PERIOD_MS     1U
#define CONTROL_OBSERVE_TASK_PERIOD_MS           3U

/* Startup delay */
#define CONTROL_CHASSIS_STARTUP_DELAY_MS         6U

/* Backward-compatible aliases for unprefixed macros used by control task code.
 * These allow control sources to include this header instead of app_params.h.
 */
#define INS_TASK_PERIOD_MS              CONTROL_INS_TASK_PERIOD_MS
#define CHASSIS_TASK_PERIOD_MS          CONTROL_CHASSIS_TASK_PERIOD_MS
#define MOTOR_CONTROL_TASK_PERIOD_MS    CONTROL_MOTOR_CONTROL_TASK_PERIOD_MS
#define OBSERVE_TASK_PERIOD_MS          CONTROL_OBSERVE_TASK_PERIOD_MS
#define APP_CHASSIS_STARTUP_DELAY_MS    CONTROL_CHASSIS_STARTUP_DELAY_MS

#endif
