#ifndef PLAT_H
#define PLAT_H

#include <sysregs.h>
#include <iocon.h>
#include <syscon.h>

#define PLAT_NON_UNIFIED_MEM

#ifdef RUN_INSIDE_VM
#define PLAT_CODE_MEM_BASE  0x10000
#define PLAT_CODE_MEM_SIZE  0x35000

#define PLAT_MEM_BASE       0x20010000
#define PLAT_MEM_SIZE       0x10000
#else
/* Secure aliases used when mTower owns the processor natively. */
#define PLAT_CODE_MEM_BASE  0x10000000
#define PLAT_CODE_MEM_SIZE  0x50000

#define PLAT_MEM_BASE       0x30010000
#define PLAT_MEM_SIZE       0x10000

#define PLAT_NS_CODE_MEM_BASE     0x00050000
#define PLAT_NS_CODE_MEM_SIZE     0x00025000
#define PLAT_NS_VECTOR_TABLE_BASE 0x00053000UL //this is the vector table off non-secure which is located at .lst file .text+size of text 
#define PLAT_NS_MEM_BASE          0x20000000
#define PLAT_NS_MEM_SIZE          0x00050000
#define PLAT_NS_PERIPH_BASE       0x40000000
#define PLAT_NS_PERIPH_SIZE       0x10000000
#define PLAT_NS_FRO_BASE       0x9FC00u /**< 192MHz Free Running OScillator (FRO) Control register, offset: 0x10 */
#define PLAT_NS_FRO_SIZE       0x400 /**< 192MHz Free Running OScillator (FRO) Control register, offset: 0x10 */

/* Dedicated minimum-size SAU region for the generated ioctl SG veneer. */
#define PLAT_NSC_BASE       0x10000000
#define PLAT_NSC_SIZE       0x20
#endif

#define PLAT_UART_ADDR      0x40088000UL
#define UART_NVIC_IRQ_ID    16
#define UART_IRQ_ID         EXT_INT_BASE+UART_NVIC_IRQ_ID

#define UART_RX_PORT        port1
#define UART_RX_PIN         pin24

#define UART_TX_PORT        port0
#define UART_TX_PIN         pin27

#define FCRST               FC2_RST
#define FCCLKSEL            FCCLKSEL2
#define SYSCON_AHBCLKTRL1   SYSCON_AHBCLKTRL1_FC2

// #elif GUEST1
// #define PLAT_CODE_MEM_BASE  0x40000
// #define PLAT_CODE_MEM_SIZE  0x10000

// #define PLAT_MEM_BASE       0x20030000
// #define PLAT_MEM_SIZE       0x8000

// #define PLAT_UART_ADDR      0x40089000UL
// #define UART_NVIC_IRQ_ID    17
// #define UART_IRQ_ID         EXT_INT_BASE+UART_NVIC_IRQ_ID

// #define UART_RX_PORT        port0
// #define UART_RX_PIN         pin03

// #define UART_TX_PORT        port0
// #define UART_TX_PIN         pin02

// #define FCRST               FC3_RST
// #define FCCLKSEL            FCCLKSEL3
// #define SYSCON_AHBCLKTRL1   SYSCON_AHBCLKTRL1_FC3

//#endif

#define STACK_SIZE          0x8000

#define PLAT_TIMER_FREQ     12000000ULL

#define PLAT_MAX_INTERRUPTS 75

#endif
