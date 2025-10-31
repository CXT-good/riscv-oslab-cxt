
// kernel/clock.h
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "types.h"
#include "clint.h"

// 读取时间计数器 - 使用CLINT
static inline uint64_t get_time(void) {
    return clint_get_time();
}

// 读取时钟周期计数器
static inline uint64_t get_cycles(void) {
    uint64_t cycles;
    #ifdef __riscv
    asm volatile ("rdcycle %0" : "=r" (cycles));
    #else
    cycles = 0;
    #endif
    return cycles;
}

// 设置定时器 - 使用CLINT
static inline void set_timer(uint64_t time) {
    clint_set_timer(time);
}

#endif