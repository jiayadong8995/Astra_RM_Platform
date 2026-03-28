#include "chassis_runtime_helpers.h"

#include "user_lib.h"

static void update_leg_feedback(vmc_leg_t *vmc_r, vmc_leg_t *vmc_l, const Actuator_Feedback_t *feedback);
static void update_wheel_feedback(chassis_t *chassis, const Actuator_Feedback_t *feedback);
static void update_attitude_feedback(chassis_t *chassis, const vmc_leg_t *vmc_r, const vmc_leg_t *vmc_l, const INS_t *ins);
static void limit_int(int16_t *in, int16_t min, int16_t max);
static void saturate_wheel_outputs(chassis_t *chassis);
static void saturate_leg_outputs(vmc_leg_t *vmcr, vmc_leg_t *vmcl);

void chassis_apply_feedback_snapshot(chassis_t *chassis,
                                     vmc_leg_t *vmc_r,
                                     vmc_leg_t *vmc_l,
                                     const INS_t *ins,
                                     const Actuator_Feedback_t *feedback)
{
    update_leg_feedback(vmc_r, vmc_l, feedback);
    update_wheel_feedback(chassis, feedback);
    update_attitude_feedback(chassis, vmc_r, vmc_l, ins);

    if (ins->Pitch < PITCH_FALL_THRESHOLD && ins->Pitch > -PITCH_FALL_THRESHOLD)
    {
        chassis->recover_flag = 0;
    }
}

void chassis_compute_turn_and_leg_compensation(chassis_t *chassis,
                                               const INS_t *ins,
                                               const PidTypeDef *turn_pid,
                                               const PidTypeDef *roll_pid,
                                               PidTypeDef *tp_pid)
{
    chassis->turn_T = turn_pid->Kp * (chassis->turn_set - chassis->total_yaw) - turn_pid->Kd * ins->Gyro[2];
    chassis->roll_f0 = roll_pid->Kp * (chassis->roll_set - chassis->roll) - roll_pid->Kd * ins->Gyro[1];
    chassis->leg_tp = PID_Calc(tp_pid, chassis->theta_err, 0.0f);
}

void chassis_compute_lqr_outputs(chassis_t *chassis,
                                 vmc_leg_t *vmcr,
                                 vmc_leg_t *vmcl,
                                 const float *LQR_KR,
                                 const float *LQR_KL)
{
    chassis->wheel_motor[1].torque_set = (LQR_KR[0] * (vmcr->theta)
                                       + LQR_KR[1] * (vmcr->d_theta)
                                       + LQR_KR[2] * (chassis->x_filter - chassis->x_set)
                                       + LQR_KR[3] * (chassis->v_filter - 0)
                                       + LQR_KR[4] * (chassis->myPithR - 0.0f)
                                       + LQR_KR[5] * (chassis->myPithGyroR - 0.0f));
    vmcr->Tp = (LQR_KR[6] * (vmcr->theta)
             + LQR_KR[7] * (vmcr->d_theta)
             + LQR_KR[8] * (chassis->x_filter - chassis->x_set)
             + LQR_KR[9] * (chassis->v_filter - 0)
             + LQR_KR[10] * (chassis->myPithR)
             + LQR_KR[11] * (chassis->myPithGyroR));

    chassis->wheel_motor[0].torque_set = (LQR_KL[0] * (vmcl->theta)
                                       + LQR_KL[1] * (vmcl->d_theta)
                                       + LQR_KL[2] * (chassis->x_set - chassis->x_filter)
                                       + LQR_KL[3] * (0 - chassis->v_filter)
                                       + LQR_KL[4] * (chassis->myPithL - 0.0f)
                                       + LQR_KL[5] * (chassis->myPithGyroL - 0.0f));
    vmcl->Tp = (LQR_KL[6] * (vmcl->theta)
             + LQR_KL[7] * (vmcl->d_theta)
             + LQR_KL[8] * (chassis->x_set - chassis->x_filter)
             + LQR_KL[9] * (0 - chassis->v_filter)
             + LQR_KL[10] * (chassis->myPithL)
             + LQR_KL[11] * (chassis->myPithGyroL));
}

void chassis_mix_wheel_torque(chassis_t *chassis)
{
    for (int i = 0; i < 2; i++)
    {
        chassis->wheel_motor[i].torque_set = WHEEL_TORQUE_RATIO * chassis->wheel_motor[i].torque_set
                                           + TURN_TORQUE_RATIO * chassis->turn_T;
    }
}

void chassis_saturate_outputs(chassis_t *chassis, vmc_leg_t *vmcr, vmc_leg_t *vmcl)
{
    saturate_wheel_outputs(chassis);
    saturate_leg_outputs(vmcr, vmcl);
}

static void update_leg_feedback(vmc_leg_t *vmc_r, vmc_leg_t *vmc_l, const Actuator_Feedback_t *feedback)
{
    vmc_r->phi1 = pi / 2.0f + feedback->joint_pos[0] + JOINT0_OFFSET;
    vmc_r->phi4 = pi / 2.0f + feedback->joint_pos[1] + JOINT1_OFFSET;
    vmc_l->phi1 = pi / 2.0f + feedback->joint_pos[2] + JOINT2_OFFSET;
    vmc_l->phi4 = pi / 2.0f + feedback->joint_pos[3] + JOINT3_OFFSET;
}

static void update_wheel_feedback(chassis_t *chassis, const Actuator_Feedback_t *feedback)
{
    chassis->wheel_motor[0].chassis_x = feedback->wheel_angle[0];
    chassis->wheel_motor[1].chassis_x = feedback->wheel_angle[1];
    chassis->wheel_motor[0].speed = feedback->wheel_speed[0];
    chassis->wheel_motor[1].speed = feedback->wheel_speed[1];
    chassis->wheel_motor[0].w_speed = feedback->wheel_speed[0] / WHEEL_RADIUS;
    chassis->wheel_motor[1].w_speed = feedback->wheel_speed[1] / WHEEL_RADIUS;
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

static void limit_int(int16_t *in, int16_t min, int16_t max)
{
    if (*in < min)
    {
        *in = min;
    }
    else if (*in > max)
    {
        *in = max;
    }
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
