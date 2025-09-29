#include "types.h"
#include "printf.h"
#include "console.h"
#include "colors.h"  // 包含颜色定义
#include "clock.h"   // 包含时钟读取函数

// External symbols from linker script
extern void _bss_start;
extern void _bss_end;

// 基础功能测试
void test_printf_basic(void) {
    printf("=== Basic Functionality Tests ===\n");
    
    printf("Decimal: %d\n", 42);
    printf("Negative: %d\n", -123);
    printf("Zero: %d\n", 0);
    printf("Hex: 0x%x\n", 0xABC);
    printf("String: %s\n", "Hello");
    printf("Character: %c\n", 'X');
    printf("Percent: 100%% complete\n\n");
}

// 边界情况测试
void test_printf_edge_cases(void) {
    printf("=== Edge Case Tests ===\n");
    
    printf("INT_MAX: %d\n", 2147483647);
    printf("INT_MIN: %d\n", -2147483648);
    printf("NULL string: %s\n", (char*)0);
    printf("Empty string: '%s'\n", "");
    
    int x = 42;
    printf("Pointer: %p\n", &x);
    printf("NULL pointer: %p\n\n", NULL);
}

// 格式错误测试
void test_printf_errors(void) {
    printf("=== Error Handling Tests ===\n");
    
    printf("Unknown format: %q\n");
    printf("Incomplete format: %");
    printf(" followed by text\n\n");
}

// 综合测试
void test_printf_comprehensive(void) {
    printf("=== Comprehensive Tests ===\n");
    
    printf("Multi-args: %d + %d = %d, %s %c%c%c\n", 
           2, 3, 5, "result", 'A', 'B', 'C');
           
    printf("Mixed formats: char=%c, dec=%d, hex=0x%x, str=%s\n",
           'Z', 100, 255, "mixed_test");
}


// 颜色测试函数
void test_colors(void) {
    printf("\n=== Color Output Tests ===\n");
    
    printf_color(COLOR_FG_RED, "This is RED text\n");
    printf_color(COLOR_FG_GREEN, "This is GREEN text\n");
    printf_color(COLOR_FG_YELLOW, "This is YELLOW text\n");
    printf_color(COLOR_FG_BLUE, "This is BLUE text\n");
    printf_color(COLOR_FG_MAGENTA, "This is MAGENTA text\n");
    printf_color(COLOR_FG_CYAN, "This is CYAN text\n");
    printf_color(COLOR_FG_WHITE, "This is WHITE text\n");
    
    printf_color(COLOR_FG_BRIGHT_RED, "This is BRIGHT RED text\n");
    printf_color(COLOR_FG_BRIGHT_GREEN, "This is BRIGHT GREEN text\n");
    printf("Back to normal color\n");
}

void test_cursor_positioning(void) {
    printf("\n=== Cursor Positioning Tests ===\n");

    // 第2行第1列
    goto_xy(1, 2);
    printf("Line 2: Original position\n");

    // 第3行第1列
    goto_xy(1, 3);
    printf("Line 3: Some text\n");

    // 第4行第10列
    goto_xy(10, 4);
    printf("Line 4: Moved to column 10\n");

    // 第6行第20列
    goto_xy(20, 6);
    printf("Line 6: Moved to column 20\n");

    // 第8行第1列
    goto_xy(1, 8);
    printf("Line 8: Back to start of line\n");
}

// 清除行功能测试
void test_clear_functions(void) {
    printf("\n=== Clear Function Tests ===\n");
    
    printf("Line to be cleared: This text will be removed\n");
    
    // 简单延迟
    for (volatile int i = 0; i < 1000000; i++);
    
    // 回到上一行并清除
    goto_xy(1, 5);
    clear_line();
    printf("Line was cleared and this is new text!\n");
}

// 综合演示
void demo_advanced_features(void) {
    printf("\n=== Advanced Features Demo ===\n");
    
    clear_screen();
    goto_xy(1, 1);
    
    printf_color(COLOR_FG_BLUE, "=== RISC-V OS Advanced Console Demo ===\n\n");
    
    // 创建彩色表格效果
    printf_color(COLOR_FG_CYAN, "Status Dashboard:\n");
    printf_color(COLOR_FG_GREEN, "✓ Kernel: Running\n");
    printf_color(COLOR_FG_YELLOW, "⚠ Memory: 85%% used\n");
    printf_color(COLOR_FG_RED, "✗ Network: Disconnected\n\n");
    
    // 进度条模拟
    printf_color(COLOR_FG_MAGENTA, "System Initialization: ");
    for (int i = 0; i < 10; i++) {
        printf_color(COLOR_FG_BRIGHT_GREEN, "█");
        for (volatile int j = 0; j < 500000; j++);
    }
    printf_color(COLOR_FG_GREEN, " COMPLETE\n\n");
}

void test_performance(void) {
    printf("=== Performance Test ===\n");
    uint64_t start_time = get_cycles(); // 需要实现时钟读取
    
    // 大量输出测试
    for (int i = 0; i < 1000; i++) {
        printf("Line %d: Performance testing...\n", i);
    }
    
    uint64_t end_time = get_cycles();
    printf("Time taken: %d cycles\n", end_time - start_time);
}

// Main function
void main(void) {
    // Initialize console system
    console_init();

    // test_printf_basic(); //基本格式化功能
    // test_printf_edge_cases();//边界条件处理
    // test_printf_errors();// 错误恢复测试
    // test_printf_comprehensive();//综合测试
    
    // //清屏测试
    // printf("=== Phase 1: Basic Clear Screen Test ===\n");
    // printf("This screen will be cleared in 2 seconds...\n");
    
    // for (volatile int i = 0; i < 2000000; i++);
    
    // clear_screen();
    // for (volatile int i = 0; i < 2000000; i++);

    // printf("Screen cleared! Now testing advanced features...\n\n");
    
    // test_colors();//颜色输出功能
    // test_cursor_positioning();//光标定位
    // test_clear_functions();//清除行
    
    // clear_screen();
    // goto_xy(1, 1);
    
    // demo_advanced_features();//综合测试
    
    // printf_color(COLOR_FG_GREEN, "\n=== All console features tested successfully! ===\n");
    // printf("System is ready for use.\n");

    // test_performance(); //性能测试
    
    while (1) {}
}