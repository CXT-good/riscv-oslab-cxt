// kernel/trap.c
#include "trap.h"
#include "printf.h"
#include "clock.h"
#include "uart.h"

// static char last_fast_char = '1';
// static char last_medium_char = 'A';
// static char last_slow_char = 'a';

// 最简单的中断初始化
void trap_init(void) {
    extern void trap_vector(void);
    asm volatile("csrw mtvec, %0" : : "r" ((uint64_t)trap_vector));
    printf("Trap: mtvec set\n");
}

void enable_interrupts(void) {
    // 空实现
}

void disable_interrupts(void) {
    // 空实现
}

// 中断处理函数 - 支持多个定时器
void trap_handler(struct trap_context *ctx) {
    uint64_t cause;
    asm volatile("csrr %0, mcause" : "=r" (cause));
    
    if (cause & 0x8000000000000000) {
        // 中断
        int code = cause & 0x7FFFFFFFFFFFFFFF;
        
        if (code == 7) {
            // 时钟中断
            clock_set_next_event();
            
            // 获取三个定时器的当前ticks
            uint64_t fast_ticks = get_ticks(TIMER_FAST);
            uint64_t medium_ticks = get_ticks(TIMER_MEDIUM);
            uint64_t slow_ticks = get_ticks(TIMER_SLOW);
            
            // 快速定时器：输出数字和符号
            if (fast_ticks > 0) {
                if (fast_ticks <= 10) {
                    uart_putc('0' + fast_ticks);
                } else {
                    // 循环输出符号
                    switch ((fast_ticks - 1) % 4) {
                        case 0: uart_putc('+'); break;
                        case 1: uart_putc('-'); break;
                        case 2: uart_putc('*'); break;
                        case 3: uart_putc('/'); break;
                    }
                }
            }
            
            // 中等定时器：输出大写字母
            if (medium_ticks > 0 && medium_ticks % 5 == 0) {
                uart_putc('A' + ((medium_ticks / 5 - 1) % 26));
            }
            
            // 慢速定时器：输出小写字母
            if (slow_ticks > 0 && slow_ticks % 3 == 0) {
                uart_putc('a' + ((slow_ticks / 3 - 1) % 26));
            }
            
        } else {
            // 其他中断 - 忽略
        }
    } else {
        // 异常
        int code = cause;
        printf("\n*** EXCEPTION %d ***\n", code);
        printf("mepc: %p\n", (void*)ctx->mepc);
        printf("Stopping execution.\n");
        
        while(1) {
            asm volatile("wfi");
        }
    }
}