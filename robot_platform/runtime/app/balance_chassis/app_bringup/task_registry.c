#include "task_registry.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "../app_config/app_params.h"
#include "../../../bsp/ports.h"
#include "../../../control/controllers/chassis_control_task.h"
#include "../../../control/execution/motor_control_task.h"
#include "../../../control/state/ins_task.h"
#include "../../../control/state/observe_task.h"
#include "../app_intent/remote_task.h"
#include "../app_detect/detect_task.h"

osThreadId INS_TASKHandle;
osThreadId CHASSIS_TASKHandle;
osThreadId MOTOR_CONTROL_TASKHandle;
osThreadId OBSERVE_TASKHandle;
osThreadId REMOTE_TASKHandle;
osThreadId DETECT_TASKHandle;

static void INS_Thread(void const *argument)
{
    (void)argument;
    INS_task();
}

static void Chassis_Control_Thread(void const *argument)
{
    (void)argument;
    platform_chassis_control_task();
}

static void Motor_Control_Thread(void const *argument)
{
    (void)argument;
    motor_control_task();
}

static void Observe_Thread(void const *argument)
{
    (void)argument;
    Observe_task();
}

static void Remote_Thread(void const *argument)
{
    (void)argument;
    remote_task();
}

static void Detect_Thread(void const *argument)
{
    (void)argument;
    detect_task();
}

void balance_chassis_start_tasks(void)
{
    platform_ports_init();

    osThreadDef(INS_TASK, INS_Thread, APP_INS_TASK_PRIORITY, 0, APP_INS_TASK_STACK_BYTES);
    INS_TASKHandle = osThreadCreate(osThread(INS_TASK), NULL);

    osThreadDef(CHASSIS_CONTROL_TASK, Chassis_Control_Thread, APP_CHASSIS_TASK_PRIORITY, 0, APP_CHASSIS_TASK_STACK_BYTES);
    CHASSIS_TASKHandle = osThreadCreate(osThread(CHASSIS_CONTROL_TASK), NULL);

    osThreadDef(MOTOR_CONTROL_TASK, Motor_Control_Thread, APP_MOTOR_CONTROL_TASK_PRIORITY, 0, APP_MOTOR_CONTROL_STACK_BYTES);
    MOTOR_CONTROL_TASKHandle = osThreadCreate(osThread(MOTOR_CONTROL_TASK), NULL);

    osThreadDef(OBSERVE_TASK, Observe_Thread, APP_OBSERVE_TASK_PRIORITY, 0, APP_OBSERVE_TASK_STACK_BYTES);
    OBSERVE_TASKHandle = osThreadCreate(osThread(OBSERVE_TASK), NULL);

    osThreadDef(REMOTE_TASK, Remote_Thread, APP_REMOTE_TASK_PRIORITY, 0, APP_REMOTE_TASK_STACK_BYTES);
    REMOTE_TASKHandle = osThreadCreate(osThread(REMOTE_TASK), NULL);

    osThreadDef(DETECT_TASK, Detect_Thread, APP_DETECT_TASK_PRIORITY, 0, APP_DETECT_TASK_STACK_BYTES);
    DETECT_TASKHandle = osThreadCreate(osThread(DETECT_TASK), NULL);
}
