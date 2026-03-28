#include "FreeRTOS.h"
#include "main.h"
#include "cmsis_os.h"
#include "task_registry.h"

osThreadId defaultTaskHandle;

#define DEFAULT_TASK_STACK_BYTES 512

static void StartDefaultTask(void const *argument);

void MX_FREERTOS_Init(void)
{
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, DEFAULT_TASK_STACK_BYTES);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
    balance_chassis_start_tasks();
}

static void StartDefaultTask(void const *argument)
{
    (void)argument;
    for (;;)
    {
        osDelay(1);
    }
}
