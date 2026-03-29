#include "ins_runtime.h"

#include <math.h>

static void body_frame_to_earth_frame(const float *vec_bf, float *vec_ef, const float *q);
static void earth_frame_to_body_frame(const float *vec_ef, float *vec_bf, const float *q);

void ins_runtime_state_init(INS_Runtime_State_t *state)
{
    mahony_init(&state->mahony, 1.0f, 0.0f, 0.001f);
    state->ins.AccelLPF = 0.0089f;
    state->gravity[0] = 0.0f;
    state->gravity[1] = 0.0f;
    state->gravity[2] = 9.81f;
    state->dwt_count = 0U;
    state->ins_time = 0.0f;
}

void ins_runtime_apply_sample(INS_Runtime_State_t *state,
                              float dt,
                              const float accel[3],
                              const float gyro[3])
{
    float gravity_b[3];

    state->mahony.dt = dt;

    state->ins.Accel[X] = accel[X];
    state->ins.Accel[Y] = accel[Y];
    state->ins.Accel[Z] = accel[Z];
    state->ins.Gyro[X] = gyro[X];
    state->ins.Gyro[Y] = gyro[Y];
    state->ins.Gyro[Z] = gyro[Z];

    state->accel.x = accel[0];
    state->accel.y = accel[1];
    state->accel.z = accel[2];
    state->gyro.x = gyro[0];
    state->gyro.y = gyro[1];
    state->gyro.z = gyro[2];

    mahony_input(&state->mahony, state->gyro, state->accel);
    mahony_update(&state->mahony);
    mahony_output(&state->mahony);
    RotationMatrix_update(&state->mahony);

    state->ins.q[0] = state->mahony.q0;
    state->ins.q[1] = state->mahony.q1;
    state->ins.q[2] = state->mahony.q2;
    state->ins.q[3] = state->mahony.q3;

    earth_frame_to_body_frame(state->gravity, gravity_b, state->ins.q);
    for (uint8_t i = 0; i < 3; i++)
    {
        state->ins.MotionAccel_b[i] = (state->ins.Accel[i] - gravity_b[i]) * dt / (state->ins.AccelLPF + dt)
                                    + state->ins.MotionAccel_b[i] * state->ins.AccelLPF / (state->ins.AccelLPF + dt);
    }
    body_frame_to_earth_frame(state->ins.MotionAccel_b, state->ins.MotionAccel_n, state->ins.q);

    if (fabsf(state->ins.MotionAccel_n[0]) < 0.02f)
    {
        state->ins.MotionAccel_n[0] = 0.0f;
    }
    if (fabsf(state->ins.MotionAccel_n[1]) < 0.02f)
    {
        state->ins.MotionAccel_n[1] = 0.0f;
    }
    if (fabsf(state->ins.MotionAccel_n[2]) < 0.04f)
    {
        state->ins.MotionAccel_n[2] = 0.0f;
    }

    if (state->ins_time > 3000.0f)
    {
        state->ins.ins_flag = 1U;
        state->ins.Pitch = state->mahony.roll - PITCH_OFFSET;
        state->ins.Roll = state->mahony.pitch - ROLL_OFFSET;
        state->ins.Yaw = state->mahony.yaw;

        if (state->ins.Yaw - state->ins.YawAngleLast > 3.1415926f)
        {
            state->ins.YawRoundCount--;
        }
        else if (state->ins.Yaw - state->ins.YawAngleLast < -3.1415926f)
        {
            state->ins.YawRoundCount++;
        }
        state->ins.YawTotalAngle = 6.283f * state->ins.YawRoundCount + state->ins.Yaw;
        state->ins.YawAngleLast = state->ins.Yaw;
    }
    else
    {
        state->ins_time++;
    }
}

void ins_runtime_build_msg(const INS_Runtime_State_t *state, INS_Data_t *msg)
{
    msg->pitch = state->ins.Pitch;
    msg->roll = state->ins.Roll;
    msg->yaw_total = state->ins.YawTotalAngle;
    msg->gyro[0] = state->ins.Gyro[0];
    msg->gyro[1] = state->ins.Gyro[1];
    msg->gyro[2] = state->ins.Gyro[2];
    msg->accel_b[0] = state->ins.MotionAccel_b[0];
    msg->accel_b[1] = state->ins.MotionAccel_b[1];
    msg->accel_b[2] = state->ins.MotionAccel_b[2];
    msg->ready = state->ins.ins_flag;
}

void ins_runtime_fill_robot_state(const INS_Runtime_State_t *state, platform_robot_state_t *robot_state)
{
    robot_state->timestamp_us = state->dwt_count;
    robot_state->body.roll = state->ins.Roll;
    robot_state->body.pitch = state->ins.Pitch;
    robot_state->body.yaw = state->ins.Yaw;
    robot_state->body.gyro[0] = state->ins.Gyro[0];
    robot_state->body.gyro[1] = state->ins.Gyro[1];
    robot_state->body.gyro[2] = state->ins.Gyro[2];
    robot_state->body.accel[0] = state->ins.MotionAccel_b[0];
    robot_state->body.accel[1] = state->ins.MotionAccel_b[1];
    robot_state->body.accel[2] = state->ins.MotionAccel_b[2];
    robot_state->body.orientation_valid = (state->ins.ins_flag != 0U);

    robot_state->chassis.yaw_total = state->ins.YawTotalAngle;

    robot_state->health.imu_ok = (state->ins.ins_flag != 0U);
    robot_state->health.state_valid = robot_state->body.orientation_valid;
}

static void body_frame_to_earth_frame(const float *vec_bf, float *vec_ef, const float *q)
{
    vec_ef[0] = 2.0f * ((0.5f - q[2] * q[2] - q[3] * q[3]) * vec_bf[0]
                      + (q[1] * q[2] - q[0] * q[3]) * vec_bf[1]
                      + (q[1] * q[3] + q[0] * q[2]) * vec_bf[2]);

    vec_ef[1] = 2.0f * ((q[1] * q[2] + q[0] * q[3]) * vec_bf[0]
                      + (0.5f - q[1] * q[1] - q[3] * q[3]) * vec_bf[1]
                      + (q[2] * q[3] - q[0] * q[1]) * vec_bf[2]);

    vec_ef[2] = 2.0f * ((q[1] * q[3] - q[0] * q[2]) * vec_bf[0]
                      + (q[2] * q[3] + q[0] * q[1]) * vec_bf[1]
                      + (0.5f - q[1] * q[1] - q[2] * q[2]) * vec_bf[2]);
}

static void earth_frame_to_body_frame(const float *vec_ef, float *vec_bf, const float *q)
{
    vec_bf[0] = 2.0f * ((0.5f - q[2] * q[2] - q[3] * q[3]) * vec_ef[0]
                      + (q[1] * q[2] + q[0] * q[3]) * vec_ef[1]
                      + (q[1] * q[3] - q[0] * q[2]) * vec_ef[2]);

    vec_bf[1] = 2.0f * ((q[1] * q[2] - q[0] * q[3]) * vec_ef[0]
                      + (0.5f - q[1] * q[1] - q[3] * q[3]) * vec_ef[1]
                      + (q[2] * q[3] + q[0] * q[1]) * vec_ef[2]);

    vec_bf[2] = 2.0f * ((q[1] * q[3] + q[0] * q[2]) * vec_ef[0]
                      + (q[2] * q[3] - q[0] * q[1]) * vec_ef[1]
                      + (0.5f - q[1] * q[1] - q[2] * q[2]) * vec_ef[2]);
}
