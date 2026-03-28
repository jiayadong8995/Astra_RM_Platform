#include "dm4310_drv.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static int motor_sock = -1;
static struct sockaddr_in sim_addr;
static chassis_motor_measure_t motor_measures[4]; // 2 wheels (0,1), 2 joints (2,3) or whatever

static void ensure_sock_init(void) {
    if (motor_sock >= 0) return;
    
    motor_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (motor_sock >= 0) {
        int flags = fcntl(motor_sock, F_GETFL, 0);
        fcntl(motor_sock, F_SETFL, flags | O_NONBLOCK);
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(9002); // Motor feedback port
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        bind(motor_sock, (struct sockaddr *)&addr, sizeof(addr));

        // Address to send commands to
        memset(&sim_addr, 0, sizeof(sim_addr));
        sim_addr.sin_family = AF_INET;
        sim_addr.sin_port = htons(9003); // Python Sim Motor command port
        sim_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
}

// Receive motor state asynchronously
static void poll_motor_feedback(void) {
    ensure_sock_init();
    if (motor_sock < 0) return;

    // UDP packet format: | id (1 uint32) | pos (float) | vel (float) | torque (float) |
    struct __attribute__((packed)) {
        uint32_t id;
        float pos;
        float vel;
        float torque;
    } pkt;

    int n;
    while ((n = recv(motor_sock, &pkt, sizeof(pkt), 0)) == sizeof(pkt)) {
        // Map CAN ID (like 0x01..0x04) to array index 0..3 
        // We do a simple mapping: motor_id - 1
        if (pkt.id >= 1 && pkt.id <= 4) {
            uint8_t idx = pkt.id - 1;
            motor_measures[idx].last_ecd = motor_measures[idx].ecd;
            // Fake an ECD value (0-8191) from radians
            float rotations = pkt.pos / (2.0f * 3.14159265f);
            float remainder = rotations - (int)rotations;
            if (remainder < 0) remainder += 1.0f;
            motor_measures[idx].ecd = (uint16_t)(remainder * 8191.0f);
            motor_measures[idx].speed_rpm = (int16_t)(pkt.vel * 9.54929658f); // rad/s to rpm
            motor_measures[idx].given_current = (int16_t)(pkt.torque * 1000); // fake current
        }
    }
}

void mit_ctrl(FDCAN_HandleTypeDef *hcan, uint16_t motor_id, float pos, float vel, float kp, float kd, float torq) {
    (void)hcan;
    poll_motor_feedback();
    ensure_sock_init();

    struct __attribute__((packed)) {
        uint32_t type; // 1 = MIT
        uint32_t id;
        float p;
        float v;
        float kp;
        float kd;
        float t;
    } cmd = {1, motor_id, pos, vel, kp, kd, torq};

    sendto(motor_sock, &cmd, sizeof(cmd), 0, (struct sockaddr *)&sim_addr, sizeof(sim_addr));
}

void CAN_cmd_chassis(FDCAN_HandleTypeDef *hcan, int16_t motor1, int16_t motor2, int16_t rev1, int16_t rev2) {
    (void)hcan;
    (void)rev1;
    (void)rev2;
    poll_motor_feedback();
    ensure_sock_init();

    struct __attribute__((packed)) {
        uint32_t type; // 2 = PWM/Current mode
        float m1_current;
        float m2_current;
    } cmd = {2, motor1, motor2};

    sendto(motor_sock, &cmd, sizeof(cmd), 0, (struct sockaddr *)&sim_addr, sizeof(sim_addr));
}

void DM_Motor_Init(void) {
    ensure_sock_init();
}

chassis_motor_measure_t *get_chassis_motor_measure_point(uint8_t i) {
    poll_motor_feedback();
    if (i < 4) return &motor_measures[i];
    return &motor_measures[0];
}

int enable_motor_mode(FDCAN_HandleTypeDef *hcan, uint16_t id, uint16_t mode) { (void)hcan; (void)id; (void)mode; return 0; }
void motor_sys_rest(FDCAN_HandleTypeDef *hcan) { (void)hcan; }
void joint_motor_init(Joint_Motor_t *motor, uint16_t id, uint16_t mode) { (void)motor; (void)id; (void)mode; }
