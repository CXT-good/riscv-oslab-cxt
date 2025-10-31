// kernel/uart.c
#include "types.h"
#include "uart.h"  // 添加这行

#define UART_BASE 0x10000000UL

// UART 寄存器定义
#define RHR 0    // 接收保持寄存器 (Receive Holding Register)
#define THR 0    // 发送保持寄存器 (Transmit Holding Register)  
#define IER 1    // 中断使能寄存器 (Interrupt Enable Register)
#define FCR 2    // FIFO控制寄存器 (FIFO Control Register)
#define LCR 3    // 线路控制寄存器 (Line Control Register)
#define LSR 5    // 线路状态寄存器 (Line Status Register)
#define LSR_RX_READY (1 << 0)  // 数据就绪
#define LSR_TX_IDLE (1 << 5)   // 发送器空闲

// 从 UART 寄存器读取
static inline unsigned char uart_read_reg(int reg) {
    volatile unsigned char *addr = (volatile unsigned char *)(UART_BASE + reg);
    return *addr;
}

// 写入 UART 寄存器  
static inline void uart_write_reg(int reg, unsigned char val) {
    volatile unsigned char *addr = (volatile unsigned char *)(UART_BASE + reg);
    *addr = val;
}

// 检查是否有输入字符
int uart_input_available(void) {
    return (uart_read_reg(LSR) & LSR_RX_READY) != 0;
}

// 读取输入字符
char uart_getc(void) {
    while (!uart_input_available())
        ; // 等待直到有数据
    return uart_read_reg(RHR);
}

// 输出单个字符
void uart_putc(char c) {
    // 等待直到 UART 准备好发送
    while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0)
        ;
    
    // 发送字符
    uart_write_reg(THR, c);
    
    // 如果是换行符，同时发送回车符
    if (c == '\n') {
        while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0)
            ;
        uart_write_reg(THR, '\r');
    }
}

// 输出字符串
void uart_puts(char *s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

// UART 初始化
void uart_init(void) {
    // 禁用中断
    uart_write_reg(IER, 0x00);
    
    // 设置波特率 (DLAB = 1)
    uart_write_reg(LCR, 0x80);
    uart_write_reg(0, 0x03);  // 38400 baud
    uart_write_reg(1, 0x00);
    
    // 8N1模式, 清除DLAB
    uart_write_reg(LCR, 0x03);
    
    // 启用FIFO
    uart_write_reg(FCR, 0x01);
}