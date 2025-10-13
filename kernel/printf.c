#include "types.h"
#include "printf.h"
#include "console.h"
#include <stdarg.h>

static char digits[] = "0123456789abcdef";

// 数字转换函数
static void print_number(long long num, int base, int sign) {
    char buf[64];
    int i = 0;
    unsigned long long x;

    if(sign && (sign = (num < 0)))
        x = -num;
    else
        x = num;

    i = 0;
    do {
        buf[i++] = digits[x % base];
    } while((x /= base) != 0);

    if(sign)
        buf[i++] = '-';

    while(--i >= 0)
        console_putc(buf[i]);
}

// 指针输出函数
static void print_pointer(uint64_t ptr) {
    console_putc('0');
    console_putc('x');
    
    if(ptr == 0) {
        console_putc('0');
        return;
    }
    
    print_number(ptr, 16, 0);
}

// 主printf函数
int printf(const char *fmt, ...) {
    va_list ap;   // 可变参数列表指针，用于访问不定数量的参数
    int i;
    char c;
    char *s;      // 临时字符串指针，用于处理%s格式
    
    va_start(ap, fmt);
    
    for(i = 0; (c = fmt[i]) != '\0'; i++) {
        if(c != '%') {
            console_putc(c);
            continue;
        }
        
        i++; // 跳过'%'
        
        if(fmt[i] == '\0') {
            console_putc('%');
            break;
        }
        
        switch(fmt[i]) {
            case 'd': // 有符号十进制
                print_number(va_arg(ap, int), 10, 1);
                break;
                
            case 'u': // 无符号十进制
                print_number(va_arg(ap, unsigned int), 10, 0);
                break;
                
            case 'x': // 十六进制
                print_number(va_arg(ap, unsigned int), 16, 0);
                break;
                
            case 'p': // 指针
                print_pointer(va_arg(ap, uint64_t));
                break;
                
            case 'c': // 字符
                console_putc((char)va_arg(ap, int));
                break;
                
            case 's': // 字符串
                s = va_arg(ap, char*);
                if(s == NULL) {
                    console_puts("(null)");
                } else {
                    console_puts(s);
                }
                break;
                
            case '%': // 字面量%
                console_putc('%');
                break;
                
            default: // 未知格式符
                console_putc('%');
                console_putc(fmt[i]);
                break;
        }
    }
    
    va_end(ap);
    return 0;
}