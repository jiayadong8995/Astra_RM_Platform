/**
 ******************************************************************************
 * @file    INS_task.h
 * @author  Wang Hongxi
 * @version V2.0.0
 * @date    2022/2/23
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */
#ifndef __INS_TASK_H
#define __INS_TASK_H

#include "../app_config/runtime_state.h"

#define INS_TASK_PERIOD 1

extern void INS_Init(void);
extern void INS_task(void);

#endif
