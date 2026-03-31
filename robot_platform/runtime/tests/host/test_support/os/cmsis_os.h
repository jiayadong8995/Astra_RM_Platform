#ifndef PLATFORM_TEST_CMSIS_OS_H
#define PLATFORM_TEST_CMSIS_OS_H

#include <stdint.h>

void osDelay(uint32_t milliseconds);
uint32_t osKernelSysTick(void);

#endif
