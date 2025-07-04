/**
 * @file        freertos/portable/GCC/ARMV8M/port.c
 * @brief       Provides functionality to start secure world, initialize secure
 *              and normal worlds, pass to execution to normal world.
 *
 * @copyright   Copyright (c) 2019 Samsung Electronics Co., Ltd. All Rights Reserved.
 * @author      Taras Drozdovskyi t.drozdovsky@samsung.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM0 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

// #include "M2351.h"

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL   ( ( volatile uint32_t *) 0xe000e010 )
#define portNVIC_SYSTICK_LOAD   ( ( volatile uint32_t *) 0xe000e014 )
#define portNVIC_INT_CTRL       ( ( volatile uint32_t *) 0xe000ed04 )
#define portNVIC_SYSPRI2        ( ( volatile uint32_t *) 0xe000ed20 )
#define portNVIC_SYSTICK_CLK    0x00000004
#define portNVIC_SYSTICK_INT    0x00000002
#define portNVIC_SYSTICK_ENABLE 0x00000001
#define portNVIC_PENDSVSET      0x10000000
#define portMIN_INTERRUPT_PRIORITY  ( 255UL )
#define portNVIC_PENDSV_PRI     ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI    ( portMIN_INTERRUPT_PRIORITY << 24UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR        ( 0x01000000 )

/* Constants used with memory barrier intrinsics. */
#define portSY_FULL_READ_WRITE  ( 15 )

/* Each task maintains its own interrupt status in the critical nesting
variable. */
static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/*
 * Setup the timer to generate the tick interrupts.
 */
static void prvSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortPendSVHandler(void) __attribute__ (( naked ));
void xPortSysTickHandler(void);
void vPortSVCHandler(void) __attribute__ (( naked ));

uint32_t ulSetInterruptMaskFromISR(void) __attribute__(( naked ));
void vClearInterruptMaskFromISR(uint32_t ulMask) __attribute__(( naked ));

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
static void prvPortStartFirstTask(void) __attribute__ (( naked ));

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/

//#ifndef PLATFORM_LPC55S69
/*
 * See header file for description.
 */
// StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
//     TaskFunction_t pxCode, void *pvParameters)
// {
//   /* Simulate the stack frame as it would be created by a context switch
//    interrupt. */
//   pxTopOfStack--; /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
//   *pxTopOfStack = portINITIAL_XPSR; /* xPSR */
//   pxTopOfStack--;
//   *pxTopOfStack = (StackType_t) pxCode; /* PC */
//   pxTopOfStack--;
//   *pxTopOfStack = (StackType_t) prvTaskExitError; /* LR */
//   pxTopOfStack -= 5; /* R12, R3, R2 and R1. */
//   *pxTopOfStack = (StackType_t) pvParameters; /* R0 */
//   pxTopOfStack -= 8; /* R11..R4. */

//   return pxTopOfStack;
// }
/*-----------------------------------------------------------*/

static void prvTaskExitError(void)
{
  /* A function that implements a task must not exit or attempt to return to
   its caller as there is nothing to return to.  If a task wants to exit it
   should instead call vTaskDelete( NULL ).

   Artificially force an assert() to be triggered if configASSERT() is
   defined, then stop here so application writers can catch the error. */
  configASSERT(uxCriticalNesting == ~0UL);
  portDISABLE_INTERRUPTS();
  for (;;)
    ;
}
/*-----------------------------------------------------------*/

void vPortSVCHandler(void)
{
  /* This function is no longer used, but retained for backward
   compatibility. */
}
/*-----------------------------------------------------------*/

void prvPortStartFirstTask(void)
{
  /* The MSP stack is not reset as, unlike on M3/4 parts, there is no vector
   table offset register that can be used to locate the initial stack value.
   Not all M0 parts have the application vector table at address 0. */
  __asm volatile(
      " ldr	r3, pxCurrentTCBConst2  \n" /* Obtain location of pxCurrentTCB. */
      " ldr r1, [r3]      \n"
      " ldr r0, [r1]      \n" /* The first item in pxCurrentTCB is the task top of stack. */
      " add r0, #32       \n" /* Discard everything up to r0. */
      " msr psp, r0       \n" /* This is now the new top of stack to use in the task. */
      " movs r0, #2       \n" /* Switch to the psp stack. */
      " msr CONTROL, r0   \n"
      " isb               \n"
      " pop {r0-r5}       \n" /* Pop the registers that are saved automatically. */
      " mov lr, r5        \n" /* lr is now in r5. */
      " pop {r3}          \n" /* Return address is now in r3. */
      " pop {r2}          \n" /* Pop and discard XPSR. */
      " cpsie i           \n" /* The first task has its context and interrupts can be enabled. */
      " bx r3             \n" /* Finally, jump to the user defined task code. */
      "                   \n"
      " .align 4          \n"
      "pxCurrentTCBConst2: .word pxCurrentTCB "
  );
}
/*-----------------------------------------------------------*/

// #ifndef PLATFORM_LPC55S69
// /*
//  * See header file for description.
//  */
// BaseType_t xPortStartScheduler(void)
// {
//   /* Make PendSV, CallSV and SysTick the same priroity as the kernel. */
//   *(portNVIC_SYSPRI2) |= portNVIC_PENDSV_PRI;
//   *(portNVIC_SYSPRI2) |= portNVIC_SYSTICK_PRI;

//   /* Start the timer that generates the tick ISR.  Interrupts are disabled
//    here already. */
//   prvSetupTimerInterrupt();

//   /* Initialise the critical nesting count ready for the first task. */
//   uxCriticalNesting = 0;

//   /* Start the first task. */
//   prvPortStartFirstTask();

//   /* Should not get here! */
//   return 0;
// }
// /*-----------------------------------------------------------*/
// #endif

//#ifndef PLATFORM_LPC55S69
// void vPortEndScheduler(void)
// {
//   /* Not implemented in ports where there is nothing to return to.
//    Artificially force an assert. */
//   configASSERT(uxCriticalNesting == 1000UL);
// }
/*-----------------------------------------------------------*/

//#ifndef PLATFORM_LPC55S69
// void vPortYield(void)
// {
//   /* Set a PendSV to request a context switch. */
//   *( portNVIC_INT_CTRL) = portNVIC_PENDSVSET;

//   /* Barriers are normally not required but do ensure the code is completely
//    within the specified behaviour for the architecture. */

//   __asm volatile( "dsb" ::: "memory" );
//   __asm volatile( "isb" );

// }
/*-----------------------------------------------------------*/

//#ifndef PLATFORM_LPC55S69
// void vPortEnterCritical(void)
// {
//   portDISABLE_INTERRUPTS();
//   uxCriticalNesting++;
//   __asm volatile( "dsb" ::: "memory" );
//   __asm volatile( "isb" );
// }
/*-----------------------------------------------------------*/

//#ifndef PLATFORM_LPC55S69
// void vPortExitCritical( void )
// {
// 	configASSERT( uxCriticalNesting );
//     uxCriticalNesting--;
//     if( uxCriticalNesting == 0 )
//     {
//         portENABLE_INTERRUPTS();
//     }
// }
/*-----------------------------------------------------------*/
uint32_t ulSetInterruptMaskFromISR(void)
{
  __asm volatile(
      " mrs r0, PRIMASK   \n"
      " cpsid i           \n"
      " bx lr             \n"
      ::: "memory"
  );

  /* To avoid compiler warnings.  This line will never be reached. */
  return 0;
}

/*-----------------------------------------------------------*/

void vClearInterruptMaskFromISR(uint32_t ulMask)
{
  (void) ulMask;
  __asm volatile(
      " msr PRIMASK, r0 \n"
      " bx lr           \n"
      ::: "memory"
  );
}

/*-----------------------------------------------------------*/

//".syntax unified\n"
//        "__delay:\n"
//        "subs r1, r1, #1\n"
//        "bhi __delay\n"
//        ".syntax divided");

void xPortPendSVHandler(void)
{
  /* This is a naked function. */
  __asm volatile
  (
      " .align 8              \n"
      " mrs r0, psp           \n"
      "                       \n"
      " ldr r3, pxCurrentTCBConst \n" /* Get the location of the current TCB. */
      " ldr	r2, [r3]          \n"
      "                       \n"
      " sub r0, r0, #32       \n" /* Make space for the remaining low registers. ???*/
      " str r0, [r2]          \n" /* Save the new top of stack. */
      " stmia r0!, {r4-r7}    \n" /* Store the low registers that are not saved automatically. */
      " mov r4, r8            \n" /* Store the high registers. */
      " mov r5, r9            \n"
      " mov r6, r10           \n"
      " mov r7, r11           \n"
      " stmia r0!, {r4-r7}    \n"
      "                       \n"
      " push {r3, r14}        \n"
      " cpsid i               \n"
      " bl vTaskSwitchContext \n"
      " cpsie i               \n"
      " pop {r2, r3}          \n" /* lr goes in r3. r2 now holds tcb pointer. */
      "                       \n"
      " ldr r1, [r2]          \n"
      " ldr r0, [r1]          \n" /* The first item in pxCurrentTCB is the task top of stack. */
      " add r0, r0, #16       \n" /* Move to the high registers. ???*/
      " ldmia r0!, {r4-r7}    \n" /* Pop the high registers. */
      " mov r8, r4            \n"
      " mov r9, r5            \n"
      " mov r10, r6           \n"
      " mov r11, r7           \n"
      "                       \n"
      " msr psp, r0           \n" /* Remember the new top of stack for the task. */
      "                       \n"
      " sub r0, r0, #32       \n" /* Go back for the low registers that are not automatically restored. ???*/
      " ldmia r0!, {r4-r7}    \n" /* Pop low registers.  */
      "                       \n"
      " bx r3                 \n"
      "                       \n"
      " .align 4              \n"
      "pxCurrentTCBConst: .word pxCurrentTCB  "
  );

}

/*-----------------------------------------------------------*/

void xPortSysTickHandler(void)
{
  uint32_t ulPreviousMask;
  ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
  {
    /* Increment the RTOS tick. */
    if (xTaskIncrementTick() != pdFALSE) {
      /* Pend a context switch. */
      *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
    }
  }
  portCLEAR_INTERRUPT_MASK_FROM_ISR(ulPreviousMask);
}
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void prvSetupTimerInterrupt(void)
{
  /* Configure SysTick to interrupt at the requested rate. */
  *(portNVIC_SYSTICK_LOAD) = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
  *(portNVIC_SYSTICK_CTRL) = portNVIC_SYSTICK_CLK | portNVIC_SYSTICK_INT
      | portNVIC_SYSTICK_ENABLE;
}
/*-----------------------------------------------------------*/

