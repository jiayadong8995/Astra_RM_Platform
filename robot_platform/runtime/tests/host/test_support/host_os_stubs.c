#include "os/cmsis_os.h"

static uint32_t g_platform_test_tick;

void osDelay(uint32_t milliseconds)
{
    g_platform_test_tick += milliseconds;
}

uint32_t osKernelSysTick(void)
{
    return g_platform_test_tick;
}
