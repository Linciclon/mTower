#ifndef PLAT_H
#define PLAT_H

//com hello world
#define PLAT_MEM_BASE  0x10100000
#define PLAT_MEM_SIZE  0x30000

// com bitcoin wallet
//#define PLAT_MEM_BASE  0x10100000
//#define PLAT_MEM_SIZE  0x50000

#ifdef SINGLE_CORE
#define PLAT_CPU_COUNT 1
#else
#define PLAT_CPU_COUNT 2
#endif
#define STACK_SIZE     0x4000

#define PLAT_GENERIC_TIMER_CNTCTL_BASE  (0x90001100)//(0xaa430000ull)

#define PLAT_GICD_BASE_ADDR (0x94000000) //joao
#define PLAT_GICR_BASE_ADDR (0x94100000) //joao

/* UART/SCI channel used by this RZ/T2M image (valid channels: 0 to 5). */
#ifndef RZT2M_UART_CHANNEL
#define RZT2M_UART_CHANNEL 0
#endif

#if RZT2M_UART_CHANNEL == 0
#define USE_UART0 1
#elif RZT2M_UART_CHANNEL == 1
#define USE_UART1 1
#elif RZT2M_UART_CHANNEL == 2
#define USE_UART2 1
#elif RZT2M_UART_CHANNEL == 3
#define USE_UART3 1
#elif RZT2M_UART_CHANNEL == 4
#define USE_UART4 1
#elif RZT2M_UART_CHANNEL == 5
#define USE_UART5 1
#else
#error "RZT2M_UART_CHANNEL must be between 0 and 5"
#endif

/* RXI0 is SPI 321 and each SCI channel occupies four consecutive SPIs. */
#define UART_IRQ_ID (321U + (4U * RZT2M_UART_CHANNEL))


#endif
