#ifndef PLATFORM_CONTROL_STATE_INS_STATE_MESSAGE_H
#define PLATFORM_CONTROL_STATE_INS_STATE_MESSAGE_H

#include <stdint.h>

typedef struct {
    float pitch;
    float roll;
    float yaw_total;
    float gyro[3];
    float accel_b[3];
    uint8_t ready;
} platform_ins_state_message_t;

#endif
