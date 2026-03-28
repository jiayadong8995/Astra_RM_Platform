#include "task_registry.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "rc_input_bridge.h"

#include "../app_config/app_params.h"
#include "../legacy/INS_task.h"
#include "../legacy/chassis_task.h"
#include "../legacy/motor_control_task.h"
#include "../legacy/observe_task.h"
#include "../legacy/remote_task.h"

osThreadId INS_TASKHandle;
osThreadId CHASSIS_TASKHandle;
osThreadId MOTOR_CONTROL_TASKHandle;
osThreadId OBSERVE_TASKHandle;
osThreadId REMOTE_TASKHandle;
osThreadId RC_INPUT_TASKHandle;

static void INS_Task(void const *argument);
static void Chassis_Task(void const *argument);
static void Motor_Control_Task(void const *argument);
static void OBSERVE_Task(void const *argument);
static void Remote_Task(void const *argument);
static void RC_Input_Task(void const *argument);

void balance_chassis_start_tasks(void)
{
    osThreadDef(INS_TASK, INS_Task, APP_INS_TASK_PRIORITY, 0, APP_INS_TASK_STACK_BYTES);
    INS_TASKHandle = osThreadCreate(osThread(INS_TASK), NULL);

    osThreadDef(CHASSISR_TASK, Chassis_Task, APP_CHASSIS_TASK_PRIORITY, 0, APP_CHASSIS_TASK_STACK_BYTES);
    CHASSIS_TASKHandle = osThreadCreate(osThread(CHASSISR_TASK), NULL);

    osThreadDef(CHASSISL_TASK, Motor_Control_Task, APP_MOTOR_CONTROL_TASK_PRIORITY, 0, APP_MOTOR_CONTROL_STACK_BYTES);
    MOTOR_CONTROL_TASKHandle = osThreadCreate(osThread(CHASSISL_TASK), NULL);

    osThreadDef(OBSERVE_TASK, OBSERVE_Task, APP_OBSERVE_TASK_PRIORITY, 0, APP_OBSERVE_TASK_STACK_BYTES);
    OBSERVE_TASKHandle = osThreadCreate(osThread(OBSERVE_TASK), NULL);

    osThreadDef(RC_INPUT_TASK, RC_Input_Task, APP_RC_INPUT_TASK_PRIORITY, 0, APP_RC_INPUT_TASK_STACK_BYTES);
    RC_INPUT_TASKHandle = osThreadCreate(osThread(RC_INPUT_TASK), NULL);

    osThreadDef(REMOTE_TASK, Remote_Task, APP_REMOTE_TASK_PRIORITY, 0, APP_REMOTE_TASK_STACK_BYTES);
    REMOTE_TASKHandle = osThreadCreate(osThread(REMOTE_TASK), NULL);
}

static void INS_Task(void const *argument)
{
    (void)argument;
    INS_task();
}

static void Chassis_Task(void const *argument)
{
    (void)argument;
    Chassis_task();
}

static void Motor_Control_Task(void const *argument)
{
    (void)argument;
    motor_control_task();
}

static void OBSERVE_Task(void const *argument)
{
    (void)argument;
    Observe_task();
}

static void Remote_Task(void const *argument)
{
    (void)argument;
    remote_task();
}

static void RC_Input_Task(void const *argument)
{
    (void)argument;
    rc_input_task();
}
