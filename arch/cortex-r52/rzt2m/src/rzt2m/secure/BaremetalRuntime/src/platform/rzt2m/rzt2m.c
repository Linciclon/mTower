#include <rzt2m_uart.h>

/* Keep the MMIO pointer observable in debug builds while preserving volatile
 * accesses to the peripheral registers. */
static volatile R_SCI0_Type * volatile uart __attribute__((used)) =
    (volatile R_SCI0_Type *)0x80001000;
//Channel 0 0x80001000
//Channel 2 0x80001800
//Channel 3 0x80001c00
//Channel 4 0x80002000

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


