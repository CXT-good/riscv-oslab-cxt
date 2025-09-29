// kernel/clock.h
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "types.h"

// 读取时钟周期计数器
static inline uint64_t get_cycles(void) {
    uint64_t cycles;
    #ifdef __riscv
    asm volatile ("rdcycle %0" : "=r" (cycles));
    #else
    cycles = 0;  // 非RISC-V平台的备用实现
    #endif
    return cycles;
}

// 读取时间计数器（如果支持）
static inline uint64_t get_time(void) {
    uint64_t time;
    #ifdef __riscv
    asm volatile ("rdtime %0" : "=r" (time));
    #else
    time = 0;
    #endif
    return time;
}

// 读取指令计数器
static inline uint64_t get_instret(void) {
    uint64_t instret;
    #ifdef __riscv
    asm volatile ("rdinstret %0" : "=r" (instret));
    #else
    instret = 0;
    #endif
    return instret;
}

#endif