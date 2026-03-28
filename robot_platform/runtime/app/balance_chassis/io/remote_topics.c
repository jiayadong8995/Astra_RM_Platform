#include "remote_topics.h"

void remote_runtime_bus_init(Remote_Runtime_Bus_t *bus)
{
    bus->rc_sub = SubRegister("rc_data", sizeof(RC_Data_t));
    bus->ins_sub = SubRegister("ins_data", sizeof(INS_Data_t));
    bus->chassis_state_sub = SubRegister("chassis_state", sizeof(Chassis_State_t));
    bus->leg_right_sub = SubRegister("leg_right", sizeof(Leg_Output_t));
    bus->leg_left_sub = SubRegister("leg_left", sizeof(Leg_Output_t));
    bus->cmd_pub = PubRegister("chassis_cmd", sizeof(Chassis_Cmd_t));
}

void remote_runtime_bus_pull_inputs(Remote_Runtime_Bus_t *bus,
                                    RC_Data_t *rc_msg,
                                    INS_Data_t *ins_msg,
                                    Chassis_State_t *state_msg,
                                    Leg_Output_t *right_msg,
                                    Leg_Output_t *left_msg)
{
    SubGetMessage(bus->rc_sub, rc_msg);
    SubGetMessage(bus->ins_sub, ins_msg);
    SubGetMessage(bus->chassis_state_sub, state_msg);
    SubGetMessage(bus->leg_right_sub, right_msg);
    SubGetMessage(bus->leg_left_sub, left_msg);
}

void remote_runtime_bus_publish_cmd(Remote_Runtime_Bus_t *bus, const Chassis_Cmd_t *cmd)
{
    PubPushMessage(bus->cmd_pub, (void *)cmd);
}
