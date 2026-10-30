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

#define UART_IRQ_ID 321//305+32
//Channel 0 321 //289+32
//Channel 2 
//Channel 3 
//Channel 4 337 //305+32


#endif
