#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usart.h"

void uart1_tx_dma_transmit(uint8_t *buff, uint16_t len);

#endif
