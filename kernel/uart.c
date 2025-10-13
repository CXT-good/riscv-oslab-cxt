#include "types.h"

#define UART_BASE 0x10000000UL

// UART 寄存器定义
#define THR 0    // 发送保持寄存器 (Transmit Holding Register)
#define LSR 5    // 线路状态寄存器 (Line Status Register)
#define LSR_TX_IDLE (1 << 5)  // THR 为空，准备好发送下一个字符

// 从 UART 寄存器读取
static inline unsigned char uart_read_reg(int reg)
{
    volatile unsigned char *addr = (volatile unsigned char *)(UART_BASE + reg);
    return *addr;
}

// 写入 UART 寄存器
static inline void uart_write_reg(int reg, unsigned char val)
{
    volatile unsigned char *addr = (volatile unsigned char *)(UART_BASE + reg);
    *addr = val;
}

// 输出单个字符
// uart_read_reg(LSR)：读取线路状态寄存器(LSR)的值
// LSR_TX_IDLE：定义为(1 << 5)，即第5位（从0开始计数）
// 第5位是THRE（Transmitter Holding Register Empty）标志位：
// 1 = 发送保持寄存器为空，可以接收新数据
// 0 = 发送保持寄存器有数据，正在发送中
void uart_putc(char c)
{
    // 等待直到 UART 准备好发送
    while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0)
        ;
    
    // 发送字符，硬件会自动从THR读取数据并通过串口发送出去
    uart_write_reg(THR, c);
    
    // 如果是换行符，同时发送回车符（可选）
    if (c == '\n') {
        while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0)
            ;
        uart_write_reg(THR, '\r');
    }
}

// 输出字符串
void uart_puts(char *s)
{
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

// 简单的 UART 初始化
void uart_init(void)
{
    // QEMU virt 机器的 UART 已经初始化完成
    // 此函数保留用于未来扩展
}