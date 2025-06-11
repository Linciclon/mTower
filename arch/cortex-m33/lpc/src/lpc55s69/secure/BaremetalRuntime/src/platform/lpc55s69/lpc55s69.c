#include <plat.h>
#include <lpc_uart.h>
#include <nvic.h>
#include <sysregs.h>

struct fc_uart *uart  = (void*) PLAT_UART_ADDR;

void uart_init(void)
{
    lpc_uart_init(uart);
    lpc_uart_enable(uart);

    return;
}

void uart_putc(char c)
{
    lpc_uart_putc(uart, c);
}

char uart_getchar(void)
{
    return lpc_uart_getc(uart);
}

void uart_enable_rxirq()
{
    lpc_uart_enable_rxirq(uart);
}

void uart_clear_rxirq()
{
    lpc_uart_clear_rxirq(uart);
}
