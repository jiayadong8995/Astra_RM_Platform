#include "ports.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "drivers/imu/bmi088/BMI088driver.h"
#include "drivers/remote/dbus/remote_control.h"

/* ---- SITL motor UDP context ---- */

static struct {
    int sock_fd;
    struct sockaddr_in sim_addr;
    bool initialized;
} g_motor_sitl = { .sock_fd = -1 };

static bool g_ports_initialized = false;

/* ---- helpers ---- */

static uint32_t platform_sitl_time_us(void)
{
    struct timespec now = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
    {
        return 0U;
    }
    return (uint32_t)(((uint64_t)now.tv_sec * 1000000ULL) + ((uint64_t)now.tv_nsec / 1000ULL));
}

static platform_device_result_t platform_motor_sitl_ensure_socket(void)
{
    if (g_motor_sitl.initialized)
    {
        return PLATFORM_DEVICE_RESULT_OK;
    }

    g_motor_sitl.sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_motor_sitl.sock_fd < 0)
    {
        return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
    }

    memset(&g_motor_sitl.sim_addr, 0, sizeof(g_motor_sitl.sim_addr));
    g_motor_sitl.sim_addr.sin_family = AF_INET;
    g_motor_sitl.sim_addr.sin_port = htons(9003);
    g_motor_sitl.sim_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    g_motor_sitl.initialized = true;
    return PLATFORM_DEVICE_RESULT_OK;
}

/* ---- public API ---- */

void platform_ports_init(void)
{
    if (g_ports_initialized)
    {
        return;
    }

    BMI088_Init(0, 0U);
    platform_motor_sitl_ensure_socket();
    g_ports_initialized = true;
}

platform_device_result_t platform_imu_read(platform_imu_sample_t *sample)
{
    BMI088_Read(&BMI088);

    sample->accel[0] = BMI088.Accel[0];
    sample->accel[1] = BMI088.Accel[1];
    sample->accel[2] = BMI088.Accel[2];
    sample->gyro[0] = BMI088.Gyro[0];
    sample->gyro[1] = BMI088.Gyro[1];
    sample->gyro[2] = BMI088.Gyro[2];
    sample->temperature = BMI088.Temperature;
    sample->sample_time_us = platform_sitl_time_us();
    sample->valid = true;
    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_remote_read(platform_rc_input_t *input)
{
    const RC_ctrl_t *rc = get_remote_control_point();

    if (rc == 0)
    {
        return PLATFORM_DEVICE_RESULT_INVALID;
    }

    input->channels[0] = rc->rc.ch[0];
    input->channels[1] = rc->rc.ch[1];
    input->channels[2] = rc->rc.ch[2];
    input->channels[3] = rc->rc.ch[3];
    input->channels[4] = rc->rc.ch[4];
    input->switches[0] = (uint8_t)rc->rc.s[0];
    input->switches[1] = (uint8_t)rc->rc.s[1];
    input->mouse_x = rc->mouse.x;
    input->mouse_y = rc->mouse.y;
    input->mouse_z = rc->mouse.z;
    input->mouse_left = rc->mouse.press_l;
    input->mouse_right = rc->mouse.press_r;
    input->keyboard_mask = rc->key.v;
    input->source = 1U;
    input->sample_time_us = platform_sitl_time_us();
    input->valid = (RC_data_is_error() == 0U);
    return input->valid ? PLATFORM_DEVICE_RESULT_OK : PLATFORM_DEVICE_RESULT_INVALID;
}

platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd)
{
    platform_device_result_t result = platform_motor_sitl_ensure_socket();
    if (result != PLATFORM_DEVICE_RESULT_OK)
    {
        return result;
    }

    const platform_motor_command_t *joint_cmds[4] = {
        &cmd->joints[PLATFORM_JOINT_LEFT_FRONT],
        &cmd->joints[PLATFORM_JOINT_LEFT_REAR],
        &cmd->joints[PLATFORM_JOINT_RIGHT_FRONT],
        &cmd->joints[PLATFORM_JOINT_RIGHT_REAR],
    };

    for (uint32_t i = 0; i < 4U; ++i)
    {
        struct __attribute__((packed)) {
            uint32_t type;
            uint32_t id;
            float position;
            float velocity;
            float kp;
            float kd;
            float torque;
        } packet = {
            .type = 1U,
            .id = i + 1U,
            .position = joint_cmds[i]->position_target,
            .velocity = joint_cmds[i]->velocity_target,
            .kp = joint_cmds[i]->kp,
            .kd = joint_cmds[i]->kd,
            .torque = joint_cmds[i]->torque_target,
        };

        ssize_t sent = sendto(g_motor_sitl.sock_fd, &packet, sizeof(packet), 0,
                              (struct sockaddr *)&g_motor_sitl.sim_addr,
                              sizeof(g_motor_sitl.sim_addr));
        if (sent != (ssize_t)sizeof(packet))
        {
            return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
        }
    }

    struct __attribute__((packed)) {
        uint32_t type;
        float motor1_current;
        float motor2_current;
    } wheel_packet = {
        .type = 2U,
        .motor1_current = cmd->wheels[PLATFORM_WHEEL_LEFT].current_target,
        .motor2_current = cmd->wheels[PLATFORM_WHEEL_RIGHT].current_target,
    };

    ssize_t sent = sendto(g_motor_sitl.sock_fd, &wheel_packet, sizeof(wheel_packet), 0,
                          (struct sockaddr *)&g_motor_sitl.sim_addr,
                          sizeof(g_motor_sitl.sim_addr));
    if (sent != (ssize_t)sizeof(wheel_packet))
    {
        return PLATFORM_DEVICE_RESULT_UNAVAILABLE;
    }

    return PLATFORM_DEVICE_RESULT_OK;
}

platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *feedback)
{
    for (uint8_t i = 0; i < PLATFORM_JOINT_MOTOR_COUNT; ++i)
    {
        feedback->actuator_feedback.joints[i].id = i;
        feedback->actuator_feedback.joints[i].kind = PLATFORM_MOTOR_KIND_JOINT;
        feedback->actuator_feedback.joints[i].online = true;
    }

    for (uint8_t i = 0; i < PLATFORM_WHEEL_MOTOR_COUNT; ++i)
    {
        feedback->actuator_feedback.wheels[i].id = i;
        feedback->actuator_feedback.wheels[i].kind = PLATFORM_MOTOR_KIND_WHEEL;
        feedback->actuator_feedback.wheels[i].online = true;
    }

    feedback->actuator_feedback.sample_time_us = 1000U;
    feedback->actuator_feedback.valid = true;
    return PLATFORM_DEVICE_RESULT_OK;
}
