#include "syscall_test.h"
#include "syscall.h"
#include "proc.h"
#include "printf.h"
#include "console.h"
/* do not include standard string.h to avoid prototype conflict with test header */

// Simple wrappers used by kernel tests to simulate user-level syscalls

// int getpid(void) {
//     return sys_getpid();
// }
int getpid(void) {
    printf("DEBUG: getpid wrapper called\n");
    
    // 如果 curr_proc 不存在，创建一个临时的
    if (curr_proc == NULL) {
        printf("WARNING: getpid called with no current process, using fallback\n");
        return 1;  // 返回默认值
    }
    
    return sys_getpid();
}

int fork(void) {
    printf("DEBUG: fork wrapper called\n");
    
    if (curr_proc == NULL) {
        printf("ERROR: fork called with no current process\n");
        return -1;
    }
    
    return sys_fork();
}

int getppid(void) {
    return sys_getppid();
}


// 在 syscall_wrappers.c 中修改 wait 函数
int wait(int *status) {
    printf("DEBUG: wait wrapper called, status=%p\n", status);
    
    if (curr_proc == NULL) {
        printf("ERROR: wait called with no current process\n");
        return -1;
    }
    
    // 使用系统调用版本的 wait，而不是直接调用 wait_process
    return sys_wait();
}

int exit(int status) {
    // call kernel exit helper
    exit_process(status);
    return 0; // unreachable
}

int write(int fd, const void *buf, int count) {
    if (count < 0) return -1;
    if (buf == NULL) return -1;
    if (fd != 1 && fd != 2) return -1;

    const char *cbuf = (const char*)buf;
    for (int i = 0; i < count; i++) {
        console_putc(cbuf[i]);
    }
    return count;
}

int read(int fd, void *buf, int count) {
    if (fd != 0) return -1;
    if (buf == NULL || count <= 0) return -1;
    const char *test_data = "test input from stdin\n";
    int data_len = 0;
    while (test_data[data_len]) data_len++;
    int read_len = count < data_len ? count : data_len;
    // simple copy to avoid dependence on libc memcpy signature
    char *dst = (char*)buf;
    for (int i = 0; i < read_len; i++) dst[i] = test_data[i];
    return read_len;
}

int strlen(const char *s) {
    if (!s) return 0;
    int n = 0;
    while (s[n]) n++;
    return n;
}
