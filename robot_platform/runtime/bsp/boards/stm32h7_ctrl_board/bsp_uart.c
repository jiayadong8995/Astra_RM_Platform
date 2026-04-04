#include "bsp_uart.h"

#include "dma.h"
#include "main.h"
#include "../../drivers/remote/dbus/remote_control.h"
#include "../../app/balance_chassis/app_detect/detect_task.h"
#include "usart.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;

void uart1_tx_dma_transmit(uint8_t *buff, uint16_t len)
{
    if (HAL_DMA_GetState(&hdma_usart1_tx) != HAL_DMA_STATE_READY)
    {
        return;
    }

    HAL_UART_Transmit_DMA(&huart1, buff, len);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == UART5)
    {
        if (Size == RC_FRAME_LENGTH)
        {
            HAL_UARTEx_ReceiveToIdle_DMA(&huart5, sbus_rx_buf, RC_FRAME_LENGTH * 2);
            sbus_to_rc(sbus_rx_buf, &rc_ctrl);
            detect_hook(DBUS_TOE);
        }
        else
        {
            HAL_UARTEx_ReceiveToIdle_DMA(&huart5, sbus_rx_buf, RC_FRAME_LENGTH * 2);
            memset(sbus_rx_buf, 0, RC_FRAME_LENGTH);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)
    {
        HAL_UART_Receive_DMA(&huart5, sbus_rx_buf, RC_FRAME_LENGTH * 2);
        sbus_to_rc(sbus_rx_buf, &rc_ctrl);
        detect_hook(DBUS_TOE);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart5, sbus_rx_buf, RC_FRAME_LENGTH * 2);
        memset(sbus_rx_buf, 0, RC_FRAME_LENGTH);
    }
}
