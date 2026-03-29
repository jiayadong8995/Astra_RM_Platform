#include "balance_controller.h"

#include "../internal/balance_params.h"
#include "../constraints/actuator_constraints.h"

static float LQR_K_R[12] = LQR_K_MATRIX;

static void init_chassis_state(platform_balance_runtime_t *chassis, vmc_leg_t *right_leg, vmc_leg_t *left_leg);
static void init_chassis_pids(PidTypeDef *roll, PidTypeDef *tp, PidTypeDef *turn, PidTypeDef *leg_r, PidTypeDef *leg_l);
static void run_balance_control(platform_balance_controller_t *state);
static void update_leg_feedback(vmc_leg_t *vmc_r, vmc_leg_t *vmc_l, const platform_device_feedback_t *feedback);
static void update_wheel_feedback(platform_balance_runtime_t *chassis, const platform_device_feedback_t *feedback);
static void update_attitude_feedback(platform_balance_runtime_t *chassis, const vmc_leg_t *vmc_r, const vmc_leg_t *vmc_l, const platform_ins_runtime_t *ins);
static void compute_turn_and_leg_compensation(platform_balance_runtime_t *chassis,
                                              const platform_ins_runtime_t *ins,
                                              const PidTypeDef *turn_pid,
                                              const PidTypeDef *roll_pid,
                                              PidTypeDef *tp_pid);
static void compute_lqr_outputs(platform_balance_runtime_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, const float *LQR_KR, const float *LQR_KL);
static void mix_wheel_torque(platform_balance_runtime_t *chassis);
static void apply_jump_logic(platform_balance_runtime_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, PidTypeDef *legr, PidTypeDef *legl);
static void apply_ground_detection(platform_balance_runtime_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, platform_ins_runtime_t *ins);
static void update_robot_state_contract(platform_balance_controller_t *state);
static void update_actuator_command_contract(platform_balance_controller_t *state);

void platform_balance_controller_init(platform_balance_controller_t *state)
{
    init_chassis_state(&state->chassis, &state->right_leg, &state->left_leg);
    init_chassis_pids(&state->roll_pid, &state->tp_pid, &state->turn_pid, &state->leg_r_pid, &state->leg_l_pid);
}

void platform_balance_controller_apply_inputs(platform_balance_controller_t *state,
                                              const platform_balance_controller_input_t *inputs)
{
    state->ins.attitude.pitch = inputs->ins.pitch;
    state->ins.attitude.roll = inputs->ins.roll;
    state->ins.attitude.yaw_total = inputs->ins.yaw_total;
    state->ins.sensor.gyro[0] = inputs->ins.gyro[0];
    state->ins.sensor.gyro[1] = inputs->ins.gyro[1];
    state->ins.sensor.gyro[2] = inputs->ins.gyro[2];
    state->ins.sensor.body_accel[0] = inputs->ins.accel_b[0];
    state->ins.sensor.body_accel[1] = inputs->ins.accel_b[1];
    state->ins.sensor.body_accel[2] = inputs->ins.accel_b[2];
    state->ins.health.ready = inputs->ins.ready;

    state->chassis.target.velocity_target = inputs->intent.motion_target.vx;
    state->chassis.target.position_target = inputs->intent.motion_target.x;
    state->chassis.target.yaw_target = inputs->intent.motion_target.yaw_hold
                                     ? inputs->intent.motion_target.yaw_target
                                     : inputs->intent.motion_target.yaw_rate;
    state->chassis.target.leg_length_target = inputs->intent.posture_target.leg_length;
    state->chassis.mode.start_enabled = inputs->intent.enable.start ? 1U : 0U;
    state->chassis.mode.jump_requested = inputs->intent.behavior_request.jump_request ? 1U : 0U;
    state->chassis.mode.recover_requested = inputs->intent.behavior_request.recover_request ? 1U : 0U;
    state->chassis.observe.velocity = inputs->observe.v_filter;
    state->chassis.observe.position = inputs->observe.x_filter;
}

void platform_balance_controller_step(platform_balance_controller_t *state, const platform_device_feedback_t *feedback)
{
    update_leg_feedback(&state->right_leg, &state->left_leg, feedback);
    update_wheel_feedback(&state->chassis, feedback);
    update_attitude_feedback(&state->chassis, &state->right_leg, &state->left_leg, &state->ins);

    if (state->ins.attitude.pitch < PITCH_FALL_THRESHOLD && state->ins.attitude.pitch > -PITCH_FALL_THRESHOLD)
    {
        state->chassis.mode.recover_requested = 0;
    }

    run_balance_control(state);
    update_robot_state_contract(state);
    update_actuator_command_contract(state);
}

void platform_balance_controller_build_outputs(const platform_balance_controller_t *state,
                                               platform_balance_controller_output_t *outputs)
{
    outputs->robot_state = state->robot_state;
    outputs->actuator_command = state->actuator_command;
}

static void init_chassis_state(platform_balance_runtime_t *chassis, vmc_leg_t *right_leg, vmc_leg_t *left_leg)
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

    state->right_leg.Tp = state->right_leg.Tp + state->chassis.compensation.leg_pitch;
    state->left_leg.Tp = state->left_leg.Tp + state->chassis.compensation.leg_pitch;
    state->chassis.compensation.roll_force = 0.0f;

    apply_jump_logic(&state->chassis, &state->right_leg, &state->left_leg, &state->leg_r_pid, &state->leg_l_pid);
    apply_ground_detection(&state->chassis, &state->right_leg, &state->left_leg, &state->ins);

    VMC_calc_2(&state->right_leg);
    VMC_calc_2(&state->left_leg);

    state->chassis.wheel_motor[0].give_current = state->chassis.wheel_motor[0].torque_set * M3508_TORQUE_TO_CURRENT;
    state->chassis.wheel_motor[1].give_current = state->chassis.wheel_motor[1].torque_set * M3508_TORQUE_TO_CURRENT;

    platform_constrain_wheel_outputs(&state->chassis);
    platform_constrain_leg_outputs(&state->right_leg, &state->left_leg);
}

static void update_leg_feedback(vmc_leg_t *vmc_r, vmc_leg_t *vmc_l, const platform_device_feedback_t *feedback)
{
    vmc_r->phi1 = pi / 2.0f + feedback->actuator_feedback.joints[0].position + JOINT0_OFFSET;
    vmc_r->phi4 = pi / 2.0f + feedback->actuator_feedback.joints[1].position + JOINT1_OFFSET;
    vmc_l->phi1 = pi / 2.0f + feedback->actuator_feedback.joints[2].position + JOINT2_OFFSET;
    vmc_l->phi4 = pi / 2.0f + feedback->actuator_feedback.joints[3].position + JOINT3_OFFSET;
}

static void update_wheel_feedback(platform_balance_runtime_t *chassis, const platform_device_feedback_t *feedback)
{
    chassis->wheel_motor[0].chassis_x = feedback->actuator_feedback.wheels[0].position;
    chassis->wheel_motor[1].chassis_x = feedback->actuator_feedback.wheels[1].position;
    chassis->wheel_motor[0].speed = feedback->actuator_feedback.wheels[0].velocity;
    chassis->wheel_motor[1].speed = feedback->actuator_feedback.wheels[1].velocity;
    chassis->wheel_motor[0].w_speed = feedback->actuator_feedback.wheels[0].velocity / WHEEL_RADIUS;
    chassis->wheel_motor[1].w_speed = feedback->actuator_feedback.wheels[1].velocity / WHEEL_RADIUS;
}

static void update_attitude_feedback(platform_balance_runtime_t *chassis, const vmc_leg_t *vmc_r, const vmc_leg_t *vmc_l, const platform_ins_runtime_t *ins)
{
    chassis->attitude.left_pitch_rate = -ins->sensor.gyro[0];
    chassis->attitude.left_pitch = -ins->attitude.pitch;
    chassis->attitude.right_pitch = ins->attitude.pitch;
    chassis->attitude.right_pitch_rate = ins->sensor.gyro[0];
    chassis->attitude.yaw_total = ins->attitude.yaw_total;
    chassis->attitude.roll = ins->attitude.roll;
    chassis->attitude.leg_theta_error = 0.0f - (vmc_r->theta + vmc_l->theta);
}

static void update_robot_state_contract(platform_balance_controller_t *state)
{
    state->robot_state.body.roll = state->ins.attitude.roll;
    state->robot_state.body.pitch = state->ins.attitude.pitch;
    state->robot_state.body.yaw = state->ins.attitude.yaw;
    state->robot_state.body.gyro[0] = state->ins.sensor.gyro[0];
    state->robot_state.body.gyro[1] = state->ins.sensor.gyro[1];
    state->robot_state.body.gyro[2] = state->ins.sensor.gyro[2];
    state->robot_state.body.accel[0] = state->ins.sensor.body_accel[0];
    state->robot_state.body.accel[1] = state->ins.sensor.body_accel[1];
    state->robot_state.body.accel[2] = state->ins.sensor.body_accel[2];
    state->robot_state.body.orientation_valid = (state->ins.health.ready != 0U);

    state->robot_state.chassis.x = state->chassis.observe.position;
    state->robot_state.chassis.v = state->chassis.observe.velocity;
    state->robot_state.chassis.vx = state->chassis.observe.velocity;
    state->robot_state.chassis.vy = 0.0f;
    state->robot_state.chassis.yaw_total = state->chassis.attitude.yaw_total;
    state->robot_state.chassis.turn_rate = state->chassis.compensation.turn_torque;
    state->robot_state.chassis.state_valid = (state->ins.health.ready != 0U);

    state->robot_state.legs.right.length = state->right_leg.L0;
    state->robot_state.legs.right.leg_angle = state->right_leg.theta;
    state->robot_state.legs.right.joint_pos[0] = state->right_leg.phi1;
    state->robot_state.legs.right.joint_pos[1] = state->right_leg.phi4;
    state->robot_state.legs.right.joint_vel[0] = 0.0f;
    state->robot_state.legs.right.joint_vel[1] = 0.0f;
    state->robot_state.legs.right.joint_torque_est[0] = state->right_leg.torque_set[0];
    state->robot_state.legs.right.joint_torque_est[1] = state->right_leg.torque_set[1];

    state->robot_state.legs.left.length = state->left_leg.L0;
    state->robot_state.legs.left.leg_angle = state->left_leg.theta;
    state->robot_state.legs.left.joint_pos[0] = state->left_leg.phi1;
    state->robot_state.legs.left.joint_pos[1] = state->left_leg.phi4;
    state->robot_state.legs.left.joint_vel[0] = 0.0f;
    state->robot_state.legs.left.joint_vel[1] = 0.0f;
    state->robot_state.legs.left.joint_torque_est[0] = state->left_leg.torque_set[0];
    state->robot_state.legs.left.joint_torque_est[1] = state->left_leg.torque_set[1];

    state->robot_state.wheels.left.speed = state->chassis.wheel_motor[0].speed;
    state->robot_state.wheels.left.position = state->chassis.wheel_motor[0].chassis_x;
    state->robot_state.wheels.left.torque_est = state->chassis.wheel_motor[0].torque;
    state->robot_state.wheels.left.online = true;
    state->robot_state.wheels.right.speed = state->chassis.wheel_motor[1].speed;
    state->robot_state.wheels.right.position = state->chassis.wheel_motor[1].chassis_x;
    state->robot_state.wheels.right.torque_est = state->chassis.wheel_motor[1].torque;
    state->robot_state.wheels.right.online = true;

    state->robot_state.contact.grounded = (state->chassis.mode.grounded != 0U);
    state->robot_state.contact.left_support = (state->left_leg.FN > FN_GROUND_THRESHOLD);
    state->robot_state.contact.right_support = (state->right_leg.FN > FN_GROUND_THRESHOLD);
    state->robot_state.contact.land_confidence = state->robot_state.contact.grounded ? 1.0f : 0.0f;

    state->robot_state.health.imu_ok = (state->ins.health.ready != 0U);
    state->robot_state.health.remote_ok = true;
    state->robot_state.health.actuator_ok = true;
    state->robot_state.health.state_valid = state->robot_state.body.orientation_valid;
    state->robot_state.health.degraded_mode = (state->chassis.mode.recover_requested != 0U);
}

static void update_actuator_command_contract(platform_balance_controller_t *state)
{
    state->actuator_command.start = (state->chassis.mode.start_enabled != 0U);
    state->actuator_command.control_enable = (state->chassis.mode.start_enabled != 0U);
    state->actuator_command.actuator_enable = (state->chassis.mode.start_enabled != 0U);

    state->actuator_command.motors.left_leg_joint[0].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    state->actuator_command.motors.left_leg_joint[0].torque_target = state->left_leg.torque_set[0];
    state->actuator_command.motors.left_leg_joint[0].valid = (state->chassis.mode.start_enabled != 0U);
    state->actuator_command.motors.left_leg_joint[1].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    state->actuator_command.motors.left_leg_joint[1].torque_target = state->left_leg.torque_set[1];
    state->actuator_command.motors.left_leg_joint[1].valid = (state->chassis.mode.start_enabled != 0U);
    state->actuator_command.motors.right_leg_joint[0].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    state->actuator_command.motors.right_leg_joint[0].torque_target = state->right_leg.torque_set[0];
    state->actuator_command.motors.right_leg_joint[0].valid = (state->chassis.mode.start_enabled != 0U);
    state->actuator_command.motors.right_leg_joint[1].control_mode = PLATFORM_MOTOR_CONTROL_TORQUE;
    state->actuator_command.motors.right_leg_joint[1].torque_target = state->right_leg.torque_set[1];
    state->actuator_command.motors.right_leg_joint[1].valid = (state->chassis.mode.start_enabled != 0U);
    state->actuator_command.motors.left_wheel.control_mode = PLATFORM_MOTOR_CONTROL_CURRENT;
    state->actuator_command.motors.left_wheel.current_target = (float)state->chassis.wheel_motor[0].give_current;
    state->actuator_command.motors.left_wheel.valid = (state->chassis.mode.start_enabled != 0U);
    state->actuator_command.motors.right_wheel.control_mode = PLATFORM_MOTOR_CONTROL_CURRENT;
    state->actuator_command.motors.right_wheel.current_target = (float)state->chassis.wheel_motor[1].give_current;
    state->actuator_command.motors.right_wheel.valid = (state->chassis.mode.start_enabled != 0U);
}

static void compute_turn_and_leg_compensation(platform_balance_runtime_t *chassis,
                                              const platform_ins_runtime_t *ins,
                                              const PidTypeDef *turn_pid,
                                              const PidTypeDef *roll_pid,
                                              PidTypeDef *tp_pid)
{
    chassis->compensation.turn_torque = turn_pid->Kp * (chassis->target.yaw_target - chassis->attitude.yaw_total) - turn_pid->Kd * ins->sensor.gyro[2];
    chassis->compensation.roll_force = roll_pid->Kp * (chassis->target.roll_target - chassis->attitude.roll) - roll_pid->Kd * ins->sensor.gyro[1];
    chassis->compensation.leg_pitch = PID_Calc(tp_pid, chassis->attitude.leg_theta_error, 0.0f);
}

static void compute_lqr_outputs(platform_balance_runtime_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, const float *LQR_KR, const float *LQR_KL)
{
    chassis->wheel_motor[1].torque_set = (LQR_KR[0] * (vmcr->theta) + LQR_KR[1] * (vmcr->d_theta)
                                       + LQR_KR[2] * (chassis->observe.position - chassis->target.position_target) + LQR_KR[3] * (chassis->observe.velocity - 0)
                                       + LQR_KR[4] * (chassis->attitude.right_pitch - 0.0f) + LQR_KR[5] * (chassis->attitude.right_pitch_rate - 0.0f));
    vmcr->Tp = (LQR_KR[6] * (vmcr->theta) + LQR_KR[7] * (vmcr->d_theta)
             + LQR_KR[8] * (chassis->observe.position - chassis->target.position_target) + LQR_KR[9] * (chassis->observe.velocity - 0)
             + LQR_KR[10] * (chassis->attitude.right_pitch) + LQR_KR[11] * (chassis->attitude.right_pitch_rate));

    chassis->wheel_motor[0].torque_set = (LQR_KL[0] * (vmcl->theta) + LQR_KL[1] * (vmcl->d_theta)
                                       + LQR_KL[2] * (chassis->target.position_target - chassis->observe.position) + LQR_KL[3] * (0 - chassis->observe.velocity)
                                       + LQR_KL[4] * (chassis->attitude.left_pitch - 0.0f) + LQR_KL[5] * (chassis->attitude.left_pitch_rate - 0.0f));
    vmcl->Tp = (LQR_KL[6] * (vmcl->theta) + LQR_KL[7] * (vmcl->d_theta)
             + LQR_KL[8] * (chassis->target.position_target - chassis->observe.position) + LQR_KL[9] * (0 - chassis->observe.velocity)
             + LQR_KL[10] * (chassis->attitude.left_pitch) + LQR_KL[11] * (chassis->attitude.left_pitch_rate));
}

static void mix_wheel_torque(platform_balance_runtime_t *chassis)
{
    for (int i = 0; i < 2; i++)
    {
        chassis->wheel_motor[i].torque_set = WHEEL_TORQUE_RATIO * chassis->wheel_motor[i].torque_set
                                           + TURN_TORQUE_RATIO * chassis->compensation.turn_torque;
    }
}

static void apply_jump_logic(platform_balance_runtime_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, PidTypeDef *legr, PidTypeDef *legl)
{
    if (chassis->mode.start_enabled != 1)
    {
        return;
    }

    if (chassis->mode.jump_requested == 1)
    {
        if (chassis->mode.jump_phase == 0)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, 0.18f);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, 0.18f);
            if (vmcr->L0 < 0.185f && vmcl->L0 < 0.185f) { chassis->mode.jump_elapsed_ticks++; }
            if (chassis->mode.jump_elapsed_ticks > 10) { chassis->mode.jump_elapsed_ticks = 0; chassis->mode.jump_phase = 1; }
        }
        else if (chassis->mode.jump_phase == 1)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, 0.3f);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, 0.3f);
            if (vmcr->L0 > 0.22f && vmcl->L0 > 0.22f) { chassis->mode.jump_elapsed_ticks++; }
            if (chassis->mode.jump_elapsed_ticks > 10) { chassis->mode.jump_elapsed_ticks = 0; chassis->mode.jump_phase = 2; }
        }
        else if (chassis->mode.jump_phase == 2)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, 0.18f);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, 0.18f);
            if (vmcr->L0 < 0.250f && vmcl->L0 < 0.250f) { chassis->mode.jump_elapsed_ticks++; }
            if (chassis->mode.jump_elapsed_ticks > 10) { chassis->mode.jump_elapsed_ticks = 0; chassis->mode.jump_phase = 3; }
        }
        else if (chassis->mode.jump_phase == 3)
        {
            vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, chassis->target.leg_length_target);
            vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, chassis->target.leg_length_target);
        }
    }
    else
    {
        vmcr->F0 = LEG_GRAVITY_COMP + PID_Calc(legr, vmcr->L0, chassis->target.leg_length_target) + chassis->compensation.roll_force;
        vmcl->F0 = LEG_GRAVITY_COMP + PID_Calc(legl, vmcl->L0, chassis->target.leg_length_target) - chassis->compensation.roll_force;
        chassis->mode.jump_elapsed_ticks = 0;
        chassis->mode.jump_phase = 0;
    }
}

static void apply_ground_detection(platform_balance_runtime_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl, platform_ins_runtime_t *ins)
{
    uint8_t right_flag = ground_detectionR(vmcr, ins);
    uint8_t left_flag = ground_detectionL(vmcl, ins);

    if (chassis->mode.recover_requested == 0)
    {
        if (right_flag == 1 && left_flag == 1)
        {
            chassis->wheel_motor[0].torque_set = 0.0f;
            chassis->wheel_motor[1].torque_set = 0.0f;
            chassis->observe.position = 0.0f;
            chassis->target.position_target = chassis->observe.position;
            chassis->target.yaw_target = chassis->attitude.yaw_total;
            vmcr->Tp = vmcr->Tp + chassis->compensation.leg_pitch;
            chassis->mode.grounded = 1;
        }
        else
        {
            chassis->mode.grounded = 0;
        }
    }
    else if (chassis->mode.recover_requested == 1)
    {
        vmcr->Tp = 0.0f;
        vmcl->Tp = 0.0f;
    }
}
