#include "printf.h"
#include "proc.h"
#include "mm.h"
#include "console.h"
#include "trap.h"
#include "syscall.h"
#include "clock.h"
#include "uart.h"
#include "syscall_test.h"
#include "fs_test.h"

void run_filesystem_tests(void);

void main(void) {
    
    // 运行文件系统测试
    run_filesystem_tests();
    
    printf("=== All Tests Completed ===\n");
    
    while (1) {
        asm volatile("wfi");
    }
}