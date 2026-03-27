#ifndef CAN_BSP_H
#define CAN_BSP_H

#include "main.h"

typedef FDCAN_HandleTypeDef hcan_t;

void FDCAN1_Config(void);
void FDCAN2_Config(void);
uint8_t canx_send_data(FDCAN_HandleTypeDef *hcan, uint16_t id, uint8_t *data, uint32_t len);

#endif
