#include <stdio.h>
#include <unistd.h>
#include "FreeRTOS.h"
#include "task.h"

extern void MX_FREERTOS_Init(void);

int main(void) {
    printf("[SITL] Starting Astra Balance Chassis SITL\\n");
    
    // Disable stdout buffering for correct logging
    setvbuf(stdout, NULL, _IONBF, 0);

    // Initialize FreeRTOS tasks (calls the legacy initialization)
    printf("[SITL] Calling MX_FREERTOS_Init()\\n");
    MX_FREERTOS_Init();
    
    // Start the FreeRTOS POSIX Scheduler
    printf("[SITL] Starting FreeRTOS POSIX Scheduler...\\n");
    vTaskStartScheduler();
    
    // Should never reach here
    printf("[SITL] FATAL: Scheduler exited.\\n");
    return 0;
}

// FreeRTOS hooks
void vApplicationMallocFailedHook(void) {
    printf("[SITL] vApplicationMallocFailedHook called!\\n");
    for(;;);
}

void vApplicationIdleHook(void) {
    // Prevent 100% CPU usage in Linux Idle task by sleeping a tiny bit
    usleep(100);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void)pxTask;
    printf("[SITL] vApplicationStackOverflowHook called! Task: %s\\n", pcTaskName);
    for(;;);
}

void vApplicationTickHook(void) {
}
