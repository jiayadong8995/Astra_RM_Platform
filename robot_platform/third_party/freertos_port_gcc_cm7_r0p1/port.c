/*
 * FreeRTOS Kernel V11.2.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*-----------------------------------------------------------
* Implementation of functions defined in portable.h for the ARM CM7 port.
*----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#ifndef __ARM_FP
    #error This port can only be used when the project options are configured to enable hardware floating point support.
#endif

/* Prototype of all Interrupt Service Routines (ISRs). */
typedef void ( * portISR_t )( void );

/* Constants required to manipulate the core.  Registers first... */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SHPR2_REG                    ( *( ( volatile uint32_t * ) 0xe000ed1c ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
/* ...then bits in the registers. */
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portNVIC_PENDSVCLEAR_BIT              ( 1UL << 27UL )
#define portNVIC_PEND_SYSTICK_SET_BIT         ( 1UL << 26UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT       ( 1UL << 25UL )

#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( ( ( uint32_t ) portMIN_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI                  ( ( ( uint32_t ) portMIN_INTERRUPT_PRIORITY ) << 24UL )
#define portNVIC_SVC_PRI                      ( ( ( uint32_t ) portMIN_INTERRUPT_PRIORITY ) << 24UL )
#define portAIRCR_REG                         ( *( ( volatile uint32_t * ) 0xE000ED0C ) )
#define portMAX_8_BIT_VALUE                   ( ( uint8_t ) 0xff )
#define portTOP_BIT_OF_BYTE                   ( ( uint8_t ) 0x80 )
#define portMAX_PRIGROUP_BITS                 ( ( uint8_t ) 7 )
#define portPRIORITY_GROUP_MASK               ( 0x07UL << 8UL )
#define portPRIGROUP_SHIFT                    ( 8UL )

#define portFPCCR                             ( ( volatile uint32_t * ) 0xe000ef34 ) /* Floating point context control register. */
#define portASPEN_AND_LSPEN_BITS              ( 0x3UL << 30UL )

#define portSCB_VTOR_REG                      ( *( ( portISR_t ** ) 0xE000ED08 ) )
#define portVECTOR_INDEX_SVC                  ( 11 )
#define portVECTOR_INDEX_PENDSV               ( 14 )

#ifndef configSYSTICK_CLOCK_HZ
    #define configSYSTICK_CLOCK_HZ            configCPU_CLOCK_HZ
    #define portNVIC_SYSTICK_CLK_BIT_CONFIG   portNVIC_SYSTICK_CLK_BIT
#else
    #define portNVIC_SYSTICK_CLK_BIT_CONFIG   0
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1
    /* Check the configuration. */
    #if ( configMAX_PRIORITIES > 32 )
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 different priorities as tasks that share a priority will time slice.
    #endif
#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/* Let the user override the pre-loading used to set the floating point status
 * register. */
#ifndef portINITIAL_FPSCR
    #define portINITIAL_FPSCR    ( 0 )
#endif

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR                      ( 0x01000000 )
#define portINITIAL_EXC_RETURN               ( 0xfffffffd )

/* For strict compliance with the Cortex-M spec the task start address should
 * have bit-0 clear, as it is loaded into the PC on exit from an ISR. */
#define portSTART_ADDRESS_MASK               ( ( StackType_t ) 0xfffffffeUL )

/* For strict compliance with the Cortex-M spec the task stack pointer must be
 * 8-byte aligned. */
#define portPRELOAD_REGISTERS                1

/* The MSB of the BASEPRI register is used to tell the PendSV handler whether
 * there is a critical section nesting count to restore. */
#define portNO_CRITICAL_NESTING              ( ( uint32_t ) 0 )

/*-----------------------------------------------------------*/

/* Each task maintains its own interrupt status in the critical nesting
 * variable. */
static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/* Set to 1 to pend a context switch from an ISR. */
static BaseType_t xYieldPendings = pdFALSE;

/* Used to detect the number of priority bits available in hardware. */
static uint8_t ucMaxSysCallPriority = 0;

/* If the hardware implements 8 priority bits, then the low 8 bits of
 * configMAX_SYSCALL_INTERRUPT_PRIORITY can be used as the mask. */
static const volatile uint8_t * const pcInterruptPriorityRegisters = ( uint8_t * ) 0xe000e400;

/* The priority grouping setting as set by the application. */
static uint32_t ulMaxPRIGROUPValue = 0;

/*-----------------------------------------------------------*/

void vPortSetupTimerInterrupt( void );

static void prvTaskExitError( void );

/* Externally referenced functions. */
void xPortPendSVHandler( void ) __attribute__( ( naked ) );
void xPortSysTickHandler( void );
void vPortSVCHandler( void ) __attribute__( ( naked ) );

/*-----------------------------------------------------------*/

static void prvPortStartFirstTask( void ) __attribute__( ( naked ) );
static void vPortEnableVFP( void ) __attribute__( ( naked ) );
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/

#if ( configENABLE_ERRATA_837070_WORKAROUND == 1 )
    static BaseType_t xRegionUsesTightCoupledMemory = pdFALSE;
#endif

/*-----------------------------------------------------------*/

__attribute__( ( weak ) ) void vApplicationFPUSafeIRQHandler( void )
{
    /* A weak implementation is provided to avoid linker errors if the symbol is
     * referenced but no application handler is provided. */
}

/*-----------------------------------------------------------*/

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters )
{
    /* Simulate the stack frame as it would be created by a context switch interrupt. */

    pxTopOfStack--;
    *pxTopOfStack = portINITIAL_XPSR;                                    /* xPSR */
    pxTopOfStack--;
    *pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK; /* PC */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) prvTaskExitError;                    /* LR */

    /* Save code space by skipping register initialisation. */
    pxTopOfStack -= 5;                           /* R12, R3, R2 and R1. */
    *pxTopOfStack = ( StackType_t ) pvParameters; /* R0 */

    /* A save method is being used that requires each task to maintain its
     * own exec return value. */
    pxTopOfStack--;
    *pxTopOfStack = portINITIAL_EXC_RETURN;

    pxTopOfStack -= 8; /* R11, R10, R9, R8, R7, R6, R5 and R4. */

    return pxTopOfStack;
}

/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
    volatile uint32_t ulDummy = 0;

    configASSERT( uxCriticalNesting == ~0UL );

    portDISABLE_INTERRUPTS();

    while( ulDummy == 0 )
    {
    }
}

/*-----------------------------------------------------------*/

void vPortSVCHandler( void )
{
    __asm volatile (
        "   ldr r3, =pxCurrentTCB           \n"
        "   ldr r1, [r3]                    \n"
        "   ldr r0, [r1]                    \n"
        "   ldmia r0!, {r4-r11, r14}        \n"
        "   msr psp, r0                     \n"
        "   isb                             \n"
        "   mov r0, #0                      \n"
        "   msr basepri, r0                 \n"
        "   bx r14                          \n"
        "   .ltorg                          \n"
        );
}

/*-----------------------------------------------------------*/

static void prvPortStartFirstTask( void )
{
    __asm volatile (
        " ldr r0, =0xE000ED08   \n"
        " ldr r0, [r0]          \n"
        " ldr r0, [r0]          \n"
        " msr msp, r0           \n"
        " mov r0, #0            \n"
        " msr control, r0       \n"
        " cpsie i               \n"
        " cpsie f               \n"
        " dsb                   \n"
        " isb                   \n"
        " svc 0                 \n"
        " nop                   \n"
        " .ltorg                \n"
        );
}

/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    #if ( configASSERT_DEFINED == 1 )
    {
        const portISR_t * const pxVectorTable = portSCB_VTOR_REG;
        configASSERT( pxVectorTable[ portVECTOR_INDEX_SVC ] == vPortSVCHandler );
        configASSERT( pxVectorTable[ portVECTOR_INDEX_PENDSV ] == xPortPendSVHandler );
    }
    #endif

    #if ( configASSERT_DEFINED == 1 )
    {
        volatile uint8_t ucOriginalPriority;
        volatile uint32_t ulImplementedPrioBits = 0;
        volatile uint8_t * const pucFirstUserPriorityRegister = ( volatile uint8_t * const ) ( pcInterruptPriorityRegisters + 16 );
        volatile uint8_t ucMaxPriorityValue;

        ucOriginalPriority = *pucFirstUserPriorityRegister;
        *pucFirstUserPriorityRegister = portMAX_8_BIT_VALUE;
        ucMaxPriorityValue = *pucFirstUserPriorityRegister;
        ucMaxSysCallPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY & ucMaxPriorityValue;

        configASSERT( ucMaxSysCallPriority );
        configASSERT( ( configMAX_SYSCALL_INTERRUPT_PRIORITY & ( ~ucMaxPriorityValue ) ) == 0U );

        while( ( ucMaxPriorityValue & portTOP_BIT_OF_BYTE ) == portTOP_BIT_OF_BYTE )
        {
            ulImplementedPrioBits++;
            ucMaxPriorityValue <<= ( uint8_t ) 1;
        }

        if( ulImplementedPrioBits == 8 )
        {
            configASSERT( ( configMAX_SYSCALL_INTERRUPT_PRIORITY & 0x1U ) == 0U );
            ulMaxPRIGROUPValue = 0;
        }
        else
        {
            ulMaxPRIGROUPValue = portMAX_PRIGROUP_BITS - ulImplementedPrioBits;
        }

        *pucFirstUserPriorityRegister = ucOriginalPriority;
    }
    #endif

    portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;
    portNVIC_SHPR2_REG |= portNVIC_SVC_PRI;

    vPortSetupTimerInterrupt();
    uxCriticalNesting = 0;
    vPortEnableVFP();
    *( portFPCCR ) |= portASPEN_AND_LSPEN_BITS;
    prvPortStartFirstTask();

    return 0;
}

/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    configASSERT( uxCriticalNesting == 1000UL );
}

/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;

    if( uxCriticalNesting == 1 )
    {
        configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
    }
}

/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
    configASSERT( uxCriticalNesting );
    uxCriticalNesting--;

    if( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
    }
}

/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
    __asm volatile
    (
        "   mrs r0, psp                         \n"
        "   isb                                 \n"
        "   ldr r3, =pxCurrentTCB               \n"
        "   ldr r2, [r3]                        \n"
        "   tst r14, #0x10                      \n"
        "   it eq                               \n"
        "   vstmdbeq r0!, {s16-s31}             \n"
        "   stmdb r0!, {r4-r11, r14}            \n"
        "   str r0, [r2]                        \n"
        "   stmdb sp!, {r0, r3}                 \n"
        "   mov r0, %0                          \n"
        "   cpsid i                             \n"
        "   msr basepri, r0                     \n"
        "   dsb                                 \n"
        "   isb                                 \n"
        "   cpsie i                             \n"
        "   bl vTaskSwitchContext               \n"
        "   mov r0, #0                          \n"
        "   msr basepri, r0                     \n"
        "   ldmia sp!, {r0, r3}                 \n"
        "   ldr r1, [r3]                        \n"
        "   ldr r0, [r1]                        \n"
        "   ldmia r0!, {r4-r11, r14}            \n"
        "   tst r14, #0x10                      \n"
        "   it eq                               \n"
        "   vldmiaeq r0!, {s16-s31}             \n"
        "   msr psp, r0                         \n"
        "   isb                                 \n"
        "   bx r14                              \n"
        "   .ltorg                              \n"
        ::"i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY )
    );
}

/*-----------------------------------------------------------*/

void xPortSysTickHandler( void )
{
    portDISABLE_INTERRUPTS();
    traceISR_ENTER();
    {
        if( xTaskIncrementTick() != pdFALSE )
        {
            traceISR_EXIT_TO_SCHEDULER();
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
        }
        else
        {
            traceISR_EXIT();
        }
    }
    portENABLE_INTERRUPTS();
}

/*-----------------------------------------------------------*/

#if ( configUSE_TICKLESS_IDLE == 1 )
    __attribute__( ( weak ) ) void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
    {
        ( void ) xExpectedIdleTime;
    }
#endif

/*-----------------------------------------------------------*/

__attribute__( ( weak ) ) void vPortSetupTimerInterrupt( void )
{
    portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
    portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
    portNVIC_SYSTICK_CTRL_REG = portNVIC_SYSTICK_CLK_BIT_CONFIG |
                                portNVIC_SYSTICK_INT_BIT |
                                portNVIC_SYSTICK_ENABLE_BIT;
}

/*-----------------------------------------------------------*/

static void vPortEnableVFP( void )
{
    __asm volatile
    (
        "   ldr.w r0, =0xE000ED88       \n"
        "   ldr r1, [r0]                \n"
        "   orr r1, r1, #( 0xf << 20 )  \n"
        "   str r1, [r0]                \n"
        "   bx r14                      \n"
        "   .ltorg                      \n"
    );
}

/*-----------------------------------------------------------*/

#if ( configASSERT_DEFINED == 1 )
    void vPortValidateInterruptPriority( void )
    {
        uint32_t ulCurrentInterrupt;
        uint8_t ucCurrentPriority;

        __asm volatile ( "mrs %0, ipsr" : "=r" ( ulCurrentInterrupt )::"memory" );

        if( ulCurrentInterrupt >= 16 )
        {
            ucCurrentPriority = pcInterruptPriorityRegisters[ ulCurrentInterrupt ];
            configASSERT( ucCurrentPriority >= ucMaxSysCallPriority );
            configASSERT( ( portAIRCR_REG & portPRIORITY_GROUP_MASK ) <= ulMaxPRIGROUPValue );
        }
    }
#endif
