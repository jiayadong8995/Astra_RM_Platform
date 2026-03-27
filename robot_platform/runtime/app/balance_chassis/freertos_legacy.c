#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

osThreadId defaultTaskHandle;
osThreadId INS_TASKHandle;
osThreadId CHASSIS_TASKHandle;
osThreadId MOTOR_CONTROL_TASKHandle;
osThreadId OBSERVE_TASKHandle;
osThreadId REMOTE_TASKHandle;

static void StartDefaultTask(void const *argument);
static void INS_Task(void const *argument);
static void Chassis_Task(void const *argument);
static void Motor_Control_Task(void const *argument);
static void OBSERVE_Task(void const *argument);
static void Remote_Task(void const *argument);

void INS_task(void);
void Chassis_task(void);
void motor_control_task(void);
void Observe_task(void);
void remote_task(void);

void MX_FREERTOS_Init(void)
{
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    osThreadDef(INS_TASK, INS_Task, osPriorityRealtime, 0, 512);
    INS_TASKHandle = osThreadCreate(osThread(INS_TASK), NULL);

    osThreadDef(CHASSISR_TASK, Chassis_Task, osPriorityAboveNormal, 0, 512);
    CHASSIS_TASKHandle = osThreadCreate(osThread(CHASSISR_TASK), NULL);

    osThreadDef(CHASSISL_TASK, Motor_Control_Task, osPriorityAboveNormal, 0, 512);
    MOTOR_CONTROL_TASKHandle = osThreadCreate(osThread(CHASSISL_TASK), NULL);

    osThreadDef(OBSERVE_TASK, OBSERVE_Task, osPriorityHigh, 0, 512);
    OBSERVE_TASKHandle = osThreadCreate(osThread(OBSERVE_TASK), NULL);

    osThreadDef(REMOTE_TASK, Remote_Task, osPriorityAboveNormal, 0, 512);
    REMOTE_TASKHandle = osThreadCreate(osThread(REMOTE_TASK), NULL);
}

static void StartDefaultTask(void const *argument)
{
    (void)argument;
    for (;;)
    {
        osDelay(1);
    }
}

static void INS_Task(void const *argument)
{
    (void)argument;
    for (;;)
    {
        INS_task();
    }
}

static void Chassis_Task(void const *argument)
{
    (void)argument;
    for (;;)
    {
        Chassis_task();
    }
}

static void Motor_Control_Task(void const *argument)
{
    (void)argument;
    for (;;)
    {
        motor_control_task();
    }
}

static void OBSERVE_Task(void const *argument)
{
    (void)argument;
    for (;;)
    {
        Observe_task();
    }
}

static void Remote_Task(void const *argument)
{
    (void)argument;
    for (;;)
    {
        remote_task();
    }
}
