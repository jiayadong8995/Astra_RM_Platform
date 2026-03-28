#include "FreeRTOS.h"
#include "main.h"
#include "cmsis_os.h"
#include "../app_config/app_params.h"
#include "task_registry.h"

osThreadId defaultTaskHandle;

static void StartDefaultTask(void const *argument);

void MX_FREERTOS_Init(void)
{
    osThreadDef(defaultTask, StartDefaultTask, APP_DEFAULT_TASK_PRIORITY, 0, APP_DEFAULT_TASK_STACK_BYTES);
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
