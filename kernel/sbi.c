
#include "types.h"
#include "printf.h"

// 简单的SBI调用实现
void sbi_set_timer(uint64_t stime_value) {
    #ifdef __riscv
    // 直接写入timecmp寄存器
    asm volatile (
        "csrw 0xc41, %0"
        : 
        : "r" (stime_value)
    );
    #endif
}

// SBI控制台输出
void sbi_console_putchar(char ch) {
    #ifdef __riscv
    // 使用SBI控制台调用
    register long a0 asm("a0") = ch;
    asm volatile (
        "li a7, 1\n"  // SBI_CONSOLE_PUTCHAR
        "ecall"
        : 
        : "r" (a0)
        : "a7"
    );
    #endif
}
