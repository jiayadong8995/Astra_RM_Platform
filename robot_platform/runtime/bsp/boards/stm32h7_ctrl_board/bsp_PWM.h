/**
 ******************************************************************************
 * @file    bsp_PWM.h
 ******************************************************************************
 */
#ifndef BSP_PWM_H
#define BSP_PWM_H

#include <stdint.h>

#include "tim.h"

void TIM_Set_PWM(TIM_HandleTypeDef *tim_pwmHandle, uint8_t Channel, uint16_t value);

#endif
