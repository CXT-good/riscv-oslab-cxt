# kernel/trap_entry.S
.section .text
.global trap_vector
.align 4
trap_vector:
    # 极简上下文保存 - 只保存必要的寄存器
    addi sp, sp, -16
    sd ra, 0(sp)
    
    # 调用C处理函数
    call trap_handler
    
    # 恢复上下文
    ld ra, 0(sp)
    addi sp, sp, 16
    
    # 返回
    mret