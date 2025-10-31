// 修改 main.c

#include "printf.h"
#include "console.h"
#include "clock.h"
#include "clint.h"
#include "trap.h"  // 添加trap头文件

// 定义可调节的时钟间隔
#define INTERVAL_SLOW   2000000  // 2秒
#define INTERVAL_FAST   500000   // 0.5秒
#define INTERVAL_NORMAL 1000000  // 1秒

static uint64_t current_interval = INTERVAL_NORMAL;

// 时钟间隔调节测试
void clock_interval_test(void) {
    printf("\n=== CLOCK INTERVAL TEST ===\n");
    printf("Current interval: %d cycles\n", (int)current_interval);
    printf("Ticks output speed test - observe 'T' character frequency\n\n");
}

// UART输入响应测试
void uart_input_test(void) {
    printf("\n=== UART INPUT TEST ===\n");
    printf("Please type some characters on your keyboard...\n");
    printf("If the system is working, you should see what you type.\n");
    printf("Press Enter to start typing test:\n");
}

// 修改后的主函数
int main(void) {
    // 初始化控制台
    console_init();
    
    printf("\n");
    printf("RV64 OS Booted Successfully!\n");
    printf("Interrupt and Clock Management Test\n");
    
    // 初始化中断系统
    trap_init();
    
    // 初始化定时器
    timer_init();
    
    // 测试1: 时钟滴答测试
    printf("\n=== TEST 1: CLOCK TICK TEST ===\n");
    printf("You should see 'T' characters appearing periodically\n");
    
    // 测试2: 时钟快慢测试
    clock_interval_test();
    
    // 测试3: UART输入测试
    uart_input_test();
    
    printf("\n=== ALL TESTS STARTED ===\n");
    printf("Observe:\n");
    printf("1. 'T' characters for clock ticks\n");
    printf("2. ticks count every 10 interrupts\n");
    printf("3. Keyboard input echo\n\n");
    
    // 启用全局中断
    asm volatile("csrs sstatus, %0" : : "r"(1 << 1)); // 启用S-mode中断
    
    // 主循环 - 处理输入和显示状态
    uint64_t last_display = 0;
    int test_phase = 0;
    
    while (1) {
        uint64_t current_time = clint_get_time();
        
        // 每5秒切换一次时钟间隔进行测试
        if (current_time - last_display > 5000000) {
            last_display = current_time;
            test_phase++;
            
            switch (test_phase % 3) {
                case 0:
                    current_interval = INTERVAL_NORMAL;
                    printf("\n[Clock] Interval set to NORMAL\n");
                    break;
                case 1:
                    current_interval = INTERVAL_SLOW;
                    printf("\n[Clock] Interval set to SLOW\n");
                    break;
                case 2:
                    current_interval = INTERVAL_FAST;
                    printf("\n[Clock] Interval set to FAST\n");
                    break;
            }
            
            // 更新定时器间隔
            uint64_t next_time = clint_get_time() + current_interval;
            clint_set_timer(next_time);
        }
        
        // 短延迟
        for (volatile int i = 0; i < 1000; i++);
    }
    
    return 0;
}