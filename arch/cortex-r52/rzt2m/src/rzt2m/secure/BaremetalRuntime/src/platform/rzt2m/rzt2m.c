#include <rzt2m_uart.h>
#include <plat.h>

#if defined(USE_UART0)
#define RZT2M_UART_BASE 0x80001000U
#elif defined(USE_UART1)
#define RZT2M_UART_BASE 0x80001400U
#elif defined(USE_UART2)
#define RZT2M_UART_BASE 0x80001800U
#elif defined(USE_UART3)
#define RZT2M_UART_BASE 0x80001C00U
#elif defined(USE_UART4)
#define RZT2M_UART_BASE 0x80002000U
#elif defined(USE_UART5)
#define RZT2M_UART_BASE 0x81001000U
#else
#error "No supported RZ/T2M UART was selected"
#endif

/* Keep the MMIO pointer observable in debug builds while preserving volatile
 * accesses to the peripheral registers. */
static volatile R_SCI0_Type * volatile uart __attribute__((used)) =
    (volatile R_SCI0_Type *)RZT2M_UART_BASE;

void uart_init(void)
{
    rzt2m_uart_init(uart);
    rzt2m_uart_enable(uart);

    return;
}

void uart_putc(char c)
{
    rzt2m_uart_putc(uart, c);
}

char uart_getchar(void)
{
    return rzt2m_uart_getc(uart);
}

void uart_enable_rxirq(void){
    rzt2m_uart_enable_rxirq(uart);
}

void uart_clear_platform(void){
    renesas_clear_uart(uart);
}
void uart_clear_rxirq(void){
    /*while(!(uart->flag & UART_FR_RXFE)) {
        volatile char c = uart->data;
    }
    uart->isr_clear = 0xffff;*/
    (void)rzt2m_uart_getc(uart);
}


