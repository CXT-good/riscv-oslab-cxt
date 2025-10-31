#include "trap.h"
#include "printf.h"
#include "clock.h"
#include "sbi.h"

// 中断处理函数表
static interrupt_handler_t interrupt_handlers[64];
static const char *exception_names[16] = {
    "Instruction address misaligned",
    "Instruction access fault", 
    "Illegal instruction",
    "Breakpoint",
    "Load address misaligned",
    "Load access fault",
    "Store address misaligned", 
    "Store access fault",
    "Environment call from U-mode",
    "Environment call from S-mode",
    "Reserved",
    "Environment call from M-mode",
    "Instruction page fault",
    "Load page fault",
    "Reserved", 
    "Store page fault"
};

// 全局变量 - 添加滴答计数和间隔控制
static volatile uint64_t timer_ticks = 0;
// uint64_t current_interval = 1000000; // 默认1秒间隔
 uint64_t current_interval; // 改为 extern 声明

// 初始化中断处理函数表
static void init_interrupt_handlers(void) {
    for (int i = 0; i < 64; i++) {
        interrupt_handlers[i] = NULL;
    }
}

// 注册中断处理函数
void register_interrupt(int irq, interrupt_handler_t handler) {
    if (irq >= 0 && irq < 64) {
        interrupt_handlers[irq] = handler;
        printf("TRAP: registered handler for IRQ %d at %p\n", irq, handler);
    }
}

// 启用中断
void enable_interrupt(int irq) {
    if (irq == 5) { // 时钟中断
        uint64_t sie_val = r_sie();
        w_sie(sie_val | (1 << 5));
        printf("TRAP: enabled supervisor timer interrupt\n");
    } else if (irq == 1) { // 软件中断
        uint64_t sie_val = r_sie();
        w_sie(sie_val | (1 << 1));
        printf("TRAP: enabled supervisor software interrupt\n");
    } else if (irq == 9) { // 外部中断
        uint64_t sie_val = r_sie();
        w_sie(sie_val | (1 << 9));
        printf("TRAP: enabled supervisor external interrupt\n");
    }
}

// 禁用中断
void disable_interrupt(int irq) {
    if (irq == 5) { // 时钟中断
        uint64_t sie_val = r_sie();
        w_sie(sie_val & ~(1 << 5));
        printf("TRAP: disabled supervisor timer interrupt\n");
    }
}

// 详细的CSR状态调试
void dump_csr_state_detailed(void) {
    printf("=== Detailed CSR State ===\n");
    
    // M-mode CSR
    uint64_t mstatus = r_mstatus();
    printf("mstatus: 0x%x\n", (int)mstatus);
    printf("  MPP: %d, MIE: %d, SIE: %d\n", 
           (int)((mstatus >> 11) & 0x3),
           (int)((mstatus >> 3) & 1),
           (int)((mstatus >> 1) & 1));
    
    uint64_t mie = r_mie();
    printf("mie: 0x%x\n", (int)mie);
    printf("  MEIE:%d, MTIE:%d, MSIE:%d, SEIE:%d, STIE:%d, SSIE:%d\n",
           (int)((mie >> 11) & 1), (int)((mie >> 7) & 1), (int)((mie >> 3) & 1),
           (int)((mie >> 9) & 1), (int)((mie >> 5) & 1), (int)((mie >> 1) & 1));
    
    printf("mtvec: 0x%x\n", (int)r_mtvec());
    printf("mepc: 0x%x\n", (int)r_mepc());
    printf("mcause: 0x%x\n", (int)r_mcause());
    
    uint64_t mip = r_mip();
    printf("mip: 0x%x\n", (int)mip);
    printf("  MEIP:%d, MTIP:%d, MSIP:%d, SEIP:%d, STIP:%d, SSIP:%d\n",
           (int)((mip >> 11) & 1), (int)((mip >> 7) & 1), (int)((mip >> 3) & 1),
           (int)((mip >> 9) & 1), (int)((mip >> 5) & 1), (int)((mip >> 1) & 1));
    
    // S-mode CSR
    uint64_t sstatus = r_sstatus();
    printf("sstatus: 0x%x\n", (int)sstatus);
    printf("  SIE: %d\n", (int)((sstatus >> 1) & 1));
    
    uint64_t sie = r_sie();
    printf("sie: 0x%x\n", (int)sie);
    printf("  SEIE:%d, STIE:%d, SSIE:%d\n",
           (int)((sie >> 9) & 1), (int)((sie >> 5) & 1), (int)((sie >> 1) & 1));
    
    printf("stvec: 0x%x\n", (int)r_stvec());
    printf("sepc: 0x%x\n", (int)r_sepc());
    printf("scause: 0x%x\n", (int)r_scause());
    
    uint64_t sip = r_sip();
    printf("sip: 0x%x\n", (int)sip);
    printf("  SEIP:%d, STIP:%d, SSIP:%d\n",
           (int)((sip >> 9) & 1), (int)((sip >> 5) & 1), (int)((sip >> 1) & 1));
    
    // 委托寄存器
    uint64_t mideleg = r_mideleg();
    printf("mideleg: 0x%x\n", (int)mideleg);
    printf("  SEIP delegated: %d, STIP delegated: %d, SSIP delegated: %d\n",
           (int)((mideleg >> 9) & 1), (int)((mideleg >> 5) & 1), (int)((mideleg >> 1) & 1));
    
    uint64_t medeleg = r_medeleg();
    printf("medeleg: 0x%x\n", (int)medeleg);
    for (int i = 0; i < 16; i++) {
        if ((medeleg >> i) & 1) {
            printf("  Exception %d (%s) delegated\n", i, exception_names[i]);
        }
    }
    
    printf("CLINT mtime: 0x%x\n", (int)clint_get_time());
    printf("CLINT mtimecmp: 0x%x\n", (int)clint_get_timer());
    printf("==========================\n");
}

// 简化的CSR状态
void dump_csr_state(void) {
    printf("CSR State:\n");
    printf("  mstatus: 0x%x, mie: 0x%x, mip: 0x%x\n", 
           (int)r_mstatus(), (int)r_mie(), (int)r_mip());
    printf("  sstatus: 0x%x, sie: 0x%x, sip: 0x%x\n",
           (int)r_sstatus(), (int)r_sie(), (int)r_sip());
    printf("  mideleg: 0x%x, medeleg: 0x%x\n",
           (int)r_mideleg(), (int)r_medeleg());
    printf("  CLINT: time=0x%x, timecmp=0x%x\n",
           (int)clint_get_time(), (int)clint_get_timer());
}

// 检查异常是否应该被委托
static int should_delegate_exception(uint64_t cause) {
    uint64_t medeleg = r_medeleg();
    if (cause < 64 && ((medeleg >> cause) & 1)) {
        return 1;
    }
    return 0;
}

// 异常处理函数
void handle_exception(struct trapframe *tf) {
    uint64_t cause = tf->cause;
    
    printf("S-MODE EXCEPTION: %s (cause=%d) at epc=%p\n", 
           exception_names[cause], (int)cause, (void*)tf->epc);
    
    switch (cause) {
        case CAUSE_ILLEGAL_INSTRUCTION:
            printf("  Illegal instruction at %p, skipping...\n", (void*)tf->epc);
            tf->epc += 4; // 跳过非法指令
            break;
            
        case CAUSE_BREAKPOINT:
            printf("  Breakpoint at %p, continuing...\n", (void*)tf->epc);
            tf->epc += 4; // 继续执行
            break;
            
        case CAUSE_FETCH_PAGE_FAULT:
        case CAUSE_LOAD_PAGE_FAULT:
        case CAUSE_STORE_PAGE_FAULT:
            printf("  Page fault, badaddr=%p, skipping...\n", (void*)tf->tval);
            tf->epc += 4; // 跳过故障指令
            break;
            
        case CAUSE_SUPERVISOR_ECALL:
            printf("  Supervisor ECALL at %p, skipping...\n", (void*)tf->epc);
            tf->epc += 4; // 跳过ecall指令
            break;
            
        default:
            printf("  Unhandled S-mode exception, epc += 4 to continue\n");
            tf->epc += 4; // 跳过当前指令继续执行
    }
}

// 页错误处理
void handle_page_fault(struct trapframe *tf) {
    printf("PAGE FAULT: type=%d, badaddr=%p, epc=%p\n",
           (int)(tf->cause), (void*)tf->tval, (void*)tf->epc);
    tf->epc += 4;
}

// 非法指令处理
void handle_illegal_instruction(struct trapframe *tf) {
    printf("ILLEGAL INSTRUCTION at %p\n", (void*)tf->epc);
    tf->epc += 4;
}

// 时钟中断处理函数 - 修改为支持测试功能
void timer_interrupt_handler(struct trapframe *tf) {
    timer_ticks++;
    
    // 测试1: 每次时钟中断输出 "T" 字符
    printf("T");
    
    // 设置下一次定时器中断，使用动态间隔
    uint64_t current_time = clint_get_time();
    uint64_t next_time = current_time + current_interval;
    clint_set_timer(next_time);
    
    asm volatile ("fence w, w" : : : "memory");
    
    // 测试2: 每10次中断输出一次ticks计数
    if (timer_ticks % 10 == 0) {
        printf("\n[Timer] ticks=%d, interval=%d\n", 
               (int)timer_ticks, (int)current_interval);
    }
}

// 软件中断处理函数
void software_interrupt_handler(struct trapframe *tf) {
    printf("S"); // 简单标记软件中断
    // 清除软件中断挂起位
    asm volatile ("csrc sip, %0" : : "r" (1 << 1));
}

// UART输入中断处理函数
void uart_interrupt_handler(struct trapframe *tf) {
    // 这里可以添加UART输入处理逻辑
    // 对于测试，我们简单输出一个字符表示收到了UART中断
    printf("U");
}

// S-mode陷阱处理函数
void s_trap_handler(struct trapframe *tf) {
    uint64_t cause = tf->cause;
    
    // 检查是否是中断
    if (cause & (1ULL << 63)) {
        // 中断处理
        uint64_t interrupt_type = cause & ~(1ULL << 63);
        
        // 调用注册的中断处理函数
        if (interrupt_handlers[interrupt_type]) {
            interrupt_handlers[interrupt_type](tf);
        } else {
            printf("Unhandled S-mode interrupt: %d\n", (int)interrupt_type);
        }
    } else {
        // 异常处理
        handle_exception(tf);
    }
}

// M-mode陷阱处理函数 - 详细调试版本
void m_trap_handler(struct trapframe *tf) {
    uint64_t cause = tf->cause;
    
    printf("\n=== M-TRAP ENTERED ===\n");
    printf("M-TRAP: cause=0x%x (", (int)cause);
    
    // 检查是否是中断
    if (cause & (1ULL << 63)) {
        uint64_t interrupt_type = cause & ~(1ULL << 63);
        printf("interrupt %d)\n", (int)interrupt_type);
        
        if (interrupt_type == 7) { // M-mode定时器中断
            printf("M-TRAP: M-mode timer interrupt\n");
            uint64_t current_time = clint_get_time();
            uint64_t next_time = current_time + 5000000;
            clint_set_timer(next_time);
            printf("M"); // 标记M-mode定时器中断
        } else {
            printf("M-TRAP: unhandled interrupt type %d\n", (int)interrupt_type);
        }
    } else {
        uint64_t exception_type = cause;
        printf("exception %d: %s)\n", (int)exception_type, exception_names[exception_type]);
        
        // 详细检查委托状态
        printf("M-TRAP: medeleg=0x%x\n", (int)r_medeleg());
        if (should_delegate_exception(exception_type)) {
            printf("M-TRAP: WARNING! Exception %d SHOULD be delegated but trapped in M-mode!\n", 
                   (int)exception_type);
            printf("M-TRAP: This indicates a problem with exception delegation!\n");
        } else {
            printf("M-TRAP: Exception %d is NOT delegated (expected)\n", (int)exception_type);
        }
        
        printf("M-TRAP: epc=%p, tval=0x%x\n", (void*)tf->epc, (int)tf->tval);
        
        // 对于M-mode异常，尝试恢复执行
        switch (exception_type) {
            case CAUSE_ILLEGAL_INSTRUCTION:
            case CAUSE_BREAKPOINT:
            case CAUSE_SUPERVISOR_ECALL:
                printf("M-TRAP: Skipping instruction at %p\n", (void*)tf->epc);
                tf->epc += 4; // 跳过当前指令
                break;
            default:
                printf("M-TRAP: Cannot handle exception %d\n", (int)exception_type);
                dump_csr_state_detailed();
                printf("M-TRAP: Stopping...\n");
                while(1); // 严重错误，停机
        }
    }
    
    printf("=== M-TRAP EXIT ===\n");
}

// 中断系统初始化 - 增加调试信息
void trap_init(void) {
    printf("TRAP: starting interrupt system initialization...\n");
    
    // 初始化中断处理函数表
    init_interrupt_handlers();
    
    // 注册中断处理函数
    register_interrupt(5, timer_interrupt_handler);  // S-mode时钟中断
    register_interrupt(1, software_interrupt_handler); // S-mode软件中断
    register_interrupt(10, uart_interrupt_handler);  // UART中断（假设为IRQ 10）
    
    // 1. 设置M-mode陷阱向量
    uint64_t mtvec_val = (uint64_t)timervec | 1; // 直接模式
    w_mtvec(mtvec_val);
    printf("TRAP: mtvec set to %p (aligned: %s)\n", 
           (void*)r_mtvec(), (r_mtvec() & 0x3) == 1 ? "direct" : "vectored");
    
    // 2. 设置S-mode陷阱向量
    uint64_t stvec_val = (uint64_t)kernelvec | 1; // 直接模式
    w_stvec(stvec_val);
    printf("TRAP: stvec set to %p (aligned: %s)\n", 
           (void*)r_stvec(), (r_stvec() & 0x3) == 1 ? "direct" : "vectored");
    
    // 3. 配置中断委托
    uint64_t mideleg_val = (1 << 1) | (1 << 5) | (1 << 9); // SSIP, STIP, SEIP
    w_mideleg(mideleg_val);
    printf("TRAP: mideleg set to 0x%x\n", (int)r_mideleg());
    
    // 4. 配置异常委托 - 委托常见异常给S-mode
    uint64_t medeleg_val = (1 << CAUSE_ILLEGAL_INSTRUCTION) |
                          (1 << CAUSE_BREAKPOINT) |
                          (1 << CAUSE_SUPERVISOR_ECALL) |
                          (1 << CAUSE_FETCH_PAGE_FAULT) |
                          (1 << CAUSE_LOAD_PAGE_FAULT) |
                          (1 << CAUSE_STORE_PAGE_FAULT);
    w_medeleg(medeleg_val);
    printf("TRAP: medeleg set to 0x%x\n", (int)r_medeleg());
    
    // 5. 启用M-mode中断
    w_mie(r_mie() | (1 << 7)); // MTIE
    printf("TRAP: mie set to 0x%x\n", (int)r_mie());
    
    // 6. 启用S-mode中断
    w_sie(r_sie() | (1 << 1) | (1 << 5) | (1 << 9)); // SSIE, STIE, SEIE
    printf("TRAP: sie set to 0x%x\n", (int)r_sie());
    
    // 7. 全局启用中断
    w_mstatus(r_mstatus() | (1 << 3)); // MIE
    printf("TRAP: mstatus set to 0x%x\n", (int)r_mstatus());
    
    // 启用S-mode中断
    w_sstatus(r_sstatus() | (1 << 1)); // SIE
    printf("TRAP: sstatus set to 0x%x\n", (int)r_sstatus());
    
    printf("TRAP: interrupt system initialized\n");
    dump_csr_state_detailed();
}

// 定时器初始化
void timer_init(void) {
    printf("TRAP: starting timer initialization...\n");
    
    uint64_t current_time = clint_get_time();
    printf("TRAP: current CLINT time: %d cycles\n", (int)current_time);
    
    uint64_t interval = current_interval;
    uint64_t next_time = current_time + interval;
    
    printf("TRAP: setting timer for %d cycles\n", (int)interval);
    
    clint_set_timer(next_time);
    asm volatile ("fence w, w" : : : "memory");
    
    // 启用S-mode时钟中断
    enable_interrupt(5);
    
    printf("TRAP: timer initialization completed\n");
}

// 获取时钟滴答计数
uint64_t get_timer_ticks(void) {
    return timer_ticks;
}

// 设置时钟间隔
void set_timer_interval(uint64_t interval) {
    current_interval = interval;
    printf("TRAP: timer interval set to %d cycles\n", (int)interval);
}

// 获取当前时钟间隔
uint64_t get_timer_interval(void) {
    return current_interval;
}

// M-mode 陷阱处理函数 - 供汇编代码调用
void trap_handler(void) {
    // 创建简单的陷阱帧
    struct trapframe tf;
    
    // 读取相关CSR寄存器
    tf.epc = r_mepc();
    tf.cause = r_mcause();
    tf.tval = r_mtval();
    
    // 调用M-mode陷阱处理函数
    m_trap_handler(&tf);
    
    // 恢复上下文
    w_mepc(tf.epc);
}