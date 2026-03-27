#include "bsp_dwt.h"
#include <time.h>

DWT_Time_t SysTime;

void DWT_Init(uint32_t CPU_Freq_mHz) {
    (void)CPU_Freq_mHz;
}

float DWT_GetDeltaT(uint32_t *cnt_last) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint32_t current_cnt = (uint32_t)(ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
    
    if (cnt_last == NULL) return 0.0f;
    
    float dt = (current_cnt - *cnt_last) / 1000000.0f;
    *cnt_last = current_cnt;
    return dt;
}

double DWT_GetDeltaT64(uint32_t *cnt_last) {
    return (double)DWT_GetDeltaT(cnt_last);
}

float DWT_GetTimeline_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (float)ts.tv_sec + (float)ts.tv_nsec / 1000000000.0f;
}

float DWT_GetTimeline_ms(void) {
    return DWT_GetTimeline_s() * 1000.0f;
}

uint64_t DWT_GetTimeline_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

void DWT_Delay(float Delay) {
    // Implement using POSIX sleep
    struct timespec ts;
    ts.tv_sec = (time_t)Delay;
    ts.tv_nsec = (long)((Delay - ts.tv_sec) * 1000000000.0f);
    nanosleep(&ts, NULL);
}

void DWT_SysTimeUpdate(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    SysTime.s = ts.tv_sec;
    SysTime.ms = ts.tv_nsec / 1000000;
    SysTime.us = (ts.tv_nsec % 1000000) / 1000;
}
