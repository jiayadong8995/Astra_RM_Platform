#include "task_registry.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "../app_config/app_params.h"
#include "../../../control/task_registry/control_task_registry.h"
#include "remote_task.h"

osThreadId REMOTE_TASKHandle;

static void Remote_Task(void const *argument);

void balance_chassis_start_tasks(void)
{
    platform_control_start_tasks();

    osThreadDef(REMOTE_TASK, Remote_Task, APP_REMOTE_TASK_PRIORITY, 0, APP_REMOTE_TASK_STACK_BYTES);
    REMOTE_TASKHandle = osThreadCreate(osThread(REMOTE_TASK), NULL);
}

static void Remote_Task(void const *argument)
{
    (void)argument;
    remote_task();
}
