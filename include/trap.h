// trap.h
#ifndef _TRAP_H_
#define _TRAP_H_

#include "types.h"

// 陷阱帧结构
struct trapframe {
    uint64_t epc;
    uint64_t cause;
    uint64_t tval;
    // 可以添加更多寄存器...
};

// 中断处理函数类型
typedef void (*interrupt_handler_t)(struct trapframe *);

// 函数声明
void trap_init(void);
void timer_init(void);
void enable_interrupt(int irq);
void disable_interrupt(int irq);
void register_interrupt(int irq, interrupt_handler_t handler);
uint64_t get_timer_ticks(void);
void set_timer_interval(uint64_t interval);
uint64_t get_timer_interval(void);

// 陷阱处理函数
void m_trap_handler(struct trapframe *tf);
void s_trap_handler(struct trapframe *tf);
void trap_handler(void);  // 添加这行

// 中断处理函数
void timer_interrupt_handler(struct trapframe *tf);
void software_interrupt_handler(struct trapframe *tf);
void uart_interrupt_handler(struct trapframe *tf);

// CSR 操作
static inline uint64_t r_mstatus() { uint64_t x; asm volatile("csrr %0, mstatus" : "=r"(x)); return x; }
static inline void w_mstatus(uint64_t x) { asm volatile("csrw mstatus, %0" : : "r"(x)); }
static inline uint64_t r_mie() { uint64_t x; asm volatile("csrr %0, mie" : "=r"(x)); return x; }
static inline void w_mie(uint64_t x) { asm volatile("csrw mie, %0" : : "r"(x)); }
static inline uint64_t r_mip() { uint64_t x; asm volatile("csrr %0, mip" : "=r"(x)); return x; }
static inline uint64_t r_mtvec() { uint64_t x; asm volatile("csrr %0, mtvec" : "=r"(x)); return x; }
static inline void w_mtvec(uint64_t x) { asm volatile("csrw mtvec, %0" : : "r"(x)); }
static inline uint64_t r_mepc() { uint64_t x; asm volatile("csrr %0, mepc" : "=r"(x)); return x; }
static inline void w_mepc(uint64_t x) { asm volatile("csrw mepc, %0" : : "r"(x)); }
static inline uint64_t r_mcause() { uint64_t x; asm volatile("csrr %0, mcause" : "=r"(x)); return x; }
static inline uint64_t r_mtval() { uint64_t x; asm volatile("csrr %0, mtval" : "=r"(x)); return x; }
static inline uint64_t r_medeleg() { uint64_t x; asm volatile("csrr %0, medeleg" : "=r"(x)); return x; }
static inline void w_medeleg(uint64_t x) { asm volatile("csrw medeleg, %0" : : "r"(x)); }
static inline uint64_t r_mideleg() { uint64_t x; asm volatile("csrr %0, mideleg" : "=r"(x)); return x; }
static inline void w_mideleg(uint64_t x) { asm volatile("csrw mideleg, %0" : : "r"(x)); }

// S-mode CSR
static inline uint64_t r_sstatus() { uint64_t x; asm volatile("csrr %0, sstatus" : "=r"(x)); return x; }
static inline void w_sstatus(uint64_t x) { asm volatile("csrw sstatus, %0" : : "r"(x)); }
static inline uint64_t r_sie() { uint64_t x; asm volatile("csrr %0, sie" : "=r"(x)); return x; }
static inline void w_sie(uint64_t x) { asm volatile("csrw sie, %0" : : "r"(x)); }
static inline uint64_t r_sip() { uint64_t x; asm volatile("csrr %0, sip" : "=r"(x)); return x; }
static inline uint64_t r_stvec() { uint64_t x; asm volatile("csrr %0, stvec" : "=r"(x)); return x; }
static inline void w_stvec(uint64_t x) { asm volatile("csrw stvec, %0" : : "r"(x)); }
static inline uint64_t r_sepc() { uint64_t x; asm volatile("csrr %0, sepc" : "=r"(x)); return x; }
static inline void w_sepc(uint64_t x) { asm volatile("csrw sepc, %0" : : "r"(x)); }
static inline uint64_t r_scause() { uint64_t x; asm volatile("csrr %0, scause" : "=r"(x)); return x; }

// 异常原因
#define CAUSE_MISALIGNED_FETCH 0x0
#define CAUSE_FETCH_ACCESS 0x1
#define CAUSE_ILLEGAL_INSTRUCTION 0x2
#define CAUSE_BREAKPOINT 0x3
#define CAUSE_MISALIGNED_LOAD 0x4
#define CAUSE_LOAD_ACCESS 0x5
#define CAUSE_MISALIGNED_STORE 0x6
#define CAUSE_STORE_ACCESS 0x7
#define CAUSE_USER_ECALL 0x8
#define CAUSE_SUPERVISOR_ECALL 0x9
#define CAUSE_MACHINE_ECALL 0xb
#define CAUSE_FETCH_PAGE_FAULT 0xc
#define CAUSE_LOAD_PAGE_FAULT 0xd
#define CAUSE_STORE_PAGE_FAULT 0xf

// 外部声明
extern void timervec(void);
extern void kernelvec(void);

#endif