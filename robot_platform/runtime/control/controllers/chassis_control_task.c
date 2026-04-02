#include "chassis_control_task.h"

#include "cmsis_os.h"

#include "app_params.h"
#include "../readiness.h"
#include "../topics.h"
#include "../../app/balance_chassis/app_io/chassis_topics.h"

void chassis_task_init(platform_chassis_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    runtime->ins_sub = SubRegister(TOPIC_INS_DATA, sizeof(platform_ins_state_message_t));
    runtime->cmd_sub = SubRegister(TOPIC_ROBOT_INTENT, sizeof(platform_robot_intent_t));
    runtime->observe_sub = SubRegister(TOPIC_CHASSIS_OBSERVE, sizeof(platform_chassis_observe_message_t));
    runtime->device_feedback_sub = SubRegister(TOPIC_DEVICE_FEEDBACK, sizeof(platform_device_feedback_t));
    runtime->robot_state_pub = PubRegister(TOPIC_ROBOT_STATE, sizeof(platform_robot_state_t));
    runtime->actuator_command_pub = PubRegister(TOPIC_ACTUATOR_CMD, sizeof(platform_actuator_command_t));
    platform_balance_controller_init(&runtime->runtime_state);
}

void chassis_task_prepare(platform_chassis_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    platform_readiness_wait_ins_and_feedback(runtime->ins_sub, runtime->device_feedback_sub,
                                             &runtime->inputs.ins, &runtime->inputs.feedback);
    platform_balance_controller_apply_inputs(&runtime->runtime_state, &runtime->inputs);
    osDelay(APP_CHASSIS_STARTUP_DELAY_MS);
}

void chassis_task_step(platform_chassis_task_runtime_t *runtime)
{
    if (runtime == NULL)
    {
        return;
    }

    SubGetMessage(runtime->ins_sub, &runtime->inputs.ins);
    SubGetMessage(runtime->cmd_sub, &runtime->inputs.intent);
    SubGetMessage(runtime->observe_sub, &runtime->inputs.observe);
    SubGetMessage(runtime->device_feedback_sub, &runtime->inputs.feedback);
    platform_balance_controller_apply_inputs(&runtime->runtime_state, &runtime->inputs);
    platform_balance_controller_step(&runtime->runtime_state, &runtime->inputs.feedback);
    platform_balance_controller_build_outputs(&runtime->runtime_state, &runtime->outputs);
    PubPushMessage(runtime->robot_state_pub, (void *)&runtime->outputs.robot_state);
    PubPushMessage(runtime->actuator_command_pub, (void *)&runtime->outputs.actuator_command);
    chassis_observation_on_publish(&runtime->outputs.actuator_command);
}

void platform_chassis_control_task(void)
{
    platform_chassis_task_runtime_t runtime = {0};

    chassis_task_init(&runtime);
    chassis_task_prepare(&runtime);

    while (1)
    {
        chassis_task_step(&runtime);
        osDelay(CHASSIS_TASK_PERIOD_MS);
    }
}
