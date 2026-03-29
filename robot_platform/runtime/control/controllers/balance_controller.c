#include "balance_controller.h"

#include "../../app/balance_chassis/app_config/robot_def.h"
#include "user_lib.h"

static float LQR_K_R[12] = LQR_K_MATRIX;

static void init_chassis_state(chassis_t *chassis, vmc_leg_t *right_leg, vmc_leg_t *left_leg);
static void init_chassis_pids(PidTypeDef *roll, PidTypeDef *tp, PidTypeDef *turn, PidTypeDef *leg_r, PidTypeDef *leg_l);
static void run_balance_control(platform_balance_controller_t *state);
static void update_leg_feedback(vmc_leg_t *vmc_r, vmc_leg_t *vmc_l, const platform_device_feedback_t *feedback);
static void update_wheel_feedback(chassis_t *chassis, const platform_device_feedback_t *feedback);
static void update_attitude_feedback(chassis_t *chassis, const vmc_leg_t *vmc_r, const vmc_leg_t *vmc_l, const INS_t *ins);
static void limit_int(int16_t *in, int16_t min, int16_t max);
static void saturate_wheel_outputs(chassis_t *chassis);
static void saturate_leg_outputs(vmc_leg_t *vmcr, vmc_leg_t *vmcl);
static void compute_turn_and_leg_compensation(chassis_t *chassis,
                                              const INS_t *ins,
                                              const PidTypeDef *turn_pid,
                                              const PidTypeDef *roll_pid,
                                              PidTypeDef *tp_pid);
static void compute_lqr_outputs(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, const float *LQR_KR, const float *LQR_KL);
static void mix_wheel_torque(chassis_t *chassis);
static void apply_jump_logic(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, PidTypeDef *legr, PidTypeDef *legl);
static void apply_ground_detection(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, INS_t *ins);

void platform_balance_controller_init(platform_balance_controller_t *state)
{
    init_chassis_state(&state->chassis, &state->right_leg, &state->left_leg);
    init_chassis_pids(&state->roll_pid, &state->tp_pid, &state->turn_pid, &state->leg_r_pid, &state->leg_l_pid);
}

void platform_balance_controller_apply_inputs(platform_balance_controller_t *state,
                                              const platform_balance_controller_input_t *inputs)
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

void platform_balance_controller_step(platform_balance_controller_t *state, const platform_device_feedback_t *feedback)
{
    update_leg_feedback(&state->right_leg, &state->left_leg, feedback);
    update_wheel_feedback(&state->chassis, feedback);
    update_attitude_feedback(&state->chassis, &state->right_leg, &state->left_leg, &state->ins);

    if (state->ins.Pitch < PITCH_FALL_THRESHOLD && state->ins.Pitch > -PITCH_FALL_THRESHOLD)
    {
        state->chassis.recover_flag = 0;
    }

    run_balance_control(state);
}

void platform_balance_controller_build_outputs(const platform_balance_controller_t *state,
                                               platform_balance_controller_output_t *outputs)
{
    outputs->robot_state.body.roll = state->ins.Roll;
    outputs->robot_state.body.pitch = state->ins.Pitch;
    outputs->robot_state.body.yaw = state->ins.Yaw;
    outputs->robot_state.body.gyro[0] = state->ins.Gyro[0];
    outputs->robot_state.body.gyro[1] = state->ins.Gyro[1];
    outputs->robot_state.body.gyro[2] = state->ins.Gyro[2];
    outputs->robot_state.body.accel[0] = state->ins.MotionAccel_b[0];
    outputs->robot_state.body.accel[1] = state->ins.MotionAccel_b[1];
    outputs->robot_state.body.accel[2] = state->ins.MotionAccel_b[2];
    outputs->robot_state.body.orientation_valid = (state->ins.ins_flag != 0U);

    outputs->robot_state.chassis.x = state->chassis.x_filter;
    outputs->robot_state.chassis.v = state->chassis.v_filter;
    outputs->robot_state.chassis.vx = state->chassis.v_filter;
    outputs->robot_state.chassis.vy = 0.0f;
    outputs->robot_state.chassis.yaw_total = state->chassis.total_yaw;
    outputs->robot_state.chassis.turn_rate = state->chassis.turn_T;
    outputs->robot_state.chassis.state_valid = (state->ins.ins_flag != 0U);

    outputs->robot_state.legs.right.length = state->right_leg.L0;
    outputs->robot_state.legs.right.leg_angle = state->right_leg.theta;
    outputs->robot_state.legs.right.joint_pos[0] = state->right_leg.phi1;
    outputs->robot_state.legs.right.joint_pos[1] = state->right_leg.phi4;
    outputs->robot_state.legs.right.joint_vel[0] = 0.0f;
    outputs->robot_state.legs.right.joint_vel[1] = 0.0f;
    outputs->robot_state.legs.right.joint_torque_est[0] = state->right_leg.torque_set[0];
    outputs->robot_state.legs.right.joint_torque_est[1] = state->right_leg.torque_set[1];

    outputs->robot_state.legs.left.length = state->left_leg.L0;
    outputs->robot_state.legs.left.leg_angle = state->left_leg.theta;
    outputs->robot_state.legs.left.joint_pos[0] = state->left_leg.phi1;
    outputs->robot_state.legs.left.joint_pos[1] = state->left_leg.phi4;
    outputs->robot_state.legs.left.joint_vel[0] = 0.0f;
    outputs->robot_state.legs.left.joint_vel[1] = 0.0f;
    outputs->robot_state.legs.left.joint_torque_est[0] = state->left_leg.torque_set[0];
    outputs->robot_state.legs.left.joint_torque_est[1] = state->left_leg.torque_set[1];

    outputs->robot_state.wheels.left.speed = state->chassis.wheel_motor[0].speed;
    outputs->robot_state.wheels.left.position = state->chassis.wheel_motor[0].chassis_x;
    outputs->robot_state.wheels.left.torque_est = state->chassis.wheel_motor[0].torque;
    outputs->robot_state.wheels.left.online = true;
    outputs->robot_state.wheels.right.speed = state->chassis.wheel_motor[1].speed;
    outputs->robot_state.wheels.right.position = state->chassis.wheel_motor[1].chassis_x;
    outputs->robot_state.wheels.right.torque_est = state->chassis.wheel_motor[1].torque;
    outputs->robot_state.wheels.right.online = true;

    outputs->robot_state.contact.grounded = (state->chassis.text_jump_true != 0U);
    outputs->robot_state.contact.left_support = (state->left_leg.FN > FN_GROUND_THRESHOLD);
    outputs->robot_state.contact.right_support = (state->right_leg.FN > FN_GROUND_THRESHOLD);
    outputs->robot_state.contact.land_confidence = outputs->robot_state.contact.grounded ? 1.0f : 0.0f;

    outputs->robot_state.health.imu_ok = (state->ins.ins_flag != 0U);
    outputs->robot_state.health.remote_ok = true;
    outputs->robot_state.health.actuator_ok = true;
    outputs->robot_state.health.state_valid = outputs->robot_state.body.orientation_valid;
    outputs->robot_state.health.degraded_mode = (state->chassis.recover_flag != 0U);

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

    outputs->actuator_command.start = (state->chassis.start_flag != 0U);
    outputs->actuator_command.control_enable = (state->chassis.start_flag != 0U);
    outputs->actuator_command.actuator_enable = (state->chassis.start_flag != 0U);

    outputs->actuator_command.motors.left_leg_joint[0].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    outputs->actuator_command.motors.left_leg_joint[0].torque_target = state->left_leg.torque_set[0];
    outputs->actuator_command.motors.left_leg_joint[0].valid = (state->chassis.start_flag != 0U);
    outputs->actuator_command.motors.left_leg_joint[1].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    outputs->actuator_command.motors.left_leg_joint[1].torque_target = state->left_leg.torque_set[1];
    outputs->actuator_command.motors.left_leg_joint[1].valid = (state->chassis.start_flag != 0U);
    outputs->actuator_command.motors.right_leg_joint[0].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    outputs->actuator_command.motors.right_leg_joint[0].torque_target = state->right_leg.torque_set[0];
    outputs->actuator_command.motors.right_leg_joint[0].valid = (state->chassis.start_flag != 0U);
    outputs->actuator_command.motors.right_leg_joint[1].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    outputs->actuator_command.motors.right_leg_joint[1].torque_target = state->right_leg.torque_set[1];
    outputs->actuator_command.motors.right_leg_joint[1].valid = (state->chassis.start_flag != 0U);
    outputs->actuator_command.motors.left_wheel.control_mode = PLATFORM_MOTOR_CONTROL_CURRENT;
    outputs->actuator_command.motors.left_wheel.current_target = (float)state->chassis.wheel_motor[0].give_current;
    outputs->actuator_command.motors.left_wheel.valid = (state->chassis.start_flag != 0U);
    outputs->actuator_command.motors.right_wheel.control_mode = PLATFORM_MOTOR_CONTROL_CURRENT;
    outputs->actuator_command.motors.right_wheel.current_target = (float)state->chassis.wheel_motor[1].give_current;
    outputs->actuator_command.motors.right_wheel.valid = (state->chassis.start_flag != 0U);
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

static void run_balance_control(platform_balance_controller_t *state)
{
    VMC_calc_1_right(&state->right_leg, &state->ins, 2.0f / 1000.0f);
    VMC_calc_1_left(&state->left_leg, &state->ins, 2.0f / 1000.0f);
    compute_turn_and_leg_compensation(&state->chassis, &state->ins, &state->turn_pid, &state->roll_pid, &state->tp_pid);
    compute_lqr_outputs(&state->chassis, &state->right_leg, &state->left_leg, LQR_K_R, LQR_K_R);
    mix_wheel_torque(&state->chassis);

    state->right_leg.Tp = state->right_leg.Tp + state->chassis.leg_tp;
    state->left_leg.Tp = state->left_leg.Tp + state->chassis.leg_tp;
    state->chassis.roll_f0 = 0.0f;

    apply_jump_logic(&state->chassis, &state->right_leg, &state->left_leg, &state->leg_r_pid, &state->leg_l_pid);
    apply_ground_detection(&state->chassis, &state->right_leg, &state->left_leg, &state->ins);

    VMC_calc_2(&state->right_leg);
    VMC_calc_2(&state->left_leg);

    state->chassis.wheel_motor[0].give_current = state->chassis.wheel_motor[0].torque_set * M3508_TORQUE_TO_CURRENT;
    state->chassis.wheel_motor[1].give_current = state->chassis.wheel_motor[1].torque_set * M3508_TORQUE_TO_CURRENT;

    saturate_wheel_outputs(&state->chassis);
    saturate_leg_outputs(&state->right_leg, &state->left_leg);
}

static void update_leg_feedback(vmc_leg_t *vmc_r, vmc_leg_t *vmc_l, const platform_device_feedback_t *feedback)
{
    vmc_r->phi1 = pi / 2.0f + feedback->actuator_feedback.joints[0].position + JOINT0_OFFSET;
    vmc_r->phi4 = pi / 2.0f + feedback->actuator_feedback.joints[1].position + JOINT1_OFFSET;
    vmc_l->phi1 = pi / 2.0f + feedback->actuator_feedback.joints[2].position + JOINT2_OFFSET;
    vmc_l->phi4 = pi / 2.0f + feedback->actuator_feedback.joints[3].position + JOINT3_OFFSET;
}

static void update_wheel_feedback(chassis_t *chassis, const platform_device_feedback_t *feedback)
{
    chassis->wheel_motor[0].chassis_x = feedback->actuator_feedback.wheels[0].position;
    chassis->wheel_motor[1].chassis_x = feedback->actuator_feedback.wheels[1].position;
    chassis->wheel_motor[0].speed = feedback->actuator_feedback.wheels[0].velocity;
    chassis->wheel_motor[1].speed = feedback->actuator_feedback.wheels[1].velocity;
    chassis->wheel_motor[0].w_speed = feedback->actuator_feedback.wheels[0].velocity / WHEEL_RADIUS;
    chassis->wheel_motor[1].w_speed = feedback->actuator_feedback.wheels[1].velocity / WHEEL_RADIUS;
}

static void update_attitude_feedback(chassis_t *chassis, const vmc_leg_t *vmc_r, const vmc_leg_t *vmc_l, const INS_t *ins)
{
    chassis->myPithGyroL = -ins->Gyro[0];
    chassis->myPithL = -ins->Pitch;
    chassis->myPithR = ins->Pitch;
    chassis->myPithGyroR = ins->Gyro[0];
    chassis->total_yaw = ins->YawTotalAngle;
    chassis->roll = ins->Roll;
    chassis->theta_err = 0.0f - (vmc_r->theta + vmc_l->theta);
}

static void compute_turn_and_leg_compensation(chassis_t *chassis,
                                              const INS_t *ins,
                                              const PidTypeDef *turn_pid,
                                              const PidTypeDef *roll_pid,
                                              PidTypeDef *tp_pid)
{
    chassis->turn_T = turn_pid->Kp * (chassis->turn_set - chassis->total_yaw) - turn_pid->Kd * ins->Gyro[2];
    chassis->roll_f0 = roll_pid->Kp * (chassis->roll_set - chassis->roll) - roll_pid->Kd * ins->Gyro[1];
    chassis->leg_tp = PID_Calc(tp_pid, chassis->theta_err, 0.0f);
}

static void compute_lqr_outputs(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, const float *LQR_KR, const float *LQR_KL)
{
    chassis->wheel_motor[1].torque_set = (LQR_KR[0] * (vmcr->theta) + LQR_KR[1] * (vmcr->d_theta)
                                       + LQR_KR[2] * (chassis->x_filter - chassis->x_set) + LQR_KR[3] * (chassis->v_filter - 0)
                                       + LQR_KR[4] * (chassis->myPithR - 0.0f) + LQR_KR[5] * (chassis->myPithGyroR - 0.0f));
    vmcr->Tp = (LQR_KR[6] * (vmcr->theta) + LQR_KR[7] * (vmcr->d_theta)
             + LQR_KR[8] * (chassis->x_filter - chassis->x_set) + LQR_KR[9] * (chassis->v_filter - 0)
             + LQR_KR[10] * (chassis->myPithR) + LQR_KR[11] * (chassis->myPithGyroR));

    chassis->wheel_motor[0].torque_set = (LQR_KL[0] * (vmcl->theta) + LQR_KL[1] * (vmcl->d_theta)
                                       + LQR_KL[2] * (chassis->x_set - chassis->x_filter) + LQR_KL[3] * (0 - chassis->v_filter)
                                       + LQR_KL[4] * (chassis->myPithL - 0.0f) + LQR_KL[5] * (chassis->myPithGyroL - 0.0f));
    vmcl->Tp = (LQR_KL[6] * (vmcl->theta) + LQR_KL[7] * (vmcl->d_theta)
             + LQR_KL[8] * (chassis->x_set - chassis->x_filter) + LQR_KL[9] * (0 - chassis->v_filter)
             + LQR_KL[10] * (chassis->myPithL) + LQR_KL[11] * (chassis->myPithGyroL));
}

static void mix_wheel_torque(chassis_t *chassis)
{
    for (int i = 0; i < 2; i++)
    {
        chassis->wheel_motor[i].torque_set = WHEEL_TORQUE_RATIO * chassis->wheel_motor[i].torque_set
                                           + TURN_TORQUE_RATIO * chassis->turn_T;
    }
}

static void apply_jump_logic(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, PidTypeDef *legr, PidTypeDef *legl)
{
    if (chassis->start_flag != 1)
    {
        return;
    }

    if (chassis->jump_flag == 1)
    {
        if (chassis->jump_status == 0)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, 0.18f);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, 0.18f);
            if (vmcr->L0 < 0.185f && vmcl->L0 < 0.185f) { chassis->jump_time++; }
            if (chassis->jump_time > 10) { chassis->jump_time = 0; chassis->jump_status = 1; }
        }
        else if (chassis->jump_status == 1)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, 0.3f);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, 0.3f);
            if (vmcr->L0 > 0.22f && vmcl->L0 > 0.22f) { chassis->jump_time++; }
            if (chassis->jump_time > 10) { chassis->jump_time = 0; chassis->jump_status = 2; }
        }
        else if (chassis->jump_status == 2)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, 0.18f);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, 0.18f);
            if (vmcr->L0 < 0.250f && vmcl->L0 < 0.250f) { chassis->jump_time++; }
            if (chassis->jump_time > 10) { chassis->jump_time = 0; chassis->jump_status = 3; }
        }
        else if (chassis->jump_status == 3)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, chassis->leg_set);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, chassis->leg_set);
        }
    }
    else
    {
        vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, chassis->leg_set) + chassis->roll_f0;
        vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, chassis->leg_set) - chassis->roll_f0;
        chassis->jump_time = 0;
        chassis->jump_status = 0;
    }
}

static void apply_ground_detection(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, INS_t *ins)
{
    uint8_t right_flag = ground_detectionR(vmcr, ins);
    uint8_t left_flag = ground_detectionL(vmcl, ins);

    if (chassis->recover_flag == 0)
    {
        if (right_flag == 1 && left_flag == 1)
        {
            chassis->wheel_motor[0].torque_set = 0.0f;
            chassis->wheel_motor[1].torque_set = 0.0f;
            chassis->x_filter = 0.0f;
            chassis->x_set = chassis->x_filter;
            chassis->turn_set = chassis->total_yaw;
            vmcr->Tp = vmcr->Tp + chassis->leg_tp;
            chassis->text_jump_true = 1;
        }
        else
        {
            chassis->text_jump_true = 0;
        }
    }
    else if (chassis->recover_flag == 1)
    {
        vmcr->Tp = 0.0f;
        vmcl->Tp = 0.0f;
    }
}

static void limit_int(int16_t *in, int16_t min, int16_t max)
{
    if (*in < min) { *in = min; }
    else if (*in > max) { *in = max; }
}

static void saturate_wheel_outputs(chassis_t *chassis)
{
    for (int i = 0; i < 2; i++)
    {
        mySaturate(&chassis->wheel_motor[i].torque_set, -WHEEL_TORQUE_MAX, WHEEL_TORQUE_MAX);
        limit_int(&chassis->wheel_motor[i].give_current, -8000, 8000);
    }
}

static void saturate_leg_outputs(vmc_leg_t *vmcr, vmc_leg_t *vmcl)
{
    mySaturate(&vmcr->F0, -100.0f, 100.0f);
    mySaturate(&vmcr->torque_set[1], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&vmcr->torque_set[0], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&vmcl->F0, -100.0f, 100.0f);
    mySaturate(&vmcl->torque_set[1], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
    mySaturate(&vmcl->torque_set[0], -JOINT_TORQUE_MAX, JOINT_TORQUE_MAX);
}
