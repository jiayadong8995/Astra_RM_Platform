/*
 * FreeRTOS Kernel V11.2.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define portCHAR          char
#define portFLOAT         float
#define portDOUBLE        double
#define portLONG          long
#define portSHORT         short
#define portSTACK_TYPE    uint32_t
#define portBASE_TYPE     long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if ( configUSE_16_BIT_TICKS == 1 )
typedef uint16_t TickType_t;
#define portMAX_DELAY              ( TickType_t ) 0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY              ( TickType_t ) 0xffffffffUL
#define portTICK_TYPE_IS_ATOMIC    1
#endif

#define portSTACK_GROWTH      ( -1 )
#define portTICK_PERIOD_MS    ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT    8
#define portDONT_DISCARD      __attribute__( ( used ) )

#define portYIELD()                                     \
    {                                                   \
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
        __asm volatile ( "dsb" ::: "memory" );          \
        __asm volatile ( "isb" );                       \
    }

#define portNVIC_INT_CTRL_REG     ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT    ( 1UL << 28UL )
#define portVECTACTIVE_MASK       ( 0xFFUL )

#ifndef traceISR_ENTER
#define traceISR_ENTER()
#endif

#ifndef traceISR_EXIT
#define traceISR_EXIT()
#endif

#ifndef traceISR_EXIT_TO_SCHEDULER
#define traceISR_EXIT_TO_SCHEDULER()
#endif

#define portEND_SWITCHING_ISR( xSwitchRequired ) \
    do                                           \
    {                                            \
        if( xSwitchRequired != pdFALSE )         \
        {                                        \
            traceISR_EXIT_TO_SCHEDULER();        \
            portYIELD();                         \
        }                                        \
        else                                     \
        {                                        \
            traceISR_EXIT();                     \
        }                                        \
    } while( 0 )

#define portYIELD_FROM_ISR( x )    portEND_SWITCHING_ISR( x )

extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );

#define portSET_INTERRUPT_MASK_FROM_ISR()         ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( x )    vPortSetBASEPRI( x )
#define portDISABLE_INTERRUPTS()                  vPortRaiseBASEPRI()
#define portENABLE_INTERRUPTS()                   vPortSetBASEPRI( 0 )
#define portENTER_CRITICAL()                      vPortEnterCritical()
#define portEXIT_CRITICAL()                       vPortExitCritical()

#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )    void vFunction( void * pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )          void vFunction( void * pvParameters )

#ifndef portSUPPRESS_TICKS_AND_SLEEP
extern void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );
#define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime )    vPortSuppressTicksAndSleep( xExpectedIdleTime )
#endif

#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    1
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1
__attribute__( ( always_inline ) ) static inline uint8_t ucPortCountLeadingZeros( uint32_t ulBitmap )
{
    uint8_t ucReturn;

    __asm volatile ( "clz %0, %1" : "=r" ( ucReturn ) : "r" ( ulBitmap ) : "memory" );

    return ucReturn;
}

#if ( configMAX_PRIORITIES > 32 )
#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.
#endif

#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )    ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )     ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )
#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )  uxTopPriority = ( 31UL - ( uint32_t ) ucPortCountLeadingZeros( ( uxReadyPriorities ) ) )
#endif

#if ( configASSERT_DEFINED == 1 )
void vPortValidateInterruptPriority( void );
#define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()    vPortValidateInterruptPriority()
#endif

#define portNOP()
#define portINLINE __inline

#ifndef portFORCE_INLINE
#define portFORCE_INLINE inline __attribute__( ( always_inline ) )
#endif

portFORCE_INLINE static BaseType_t xPortIsInsideInterrupt( void )
{
    uint32_t ulCurrentInterrupt;

    __asm volatile ( "mrs %0, ipsr" : "=r" ( ulCurrentInterrupt )::"memory" );

    return ( ulCurrentInterrupt == 0 ) ? pdFALSE : pdTRUE;
}

portFORCE_INLINE static void vPortRaiseBASEPRI( void )
{
    uint32_t ulNewBASEPRI;

    __asm volatile
    (
        "   mov %0, %1                                              \n"
        "   cpsid i                                                 \n"
        "   msr basepri, %0                                         \n"
        "   isb                                                     \n"
        "   dsb                                                     \n"
        "   cpsie i                                                 \n"
        : "=r" ( ulNewBASEPRI ) : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) : "memory"
    );
}

portFORCE_INLINE static uint32_t ulPortRaiseBASEPRI( void )
{
    uint32_t ulOriginalBASEPRI, ulNewBASEPRI;

    __asm volatile
    (
        "   mrs %0, basepri                                         \n"
        "   mov %1, %2                                              \n"
        "   cpsid i                                                 \n"
        "   msr basepri, %1                                         \n"
        "   isb                                                     \n"
        "   dsb                                                     \n"
        "   cpsie i                                                 \n"
        : "=r" ( ulOriginalBASEPRI ), "=r" ( ulNewBASEPRI ) : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) : "memory"
    );

    return ulOriginalBASEPRI;
}

portFORCE_INLINE static void vPortSetBASEPRI( uint32_t ulNewMaskValue )
{
    __asm volatile
    (
        "   msr basepri, %0 " ::"r" ( ulNewMaskValue ) : "memory"
    );
}

#define portMEMORY_BARRIER()    __asm volatile ( "" ::: "memory" )

#ifdef __cplusplus
}
#endif

#endif
