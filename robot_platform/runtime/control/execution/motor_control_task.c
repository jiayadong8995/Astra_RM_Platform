#include "motor_control_task.h"

#include "cmsis_os.h"
#include "../control_config/control_task_params.h"
#include "../readiness.h"
#include "../topics.h"
#include "actuator_gateway.h"

void motor_control_task_init(platform_motor_control_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    runtime->ins_sub = SubRegister(TOPIC_INS_DATA, sizeof(platform_ins_state_message_t));
    runtime->actuator_cmd_sub = SubRegister(TOPIC_ACTUATOR_CMD, sizeof(platform_actuator_command_t));
    runtime->device_feedback_pub = PubRegister(TOPIC_DEVICE_FEEDBACK, sizeof(platform_device_feedback_t));
}

void motor_control_task_prepare(platform_motor_control_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_readiness_wait_ins(runtime->ins_sub, &runtime->ins_msg);
    platform_actuator_gateway_init();
    osDelay(APP_CHASSIS_STARTUP_DELAY_MS);
}

void motor_control_task_step(platform_motor_control_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    SubGetMessage(runtime->actuator_cmd_sub, &runtime->actuator_msg);
    if (platform_actuator_gateway_capture_feedback(&runtime->device_feedback) == PLATFORM_DEVICE_RESULT_OK)
    {
        PubPushMessage(runtime->device_feedback_pub, (void *)&runtime->device_feedback);
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
