#include "can_bsp.h"

void FDCAN1_Config(void) {
    // SITL dummy
}

void FDCAN2_Config(void) {
    // SITL dummy
}

uint8_t canx_send_data(FDCAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len) {
    (void)hcan;
    (void)id;
    (void)data;
    (void)len;
    return 0; // Success
}
