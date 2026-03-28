#include "chassis_topics.h"

#include "cmsis_os.h"

static void update_ins_runtime(INS_t *ins, const INS_Data_t *msg);
static void apply_chassis_cmd(chassis_t *chassis, const Chassis_Cmd_t *cmd);
static void apply_observe_state(chassis_t *chassis, const Chassis_Observe_t *observe);

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

void chassis_runtime_bus_wait_ready(Chassis_Runtime_Bus_t *bus,
                                    chassis_t *chassis,
                                    INS_t *ins,
                                    INS_Data_t *ins_msg,
                                    Chassis_Cmd_t *cmd_msg,
                                    Chassis_Observe_t *observe_msg,
                                    Actuator_Feedback_t *feedback_msg)
{
    while (ins->ins_flag == 0 || feedback_msg->ready == 0U)
    {
        chassis_runtime_bus_pull_inputs(bus, chassis, ins, ins_msg, cmd_msg, observe_msg, feedback_msg);
        osDelay(1);
    }
}

void chassis_runtime_bus_pull_inputs(Chassis_Runtime_Bus_t *bus,
                                     chassis_t *chassis,
                                     INS_t *ins,
                                     INS_Data_t *ins_msg,
                                     Chassis_Cmd_t *cmd_msg,
                                     Chassis_Observe_t *observe_msg,
                                     Actuator_Feedback_t *feedback_msg)
{
    if (SubGetMessage(bus->ins_sub, ins_msg))
    {
        update_ins_runtime(ins, ins_msg);
    }
    if (SubGetMessage(bus->cmd_sub, cmd_msg))
    {
        apply_chassis_cmd(chassis, cmd_msg);
    }
    if (SubGetMessage(bus->observe_sub, observe_msg))
    {
        apply_observe_state(chassis, observe_msg);
    }
    SubGetMessage(bus->actuator_feedback_sub, feedback_msg);
}

void chassis_runtime_bus_publish_outputs(Chassis_Runtime_Bus_t *bus,
                                         const chassis_t *chassis,
                                         const vmc_leg_t *right_leg,
                                         const vmc_leg_t *left_leg)
{
    Chassis_State_t state_msg = {
        .v_filter = chassis->v_filter,
        .x_filter = chassis->x_filter,
        .x_set = chassis->x_set,
        .total_yaw = chassis->total_yaw,
        .roll = chassis->roll,
        .turn_set = chassis->turn_set,
    };
    Leg_Output_t right_msg = {
        .joint_torque = {right_leg->torque_set[0], right_leg->torque_set[1]},
        .wheel_torque = chassis->wheel_motor[1].torque_set,
        .wheel_current = chassis->wheel_motor[1].give_current,
        .leg_length = right_leg->L0,
    };
    Leg_Output_t left_msg = {
        .joint_torque = {left_leg->torque_set[0], left_leg->torque_set[1]},
        .wheel_torque = chassis->wheel_motor[0].torque_set,
        .wheel_current = chassis->wheel_motor[0].give_current,
        .leg_length = left_leg->L0,
    };
    Actuator_Cmd_t actuator_msg = {
        .joint_torque = {
            right_leg->torque_set[0],
            right_leg->torque_set[1],
            left_leg->torque_set[0],
            left_leg->torque_set[1],
        },
        .wheel_current = {
            chassis->wheel_motor[0].give_current,
            chassis->wheel_motor[1].give_current,
        },
        .start_flag = chassis->start_flag,
    };

    PubPushMessage(bus->chassis_state_pub, &state_msg);
    PubPushMessage(bus->leg_right_pub, &right_msg);
    PubPushMessage(bus->leg_left_pub, &left_msg);
    PubPushMessage(bus->actuator_cmd_pub, &actuator_msg);
}

static void update_ins_runtime(INS_t *ins, const INS_Data_t *msg)
{
    ins->Pitch = msg->pitch;
    ins->Roll = msg->roll;
    ins->YawTotalAngle = msg->yaw_total;
    ins->Gyro[0] = msg->gyro[0];
    ins->Gyro[1] = msg->gyro[1];
    ins->Gyro[2] = msg->gyro[2];
    ins->MotionAccel_b[0] = msg->accel_b[0];
    ins->MotionAccel_b[1] = msg->accel_b[1];
    ins->MotionAccel_b[2] = msg->accel_b[2];
    ins->ins_flag = msg->ready;
}

static void apply_chassis_cmd(chassis_t *chassis, const Chassis_Cmd_t *cmd)
{
    chassis->v_set = cmd->vx_cmd;
    chassis->turn_set = cmd->turn_cmd;
    chassis->leg_set = cmd->leg_set;
    chassis->start_flag = cmd->start_flag;
    chassis->jump_flag = cmd->jump_flag;
    chassis->recover_flag = cmd->recover_flag;
}

static void apply_observe_state(chassis_t *chassis, const Chassis_Observe_t *observe)
{
    chassis->v_filter = observe->v_filter;
    chassis->x_filter = observe->x_filter;
}
