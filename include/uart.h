// include/uart.h
#ifndef _UART_H_
#define _UART_H_

// UART 函数声明
void uart_init(void);
void uart_putc(char c);
void uart_puts(char *s);
int uart_input_available(void);
char uart_getc(void);

#endif