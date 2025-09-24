// 在kernel/main.c开头添加
typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

// External symbols from linker script
extern void _bss_start;
extern void _bss_end;

void uart_init(void);
void uart_puts(char *s);

// Main function
void main(void)
{
    // Initialize UART
    uart_init();
    
    // Print welcome message
    uart_puts("Hello OS\n");
    uart_puts("Minimal RISC-V OS booted successfully!\n");
    
    // Infinite loop
    while (1) {
        // Do nothing or add simple heartbeat
    }
}