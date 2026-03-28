#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "rc_input_bridge.h"

osThreadId defaultTaskHandle;
osThreadId INS_TASKHandle;
osThreadId CHASSIS_TASKHandle;
osThreadId MOTOR_CONTROL_TASKHandle;
osThreadId OBSERVE_TASKHandle;
osThreadId REMOTE_TASKHandle;
osThreadId RC_INPUT_TASKHandle;

#define DEFAULT_TASK_STACK_BYTES 512
#define INS_TASK_STACK_BYTES 2048
#define CHASSIS_TASK_STACK_BYTES 4096
#define MOTOR_CONTROL_TASK_STACK_BYTES 2048
#define OBSERVE_TASK_STACK_BYTES 2048
#define RC_INPUT_TASK_STACK_BYTES 1024
#define REMOTE_TASK_STACK_BYTES 2048

static void StartDefaultTask(void const *argument);
static void INS_Task(void const *argument);
static void Chassis_Task(void const *argument);
static void Motor_Control_Task(void const *argument);
static void OBSERVE_Task(void const *argument);
static void Remote_Task(void const *argument);
static void RC_Input_Task(void const *argument);

void INS_task(void);
void Chassis_task(void);
void motor_control_task(void);
void Observe_task(void);
void remote_task(void);
void MX_FREERTOS_Init(void)
{
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, DEFAULT_TASK_STACK_BYTES);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    osThreadDef(INS_TASK, INS_Task, osPriorityRealtime, 0, INS_TASK_STACK_BYTES);
    INS_TASKHandle = osThreadCreate(osThread(INS_TASK), NULL);

    osThreadDef(CHASSISR_TASK, Chassis_Task, osPriorityAboveNormal, 0, CHASSIS_TASK_STACK_BYTES);
    CHASSIS_TASKHandle = osThreadCreate(osThread(CHASSISR_TASK), NULL);

    osThreadDef(CHASSISL_TASK, Motor_Control_Task, osPriorityAboveNormal, 0, MOTOR_CONTROL_TASK_STACK_BYTES);
    MOTOR_CONTROL_TASKHandle = osThreadCreate(osThread(CHASSISL_TASK), NULL);

    osThreadDef(OBSERVE_TASK, OBSERVE_Task, osPriorityHigh, 0, OBSERVE_TASK_STACK_BYTES);
    OBSERVE_TASKHandle = osThreadCreate(osThread(OBSERVE_TASK), NULL);

    osThreadDef(RC_INPUT_TASK, RC_Input_Task, osPriorityAboveNormal, 0, RC_INPUT_TASK_STACK_BYTES);
    RC_INPUT_TASKHandle = osThreadCreate(osThread(RC_INPUT_TASK), NULL);

    osThreadDef(REMOTE_TASK, Remote_Task, osPriorityAboveNormal, 0, REMOTE_TASK_STACK_BYTES);
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

static void RC_Input_Task(void const *argument)
{
    (void)argument;
    for (;;)
    {
        rc_input_task();
    }
}
