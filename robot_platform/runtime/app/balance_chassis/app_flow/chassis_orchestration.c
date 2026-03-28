#include "chassis_orchestration.h"

#include "../app_config/robot_def.h"
#include "chassis_control_support.h"

static float LQR_K_R[12] = LQR_K_MATRIX;

static void init_chassis_state(chassis_t *chassis, vmc_leg_t *right_leg, vmc_leg_t *left_leg);
static void init_chassis_pids(PidTypeDef *roll, PidTypeDef *tp, PidTypeDef *turn, PidTypeDef *leg_r, PidTypeDef *leg_l);
static void run_chassis_control_loop(Chassis_Runtime_State_t *state);

void chassis_runtime_state_init(Chassis_Runtime_State_t *state)
{
    init_chassis_state(&state->chassis, &state->right_leg, &state->left_leg);
    init_chassis_pids(&state->roll_pid, &state->tp_pid, &state->turn_pid, &state->leg_r_pid, &state->leg_l_pid);
}

void chassis_runtime_apply_bus_inputs(Chassis_Runtime_State_t *state, const Chassis_Bus_Input_t *inputs)
{
    state->ins.Pitch = inputs->ins.pitch;
    state->ins.Roll = inputs->ins.roll;
    state->ins.YawTotalAngle = inputs->ins.yaw_total;
    state->ins.Gyro[0] = inputs->ins.gyro[0];
    state->ins.Gyro[1] = inputs->ins.gyro[1];
    state->ins.Gyro[2] = inputs->ins.gyro[2];
    state->ins.MotionAccel_b[0] = inputs->ins.accel_b[0];
    state->ins.MotionAccel_b[1] = inputs->ins.accel_b[1];
    state->ins.MotionAccel_b[2] = inputs->ins.accel_b[2];
    state->ins.ins_flag = inputs->ins.ready;

    state->chassis.v_set = inputs->cmd.vx_cmd;
    state->chassis.turn_set = inputs->cmd.turn_cmd;
    state->chassis.leg_set = inputs->cmd.leg_set;
    state->chassis.start_flag = inputs->cmd.start_flag;
    state->chassis.jump_flag = inputs->cmd.jump_flag;
    state->chassis.recover_flag = inputs->cmd.recover_flag;
    state->chassis.v_filter = inputs->observe.v_filter;
    state->chassis.x_filter = inputs->observe.x_filter;
}

void chassis_runtime_step(Chassis_Runtime_State_t *state, const Actuator_Feedback_t *feedback)
{
    chassis_apply_feedback_snapshot(&state->chassis, &state->right_leg, &state->left_leg, &state->ins, feedback);
    run_chassis_control_loop(state);
}

void chassis_runtime_build_bus_outputs(const Chassis_Runtime_State_t *state, Chassis_Bus_Output_t *outputs)
{
    outputs->state.v_filter = state->chassis.v_filter;
    outputs->state.x_filter = state->chassis.x_filter;
    outputs->state.x_set = state->chassis.x_set;
    outputs->state.total_yaw = state->chassis.total_yaw;
    outputs->state.roll = state->chassis.roll;
    outputs->state.turn_set = state->chassis.turn_set;

    outputs->right_leg.joint_torque[0] = state->right_leg.torque_set[0];
    outputs->right_leg.joint_torque[1] = state->right_leg.torque_set[1];
    outputs->right_leg.wheel_torque = state->chassis.wheel_motor[1].torque_set;
    outputs->right_leg.wheel_current = state->chassis.wheel_motor[1].give_current;
    outputs->right_leg.leg_length = state->right_leg.L0;

    outputs->left_leg.joint_torque[0] = state->left_leg.torque_set[0];
    outputs->left_leg.joint_torque[1] = state->left_leg.torque_set[1];
    outputs->left_leg.wheel_torque = state->chassis.wheel_motor[0].torque_set;
    outputs->left_leg.wheel_current = state->chassis.wheel_motor[0].give_current;
    outputs->left_leg.leg_length = state->left_leg.L0;

    outputs->actuator_cmd.joint_torque[0] = state->right_leg.torque_set[0];
    outputs->actuator_cmd.joint_torque[1] = state->right_leg.torque_set[1];
    outputs->actuator_cmd.joint_torque[2] = state->left_leg.torque_set[0];
    outputs->actuator_cmd.joint_torque[3] = state->left_leg.torque_set[1];
    outputs->actuator_cmd.wheel_current[0] = state->chassis.wheel_motor[0].give_current;
    outputs->actuator_cmd.wheel_current[1] = state->chassis.wheel_motor[1].give_current;
    outputs->actuator_cmd.start_flag = state->chassis.start_flag;
}

static void init_chassis_state(chassis_t *chassis, vmc_leg_t *right_leg, vmc_leg_t *left_leg)
{
    for (int i = 0; i < 2; i++)
    {
        chassis->wheel_motor[i].chassis_x = 0.0f;
    }
    VMC_init(right_leg);
    VMC_init(left_leg);
}

static void init_chassis_pids(PidTypeDef *roll, PidTypeDef *tp, PidTypeDef *turn, PidTypeDef *leg_r, PidTypeDef *leg_l)
{
    static const float roll_pid[3] = {ROLL_PID_KP, ROLL_PID_KI, ROLL_PID_KD};
    static const float leg_r_pid[3] = {LEG_PID_KP, LEG_PID_KI, LEG_PID_KD};
    static const float leg_l_pid[3] = {LEG_PID_KP, LEG_PID_KI, LEG_PID_KD};
    static const float tp_pid[3] = {TP_PID_KP, TP_PID_KI, TP_PID_KD};
    static const float turn_pid[3] = {TURN_PID_KP, TURN_PID_KI, TURN_PID_KD};

    PID_init(roll, PID_POSITION, roll_pid, ROLL_PID_MAX_OUT, ROLL_PID_MAX_IOUT);
    PID_init(tp, PID_POSITION, tp_pid, TP_PID_MAX_OUT, TP_PID_MAX_IOUT);
    PID_init(turn, PID_POSITION, turn_pid, TURN_PID_MAX_OUT, TURN_PID_MAX_IOUT);
    PID_init(leg_r, PID_POSITION, leg_r_pid, LEG_PID_MAX_OUT, LEG_PID_MAX_IOUT);
    PID_init(leg_l, PID_POSITION, leg_l_pid, LEG_PID_MAX_OUT, LEG_PID_MAX_IOUT);
}

static void run_chassis_control_loop(Chassis_Runtime_State_t *state)
{
    VMC_calc_1_right(&state->right_leg, &state->ins, 2.0f / 1000.0f);
    VMC_calc_1_left(&state->left_leg, &state->ins, 2.0f / 1000.0f);
    chassis_compute_turn_and_leg_compensation(&state->chassis, &state->ins, &state->turn_pid, &state->roll_pid, &state->tp_pid);
    chassis_compute_lqr_outputs(&state->chassis, &state->right_leg, &state->left_leg, LQR_K_R, LQR_K_R);
    chassis_mix_wheel_torque(&state->chassis);

    state->right_leg.Tp = state->right_leg.Tp + state->chassis.leg_tp;
    state->left_leg.Tp = state->left_leg.Tp + state->chassis.leg_tp;
    state->chassis.roll_f0 = 0.0f;

    chassis_apply_jump_logic(&state->chassis, &state->right_leg, &state->left_leg, &state->leg_r_pid, &state->leg_l_pid);
    chassis_apply_ground_detection(&state->chassis, &state->right_leg, &state->left_leg, &state->ins);

    VMC_calc_2(&state->right_leg);
    VMC_calc_2(&state->left_leg);

    state->chassis.wheel_motor[0].give_current = state->chassis.wheel_motor[0].torque_set * M3508_TORQUE_TO_CURRENT;
    state->chassis.wheel_motor[1].give_current = state->chassis.wheel_motor[1].torque_set * M3508_TORQUE_TO_CURRENT;

    chassis_saturate_outputs(&state->chassis, &state->right_leg, &state->left_leg);
}
