#include "motor_control_task.h"

#include "cmsis_os.h"
#include "../control_config/control_task_params.h"
#include "actuator_gateway.h"
#include "actuator_topics.h"

void motor_control_task_init(platform_motor_control_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_actuator_bus_init(&runtime->runtime_bus);
}

void motor_control_task_prepare(platform_motor_control_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_actuator_bus_wait_ready(&runtime->runtime_bus, &runtime->ins_msg);
    platform_actuator_gateway_init();
    osDelay(APP_CHASSIS_STARTUP_DELAY_MS);
}

void motor_control_task_step(platform_motor_control_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_actuator_bus_pull_cmd(&runtime->runtime_bus, &runtime->actuator_msg);
    if (platform_actuator_gateway_capture_feedback(&runtime->device_feedback) == PLATFORM_DEVICE_RESULT_OK)
    {
        platform_actuator_bus_publish_feedback(&runtime->runtime_bus, &runtime->device_feedback);
    }
    runtime->systick = osKernelSysTick();
    platform_actuator_gateway_dispatch_command(&runtime->actuator_msg, runtime->systick);
}

void motor_control_task(void)
{
    platform_motor_control_task_runtime_t runtime = {0};

    motor_control_task_init(&runtime);
    motor_control_task_prepare(&runtime);

    while(1)
    {
        motor_control_task_step(&runtime);
        osDelay(MOTOR_CONTROL_TASK_PERIOD_MS);
    }
}
