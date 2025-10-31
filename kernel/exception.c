// kernel/exception.c - 修复版本
#include "exception.h"
#include "printf.h"
#include "trap.h"

// 异常处理函数
void handle_exception(struct trap_context *ctx, uint64_t cause) {
    int exc_code = cause & 0xF;
    
    printf("HANDLE_EXCEPTION: code=%d at mepc=%p\n", exc_code, (void*)ctx->mepc);
    
    switch (exc_code) {
        case CAUSE_ILLEGAL_INSTRUCTION:
            printf("ILLEGAL INSTRUCTION - skipping\n");
            // 检查指令是否有效，如果无效则跳过
            if (*(uint32_t*)ctx->mepc == 0x00000000) {
                printf("Found zero instruction, skipping\n");
                ctx->mepc += 4;
            } else {
                printf("Unknown illegal instruction, halting\n");
                while(1) asm volatile("wfi");
            }
            break;
            
        case CAUSE_BREAKPOINT:
            printf("BREAKPOINT - skipping ebreak instruction\n");
            ctx->mepc += 4;  // 跳过ebreak
            break;
            
        case CAUSE_MACHINE_ECALL:
            printf("ECALL - skipping ecall instruction\n");
            ctx->mepc += 4;  // 跳过ecall
            break;
            
        default:
            printf("UNHANDLED EXCEPTION %d - HALTING\n", exc_code);
            while(1) asm volatile("wfi");
            break;
    }
    
    printf("Exception handling complete, returning to mepc=%p\n", (void*)ctx->mepc);
}

// 测试异常处理 - 使用内联汇编确保正确性
void test_exception_handling(void) {
    printf("=== Testing Exception Handling ===\n");
    
    // 测试1: 断点异常 (ebreak)
    printf("1. Testing breakpoint exception...\n");
    printf("Before ebreak\n");
    
    // 使用内联汇编确保正确执行流程
    asm volatile(
        "nop\n"                    // 确保对齐
        "ebreak\n"                 // 触发断点
        "j 1f\n"                   // 跳过填充
        ".align 2\n"               // 确保对齐
        "1:\n"
        "nop\n"                    // 继续执行
        :
        :
    );
    
    printf("After ebreak - Breakpoint handled!\n");
    
    // 测试2: 环境调用异常 (ecall)
    printf("2. Testing ECALL exception...\n");
    printf("Before ecall\n");
    
    asm volatile(
        "nop\n"                    // 确保对齐
        "ecall\n"                  // 触发环境调用
        "j 1f\n"                   // 跳过填充
        ".align 2\n"               // 确保对齐
        "1:\n"
        "nop\n"                    // 继续执行
        :
        :
    );
    
    printf("After ecall - ECALL handled!\n");
    
    printf("=== Exception Tests Completed ===\n\n");
}