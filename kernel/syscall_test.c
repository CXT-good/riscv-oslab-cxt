// kernel/syscall_test.c
#include "printf.h"
#include "proc.h"
#include "types.h"
#include "syscall_test.h"
#include "clock.h"


void test_basic_syscalls(void) {
    printf("=== Testing Basic System Calls ===\n");
    
    printf("1. Testing getpid...\n");
    int pid = getpid();
    printf("Current PID: %d\n", pid);
    
    printf("2. Testing fork...\n");
    int child_pid = fork();
    
    if (child_pid == 0) {
        // 子进程 - 这部分工作正常
        printf("Child process: PID=%d, Parent PID=%d\n", getpid(), getppid());
        printf("Child exiting with status 42\n");
        exit(42);
    } else if (child_pid > 0) {
        // 父进程 - 这里需要确保能继续执行
        printf("Parent process: PID=%d, created child %d\n", getpid(), child_pid);
        
        // 等待子进程退出
        int status;
        int waited_pid = wait(&status);
        printf("Parent: child %d exited with status: %d\n", waited_pid, status);
        
        if (waited_pid == child_pid && status == 0) {
            printf("✓ Fork/wait/exit test PASSED\n");
        } else {
            printf("✗ Fork/wait/exit test FAILED\n");
        }
        
        // 重要：确保父进程继续执行其他测试
        printf("Parent process continuing with other tests...\n");
    } else {
        printf("✗ Fork failed!\n");
    }
    
    printf("Basic system calls test completed\n\n");
}

// 参数传递测试
void test_parameter_passing(void) {
    printf("=== Testing Parameter Passing ===\n");
    
    char buffer[] = "Hello, World!";
    int buffer_len = strlen(buffer);
    
    printf("1. Testing normal parameter passing...\n");
    
    // 测试正常写入到标准输出
    int bytes_written = write(1, buffer, buffer_len);
    printf("\nWrote %d bytes to stdout\n", bytes_written);
    
    if (bytes_written == buffer_len) {
        printf("✓ Normal write test PASSED\n");
    } else {
        printf("✗ Normal write test FAILED\n");
    }
    
    printf("2. Testing edge cases...\n");
    
    // 测试边界情况
    int result;
    
    // 无效文件描述符
    result = write(-1, buffer, 10);
    printf("Write to invalid fd (-1): result=%d (expected -1)\n", result);
    
    // 空指针（需要安全检查）
    result = write(1, NULL, 10);
    printf("Write with NULL buffer: result=%d (expected -1)\n", result);
    
    // 负数长度
    result = write(1, buffer, -1);
    printf("Write with negative length: result=%d (expected -1)\n", result);
    
    // 零长度
    result = write(1, buffer, 0);
    printf("Write with zero length: result=%d (expected 0)\n", result);
    
    printf("Parameter passing test completed\n\n");
}

// 安全性测试
void test_security(void) {
    printf("=== Testing Security Checks ===\n");
    
    int result;
    
    printf("1. Testing invalid pointer access...\n");
    
    // 测试无效指针访问
    char *invalid_ptr = (char*)0x1000000;  // 可能无效的地址
    result = write(1, invalid_ptr, 10);
    printf("Invalid pointer write result: %d (expected -1)\n", result);
    
    // 测试内核空间指针（如果用户态测试）
    char *kernel_ptr = (char*)0x80000000;  // 内核地址
    result = write(1, kernel_ptr, 10);
    printf("Kernel pointer write result: %d (expected -1)\n", result);
    
    printf("2. Testing buffer boundaries...\n");
    
    // 测试缓冲区边界
    char small_buf[4];
    result = read(0, small_buf, 1000);  // 尝试读取超过缓冲区大小
    printf("Oversized read result: %d (expected -1)\n", result);
    
    // 测试未映射地址
    char *unmapped_ptr = (char*)0x30000000;  // 可能未映射的地址
    result = write(1, unmapped_ptr, 10);
    printf("Unmapped pointer write result: %d (expected -1)\n", result);
    
    printf("3. Testing permission checks...\n");
    
    // 测试只读内存写入（需要具体实现）
    // 这里可以测试对代码段的写入权限
    
    printf("Security tests completed\n\n");
}

// 性能测试
void test_syscall_performance(void) {
    printf("=== Testing System Call Performance ===\n");
    
    uint64_t start_time, end_time;
    int test_iterations = 10000;
    
    printf("1. Testing getpid performance (%d iterations)...\n", test_iterations);
    
    // 读取开始时间
    asm volatile("csrr %0, time" : "=r"(start_time));
    
    // 大量系统调用测试
    for (int i = 0; i < test_iterations; i++) {
        getpid();  // 简单的系统调用
    }
    
    // 读取结束时间
    asm volatile("csrr %0, time" : "=r"(end_time));
    
    uint64_t total_cycles = end_time - start_time;
    uint64_t avg_cycles = total_cycles / test_iterations;
    
    printf("10000 getpid() calls took %lu cycles\n", total_cycles);
    printf("Average per call: %lu cycles\n", avg_cycles);
    
    printf("2. Testing write performance...\n");
    
    const char test_buffer[] = "Performance test string\n";
    int write_len = strlen(test_buffer);
    
    asm volatile("csrr %0, time" : "=r"(start_time));
    
    for (int i = 0; i < 1000; i++) {
        write(1, test_buffer, write_len);
    }
    
    asm volatile("csrr %0, time" : "=r"(end_time));
    
    total_cycles = end_time - start_time;
    avg_cycles = total_cycles / 1000;
    
    printf("1000 write() calls took %lu cycles\n", total_cycles);
    printf("Average per call: %lu cycles\n", avg_cycles);
    
    printf("Performance tests completed\n\n");
}

// 综合测试函数
void run_comprehensive_syscall_tests(void) {
    printf("\n🔧 STARTING COMPREHENSIVE SYSTEM CALL TESTS\n");
    
    // 初始化必要的子系统
    printf("Initializing subsystems for testing...\n");
    
    // 运行测试套件
    // test_basic_syscalls();
    // test_parameter_passing();
    // test_security();
    test_syscall_performance();
    
    printf("\n✅ ALL SYSTEM CALL TESTS COMPLETED\n");
}



// main() is provided by kernel/main.c; tests are invoked from there.