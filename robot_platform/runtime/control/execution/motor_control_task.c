#include "motor_control_task.h"

#include "cmsis_os.h"
#include "../../app/balance_chassis/app_config/app_params.h"
#include "../../app/balance_chassis/app_config/robot_def.h"
#include "actuator_gateway.h"
#include "actuator_topics.h"

static uint32_t systick;

void motor_control_task(void)
{
    platform_actuator_bus_t runtime_bus = {0};
    INS_Data_t ins_msg = {0};
    platform_actuator_command_t actuator_msg = {0};
    platform_device_feedback_t device_feedback = {0};
    Actuator_Feedback_t feedback_msg = {0};
    platform_actuator_bus_init(&runtime_bus);
    platform_actuator_bus_wait_ready(&runtime_bus, &ins_msg);
    platform_actuator_gateway_init();
    osDelay(APP_CHASSIS_STARTUP_DELAY_MS);

    while(1)
    {
        platform_actuator_bus_pull_cmd(&runtime_bus, &actuator_msg);
        if (platform_actuator_gateway_capture_feedback(&device_feedback) == PLATFORM_DEVICE_RESULT_OK)
        {
            platform_actuator_gateway_build_legacy_feedback(&device_feedback, &feedback_msg);
            platform_actuator_bus_publish_feedback(&runtime_bus, &device_feedback, &feedback_msg);
        }
        systick = osKernelSysTick();
        platform_actuator_gateway_dispatch_command(&actuator_msg, systick);
        osDelay(MOTOR_CONTROL_TASK_PERIOD_MS);
    }
}
