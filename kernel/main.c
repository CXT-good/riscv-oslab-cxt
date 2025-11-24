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
#include "file_time.h"

void run_file_time_tests(void);

void run_filesystem_tests(void);

void main(void) {
    
    // // 运行文件系统测试
    // run_filesystem_tests();

    // 新增文件时间测试
    run_file_time_tests();
    
    
    printf("=== All Tests Completed ===\n");
    
    while (1) {
        asm volatile("wfi");
    }
}