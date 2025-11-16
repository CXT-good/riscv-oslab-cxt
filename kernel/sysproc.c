// kernel/sysproc.c
#include "syscall.h"
#include "proc.h"
#include "printf.h"
#include "mm.h"
#include "console.h"
#include "string.h"

#define SYSERR_SUCCESS 0
#define SYSERR_INVALID_ARGS -1
#define SYSERR_ACCESS_DENIED -2
#define SYSERR_MEMORY_FAULT -3
#define SYSERR_RESOURCE_BUSY -4
#define SYSERR_NOT_FOUND -5
#define SYSERR_NOT_SUPPORTED -6
#define SYSERR_INTERNAL -7

// 函数声明
void fork_return_point(void);
void exit_process(int status);  // 声明内核的退出函数

// 子进程的入口点
void fork_return_point(void) {
    printf("🚀 Child process %d started!\n", curr_proc->pid);
    
    // 子进程从 fork 返回 0
    if (curr_proc->trap_context) {
        curr_proc->trap_context->a0 = 0;  // 子进程返回 0
    }
    
    printf("Child process %d: doing simple task...\n", curr_proc->pid);
    
    // 执行简单任务
    for (int i = 0; i < 3; i++) {
        printf("Child %d: step %d\n", curr_proc->pid, i + 1);
        for (volatile int j = 0; j < 10000; j++); // 短延时
    }
    
    printf("✅ Child process %d: exiting\n", curr_proc->pid);
    exit_process(0);  // 使用内核的退出函数
}

// 获取当前进程的辅助函数
struct proc* myproc(void) {
    return curr_proc;
}

// 进程相关系统调用
int sys_fork(void) {
    printf("SYSCALL: fork called from pid %d\n", myproc()->pid);
    
    struct proc *p = alloc_proc();
    if (!p) {
        printf("SYSCALL: fork failed - no free process slots\n");
        set_syscall_error(SYSERR_RESOURCE_BUSY);
        return -1;
    }
    
    // 复制当前进程的上下文
    p->context = myproc()->context;
    
    // 关键修复：设置子进程的返回地址
    p->context.ra = (uint64_t)fork_return_point;
    
    // 确保栈有效
    if (p->kstack == 0) {
        void *stack = alloc_page();
        if (stack) {
            p->kstack = (uint64_t)stack;
            p->context.sp = p->kstack + PAGE_SIZE;
        }
    }
    
    p->state = RUNNABLE;
    
    printf("SYSCALL: fork created process %d, ra=%p, sp=%p\n", 
           p->pid, (void*)p->context.ra, (void*)p->context.sp);
    
    return p->pid;
}

int sys_exit(void) {
    int status;
    if(argint(0, &status) < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    printf("SYSCALL: exit called from pid %d with status %d\n", 
           myproc()->pid, status);
    
    exit_process(status);
    return 0; // unreachable
}

// 在 sysproc.c 中修改 sys_wait
int sys_wait(void) {
    uint64_t status_ptr;
    if(argaddr(0, &status_ptr) < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    printf("SYSCALL: wait called from pid %d, status_ptr=%p\n", 
           myproc()->pid, (void*)status_ptr);
    
    int status;
    int pid = wait_process(&status);
    
    // 重要：检查 wait_process 是否成功返回
    if (pid < 0) {
        printf("SYSCALL: wait_process failed, returning -1\n");
        return -1;
    }
    
    printf("SYSCALL: wait returning pid=%d, status=%d\n", pid, status);
    
    if(pid > 0 && status_ptr != 0) {
        // 将状态值拷贝回用户空间
        struct proc *p = myproc();
        if(copyout(p->pagetable, status_ptr, (char*)&status, sizeof(status)) < 0) {
            printf("SYSCALL: copyout failed in wait\n");
            set_syscall_error(SYSERR_MEMORY_FAULT);
            return -1;
        }
        printf("SYSCALL: status %d copied to user space %p\n", status, (void*)status_ptr);
    }
    
    return pid;
}

int sys_kill(void) {
    int pid;
    if(argint(0, &pid) < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    printf("SYSCALL: kill called for pid %d from pid %d\n", 
           pid, myproc()->pid);
    
    // 查找目标进程
    struct proc *target = NULL;
    spin_lock(&proc_lock);
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].state != UNUSED && proc[i].pid == pid) {
            target = &proc[i];
            break;
        }
    }
    spin_unlock(&proc_lock);
    
    if (!target) {
        set_syscall_error(SYSERR_NOT_FOUND);
        return -1;
    }
    
    // 设置终止标志
    target->killed = 1;
    
    // 如果进程在睡眠，唤醒它
    if (target->state == SLEEPING) {
        target->state = RUNNABLE;
    }
    
    return 0;
}

int sys_getpid(void) {
    int pid = myproc()->pid;
    // printf("SYSCALL: getpid returning %d\n", pid);
    return pid;
}

int sys_getppid(void) {
    struct proc *p = myproc();
    int ppid = p->parent ? p->parent->pid : 0;
    printf("SYSCALL: getppid returning %d\n", ppid);
    return ppid;
}

// 简化版文件相关系统调用
int sys_write(void) {
    int fd;
    uint64_t buf_addr;
    int n;
    
    if(argint(0, &fd) < 0 || argaddr(1, &buf_addr) < 0 || argint(2, &n) < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    if(n < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    // 增强的安全检查
    if(buf_addr == 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    // 检查内核空间指针
    if (buf_addr >= 0x80000000) {
        printf("SECURITY: write attempt with kernel pointer: 0x%lx\n", buf_addr);
        set_syscall_error(SYSERR_ACCESS_DENIED);
        return -1;
    }
    
    // 只支持标准输出和标准错误
    if(fd != 1 && fd != 2) {
        set_syscall_error(SYSERR_NOT_SUPPORTED);
        return -1;
    }
    
    // 限制写入大小
    if (n > 4096) {
        n = 4096; // 限制为4KB
    }
    
    struct proc *p = myproc();
    
    // 从用户空间读取数据
    char *kbuf = alloc_page();
    if(!kbuf) {
        set_syscall_error(SYSERR_MEMORY_FAULT);
        return -1;
    }
    
    // 确保不读取超过页面大小的数据
    if(n > PAGE_SIZE) {
        n = PAGE_SIZE;
    }
    
    if(copyin(p->pagetable, kbuf, buf_addr, n) < 0) {
        free_page(kbuf);
        set_syscall_error(SYSERR_MEMORY_FAULT);
        return -1;
    }
    
    // 直接输出到控制台
    for(int i = 0; i < n; i++) {
        console_putc(kbuf[i]);
    }
    
    free_page(kbuf);
    return n;
}

int sys_read(void) {
    int fd;
    uint64_t buf_addr;
    int n;
    
    if(argint(0, &fd) < 0 || argaddr(1, &buf_addr) < 0 || argint(2, &n) < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    if(n <= 0) {
        return 0;
    }
    
    // 增强的安全检查
    if (buf_addr == 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    // 检查内核空间指针
    if (buf_addr >= 0x80000000) {
        printf("SECURITY: read attempt with kernel pointer: 0x%lx\n", buf_addr);
        set_syscall_error(SYSERR_ACCESS_DENIED);
        return -1;
    }
    
    // 限制读取大小
    if (n > 4096) {
        n = 4096; // 限制为4KB
    }
    
    // 简化实现：返回模拟数据
    struct proc *p = myproc();
    char *kbuf = alloc_page();
    if(!kbuf) {
        set_syscall_error(SYSERR_MEMORY_FAULT);
        return -1;
    }
    
    // 模拟读取数据
    const char *test_data = "test input from stdin\n";
    int data_len = strlen(test_data);
    int read_len = n < data_len ? n : data_len;
    // copy without relying on libc memcpy
    for (int i = 0; i < read_len; i++) kbuf[i] = test_data[i];
    
    // 拷贝到用户空间
    if(copyout(p->pagetable, buf_addr, kbuf, read_len) < 0) {
        free_page(kbuf);
        set_syscall_error(SYSERR_MEMORY_FAULT);
        return -1;
    }
    
    free_page(kbuf);
    return read_len;
}

// 内存管理系统调用
int sys_brk(void) {
    uint64_t addr;
    if(argaddr(0, &addr) < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    printf("SYSCALL: brk called with addr=0x%lx\n", addr);
    
    /* struct proc *p = myproc(); not used in simplified implementation */
    
    // 简化实现：直接返回当前brk值
    // 在实际实现中，这里应该管理进程的堆空间
    
    if(addr == 0) {
        // 查询当前brk - 返回一个合理的值
        return 0x100000; // 1MB
    }
    
    // 对于非零地址，返回成功
    return 0;
}

int sys_sbrk(void) {
    int increment;
    if(argint(0, &increment) < 0) {
        set_syscall_error(SYSERR_INVALID_ARGS);
        return -1;
    }
    
    printf("SYSCALL: sbrk called with increment=%d\n", increment);
    
    // 简化实现：返回当前brk，不实际分配内存
    uint64_t current_brk = 0x100000; // 假设当前brk在1MB
    
    if(increment == 0) {
        return current_brk;
    }
    
    // 返回旧的brk值
    return current_brk;
}