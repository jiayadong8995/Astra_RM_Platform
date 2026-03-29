#include "chassis_topics.h"

#include "cmsis_os.h"

void chassis_runtime_bus_init(Chassis_Runtime_Bus_t *bus)
{
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->cmd_sub = SubRegister("chassis_cmd", sizeof(Chassis_Cmd_t));
    bus->observe_sub = SubRegister("chassis_observe", sizeof(Chassis_Observe_t));
    bus->actuator_feedback_sub = SubRegister("actuator_feedback", sizeof(Actuator_Feedback_t));

    bus->chassis_state_pub = PubRegister("chassis_state", sizeof(Chassis_State_t));
    bus->leg_right_pub = PubRegister("leg_right", sizeof(Leg_Output_t));
    bus->leg_left_pub = PubRegister("leg_left", sizeof(Leg_Output_t));
    bus->actuator_cmd_pub = PubRegister("actuator_cmd", sizeof(Actuator_Cmd_t));
}

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus, platform_balance_controller_input_t *inputs)
{
    while (inputs->ins.ready == 0U || inputs->feedback.ready == 0U)
    {
        chassis_runtime_bus_pull_inputs(bus, inputs);
        osDelay(1);
    }
}

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus, platform_balance_controller_input_t *inputs)
{
    SubGetMessage(bus->ins_sub, &inputs->ins);
    SubGetMessage(bus->cmd_sub, &inputs->cmd);
    SubGetMessage(bus->observe_sub, &inputs->observe);
    SubGetMessage(bus->actuator_feedback_sub, &inputs->feedback);
}

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus, const platform_balance_controller_output_t *outputs)
{
    PubPushMessage(bus->chassis_state_pub, (void *)&outputs->state);
    PubPushMessage(bus->leg_right_pub, (void *)&outputs->right_leg);
    PubPushMessage(bus->leg_left_pub, (void *)&outputs->left_leg);
    PubPushMessage(bus->actuator_cmd_pub, (void *)&outputs->actuator_cmd);
}
