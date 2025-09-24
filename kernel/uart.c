#define UART_BASE 0x10000000UL  // 添加UL后缀表示unsigned long

// UART registers
#define THR 0    // Transmit Holding Register
#define LSR 5    // Line Status Register
#define LSR_TX_IDLE (1 << 5)  // THR empty, ready for next character

// Read from UART register
static inline unsigned char uart_read_reg(int reg)
{
    volatile unsigned char *addr = (volatile unsigned char *)(UART_BASE + reg);
    return *addr;
}

// Write to UART register
static inline void uart_write_reg(int reg, unsigned char val)
{
    volatile unsigned char *addr = (volatile unsigned char *)(UART_BASE + reg);
    *addr = val;
}

// Output a single character
void uart_putc(char c)
{
    // Wait until UART is ready to transmit
    while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0)
        ;
    
    // Send the character
    uart_write_reg(THR, c);
}

// Output a string
void uart_puts(char *s)
{
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

// Simple UART initialization (minimal setup)
void uart_init(void)
{
    // For minimal setup, we rely on QEMU's default UART configuration
    // Just ensure we can output characters
}