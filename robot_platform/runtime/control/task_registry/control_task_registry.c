#include "control_task_registry.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "../control_config/control_task_params.h"
#include "../controllers/chassis_control_task.h"
#include "../execution/motor_control_task.h"
#include "../state/ins_task.h"
#include "../state/observe_task.h"

osThreadId INS_TASKHandle;
osThreadId CHASSIS_TASKHandle;
osThreadId MOTOR_CONTROL_TASKHandle;
osThreadId OBSERVE_TASKHandle;

static void INS_Task(void const *argument);
static void Chassis_Task(void const *argument);
static void Motor_Control_Task(void const *argument);
static void OBSERVE_Task(void const *argument);

void platform_control_start_tasks(void)
{
    osThreadDef(INS_TASK, INS_Task, CONTROL_INS_TASK_PRIORITY, 0, CONTROL_INS_TASK_STACK_BYTES);
    INS_TASKHandle = osThreadCreate(osThread(INS_TASK), NULL);

    osThreadDef(CHASSISR_TASK, Chassis_Task, CONTROL_CHASSIS_TASK_PRIORITY, 0, CONTROL_CHASSIS_TASK_STACK_BYTES);
    CHASSIS_TASKHandle = osThreadCreate(osThread(CHASSISR_TASK), NULL);

    osThreadDef(CHASSISL_TASK, Motor_Control_Task, CONTROL_MOTOR_CONTROL_TASK_PRIORITY, 0, CONTROL_MOTOR_CONTROL_STACK_BYTES);
    MOTOR_CONTROL_TASKHandle = osThreadCreate(osThread(CHASSISL_TASK), NULL);

    osThreadDef(OBSERVE_TASK, OBSERVE_Task, CONTROL_OBSERVE_TASK_PRIORITY, 0, CONTROL_OBSERVE_TASK_STACK_BYTES);
    OBSERVE_TASKHandle = osThreadCreate(osThread(OBSERVE_TASK), NULL);
}

static void INS_Task(void const *argument)
{
    (void)argument;
    INS_task();
}

static void Chassis_Task(void const *argument)
{
    (void)argument;
    platform_chassis_control_task();
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
