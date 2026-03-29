#include "ins_state_estimator.h"

#include <math.h>

static void body_frame_to_earth_frame(const float *vec_bf, float *vec_ef, const float *q);
static void earth_frame_to_body_frame(const float *vec_ef, float *vec_bf, const float *q);

void platform_ins_state_estimator_init(platform_ins_state_estimator_t *state)
{
    mahony_init(&state->mahony, 1.0f, 0.0f, 0.001f);
    state->ins.sensor.accel_lpf = 0.0089f;
    state->gravity[0] = 0.0f;
    state->gravity[1] = 0.0f;
    state->gravity[2] = 9.81f;
    state->dwt_count = 0U;
    state->ins_time = 0.0f;
}

void platform_ins_state_estimator_apply_sample(platform_ins_state_estimator_t *state,
                                               float dt,
                                               const float accel[3],
                                               const float gyro[3])
{
    float gravity_b[3];

    state->mahony.dt = dt;

    state->ins.sensor.accel[PLATFORM_AXIS_X] = accel[PLATFORM_AXIS_X];
    state->ins.sensor.accel[PLATFORM_AXIS_Y] = accel[PLATFORM_AXIS_Y];
    state->ins.sensor.accel[PLATFORM_AXIS_Z] = accel[PLATFORM_AXIS_Z];
    state->ins.sensor.gyro[PLATFORM_AXIS_X] = gyro[PLATFORM_AXIS_X];
    state->ins.sensor.gyro[PLATFORM_AXIS_Y] = gyro[PLATFORM_AXIS_Y];
    state->ins.sensor.gyro[PLATFORM_AXIS_Z] = gyro[PLATFORM_AXIS_Z];

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

    state->ins.sensor.quaternion[0] = state->mahony.q0;
    state->ins.sensor.quaternion[1] = state->mahony.q1;
    state->ins.sensor.quaternion[2] = state->mahony.q2;
    state->ins.sensor.quaternion[3] = state->mahony.q3;

    earth_frame_to_body_frame(state->gravity, gravity_b, state->ins.sensor.quaternion);
    for (uint8_t i = 0; i < 3; i++)
    {
        state->ins.sensor.body_accel[i] = (state->ins.sensor.accel[i] - gravity_b[i]) * dt / (state->ins.sensor.accel_lpf + dt)
                                        + state->ins.sensor.body_accel[i] * state->ins.sensor.accel_lpf / (state->ins.sensor.accel_lpf + dt);
    }
    body_frame_to_earth_frame(state->ins.sensor.body_accel, state->ins.sensor.world_accel, state->ins.sensor.quaternion);

    if (fabsf(state->ins.sensor.world_accel[0]) < 0.02f)
    {
        state->ins.sensor.world_accel[0] = 0.0f;
    }
    if (fabsf(state->ins.sensor.world_accel[1]) < 0.02f)
    {
        state->ins.sensor.world_accel[1] = 0.0f;
    }
    if (fabsf(state->ins.sensor.world_accel[2]) < 0.04f)
    {
        state->ins.sensor.world_accel[2] = 0.0f;
    }

    if (state->ins_time > 3000.0f)
    {
        state->ins.health.ready = 1U;
        state->ins.attitude.pitch = state->mahony.roll - PITCH_OFFSET;
        state->ins.attitude.roll = state->mahony.pitch - ROLL_OFFSET;
        state->ins.attitude.yaw = state->mahony.yaw;

        if (state->ins.attitude.yaw - state->ins.attitude.yaw_last > 3.1415926f)
        {
            state->ins.attitude.yaw_turn_count--;
        }
        else if (state->ins.attitude.yaw - state->ins.attitude.yaw_last < -3.1415926f)
        {
            state->ins.attitude.yaw_turn_count++;
        }
        state->ins.attitude.yaw_total = 6.283f * state->ins.attitude.yaw_turn_count + state->ins.attitude.yaw;
        state->ins.attitude.yaw_last = state->ins.attitude.yaw;
    }
    else
    {
        state->ins_time++;
    }
}

void platform_ins_state_estimator_build_msg(const platform_ins_state_estimator_t *state,
                                            platform_ins_state_message_t *msg)
{
    msg->pitch = state->ins.attitude.pitch;
    msg->roll = state->ins.attitude.roll;
    msg->yaw_total = state->ins.attitude.yaw_total;
    msg->gyro[0] = state->ins.sensor.gyro[0];
    msg->gyro[1] = state->ins.sensor.gyro[1];
    msg->gyro[2] = state->ins.sensor.gyro[2];
    msg->accel_b[0] = state->ins.sensor.body_accel[0];
    msg->accel_b[1] = state->ins.sensor.body_accel[1];
    msg->accel_b[2] = state->ins.sensor.body_accel[2];
    msg->ready = state->ins.health.ready;
}

void platform_ins_state_estimator_fill_robot_state(const platform_ins_state_estimator_t *state,
                                                   platform_robot_state_t *robot_state)
{
    robot_state->timestamp_us = state->dwt_count;
    robot_state->body.roll = state->ins.attitude.roll;
    robot_state->body.pitch = state->ins.attitude.pitch;
    robot_state->body.yaw = state->ins.attitude.yaw;
    robot_state->body.gyro[0] = state->ins.sensor.gyro[0];
    robot_state->body.gyro[1] = state->ins.sensor.gyro[1];
    robot_state->body.gyro[2] = state->ins.sensor.gyro[2];
    robot_state->body.accel[0] = state->ins.sensor.body_accel[0];
    robot_state->body.accel[1] = state->ins.sensor.body_accel[1];
    robot_state->body.accel[2] = state->ins.sensor.body_accel[2];
    robot_state->body.orientation_valid = (state->ins.health.ready != 0U);
    robot_state->chassis.yaw_total = state->ins.attitude.yaw_total;
    robot_state->health.imu_ok = (state->ins.health.ready != 0U);
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
