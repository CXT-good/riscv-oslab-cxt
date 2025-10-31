#ifndef _CLINT_H_
#define _CLINT_H_

#include "types.h"

// CLINT (Core Local Interrupter) 内存映射地址
#define CLINT_BASE 0x2000000L
#define CLINT_MTIMECMP(hartid) (CLINT_BASE + 0x4000 + 8 * (hartid))
#define CLINT_MTIME (CLINT_BASE + 0xBFF8)

// 读取mtime
static inline uint64_t clint_get_time(void) {
    return *(volatile uint64_t*)CLINT_MTIME;
}

// 设置mtimecmp
static inline void clint_set_timer(uint64_t time) {
    *(volatile uint64_t*)(CLINT_MTIMECMP(0)) = time;
}

// 读取mtimecmp
static inline uint64_t clint_get_timer(void) {
    return *(volatile uint64_t*)(CLINT_MTIMECMP(0));
}

#endif
