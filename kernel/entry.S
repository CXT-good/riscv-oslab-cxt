.section .text
.global _entry

_entry:
        # 设置栈指针
        la sp, stack0
        li t0, 4096
        add sp, sp, t0
        andi sp, sp, -16  # 确保16字节对齐

        # 清除BSS段
        la a0, _bss_start
        la a1, _bss_end
        j clear_bss_loop_test

clear_bss_loop:
        sd zero, 0(a0)
        addi a0, a0, 8

clear_bss_loop_test:
        blt a0, a1, clear_bss_loop

        # 直接跳转到主函数
        call main

spin:
        j spin