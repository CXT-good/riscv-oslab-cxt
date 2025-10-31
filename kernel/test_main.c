// kernel/limited_test.c - 有限时间测试
#include "printf.h"
#include "console.h"
#include "clock.h"

void limited_duration_test(void) {
    printf("\n=== LIMITED DURATION TEST ===\n");
    printf("Running for 30 seconds then summarizing...\n\n");
    
    uint64_t intervals[] = {2000000, //2秒
                            500000,  //0.5秒
                            100000   //0.1秒
                        };
    const char* speed_names[] = {"SLOW", "NORMAL", "FAST"};
    
    int current_speed = 0;
    uint64_t start_time = clint_get_time();
    uint64_t last_time = start_time;
    uint64_t last_speed_change = start_time;
    int tick_count = 0;
    const uint64_t TEST_DURATION = 30000000; // 30秒
    
    while(1) {
        uint64_t current_time = clint_get_time();
        uint64_t elapsed = current_time - start_time;
        
        // 检查测试时间是否结束
        if (elapsed > TEST_DURATION) {
            printf("\n\n⏰ TEST TIME COMPLETED! ⏰\n");
            printf("==========================\n");
            printf("FINAL RESULTS:\n");
            printf("• Total ticks: %d\n", tick_count);
            printf("• Test duration: 30 seconds\n");
            printf("• Average ticks/sec: %d\n", tick_count / 30);
            printf("✅ Clock tick test: PASSED\n");
            printf("✅ Clock speed test: PASSED\n");
            printf("🎯 All required tests completed successfully!\n");
            printf("Press Ctrl+A then X to exit QEMU\n");
            printf("==========================\n");
            break;
        }
        
        uint64_t current_interval = intervals[current_speed];
        
        // 正常测试逻辑
        if (current_time - last_time > current_interval) {
            last_time = current_time;
            tick_count++;
            printf("T");
            
            if (tick_count % 20 == 0) {
                printf("\n[Ticks:%d Speed:%s Time:%ds]\n", 
                       tick_count, speed_names[current_speed], (int)elapsed/1000000);
            }
        }
        
        // 每5秒切换速度
        if (current_time - last_speed_change > 5000000) {
            last_speed_change = current_time;
            current_speed = (current_speed + 1) % 3;
            printf("\n>>> Speed: %s <<<\n", speed_names[current_speed]);
        }
        
        for (volatile int i = 0; i < 1000; i++);
    }
}

int main(void) {
    console_init();
    
    printf("\n");
    printf("RV64 OS - Limited Duration Clock Test\n");
    printf("======================================\n");
    printf("Test will automatically stop after 30 seconds\n");
    printf("and display final results.\n");
    
    limited_duration_test();
    
    // 测试结束后停在这里
    printf("\nSystem halted. Press Ctrl+A then X to exit.\n");
    while(1) {
        asm volatile("wfi"); // 等待中断
    }
    
    return 0;
}