// 虚拟内存管理
#include "mm.h"
#include "printf.h"
#include "console.h"

#ifndef PHYSTOP
#define PHYSTOP 0x88000000UL
#endif

// 内联的简单内存设置函数
static inline void simple_memset(void* dst, char value, uint64_t n) {
    char* ptr = (char*)dst;
    for (uint64_t i = 0; i < n; i++) {
        ptr[i] = value;
    }
}

// 修复的页表遍历和创建函数
static pte_t* walk_create(pagetable_t pt, uint64_t va, int alloc) {
    pagetable_t current_pt = pt;
    
    for(int level = 2; level > 0; level--) {
        uint64_t vpn = VA2VPN(va, level);
        pte_t* pte = &current_pt[vpn];
        
        if(*pte & PTE_V) {
            // 有效的PTE，进入下一级
            current_pt = (pagetable_t)PTE_PA(*pte);
        } else {
            if(!alloc) 
                return NULL;
                
            // 分配新页表页
            pagetable_t new_pt = alloc_page();
            if(!new_pt) 
                return NULL;
                
            simple_memset(new_pt, 0, PAGE_SIZE);
            
            // 修复：正确的PTE构造
            // PTE格式：PPN[53:10] | flags[9:0]
            // PPN = 物理地址 >> 12
            *pte = (((uint64_t)new_pt) >> 12) << 10 | PTE_V;
            current_pt = new_pt;
        }
    }
    
    return &current_pt[VA2VPN(va, 0)];
}

// 创建页表
pagetable_t create_pagetable(void) {
    pagetable_t pt = alloc_page();
    if(pt) {
        simple_memset(pt, 0, PAGE_SIZE);
    }
    return pt;
}

// 修复的映射函数
int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm) {
    if(va % PAGE_SIZE != 0 || pa % PAGE_SIZE != 0) {
        printf("VMM: unaligned address: va=%p, pa=%p\n", (void*)va, (void*)pa);
        return -1;
    }
    
    pte_t* pte = walk_create(pt, va, 1);
    if(!pte) {
        printf("VMM: failed to create page table entry for va=%p\n", (void*)va);
        return -1;
    }
    
    if(*pte & PTE_V) {
        printf("VMM: page already mapped: va=%p\n", (void*)va);
        return -1;
    }

    // 修复：正确的PTE构造
    *pte = ((pa >> 12) << 10) | perm | PTE_V;
    return 0;
}

// 页表查找函数
pte_t* walk_lookup(pagetable_t pt, uint64_t va) {
    pagetable_t current_pt = pt;
    
    for(int level = 2; level >= 0; level--) {
        pte_t* pte = &current_pt[VA2VPN(va, level)];
        if(!(*pte & PTE_V))
            return NULL;
        if(level == 0)
            return pte;
        current_pt = (pagetable_t)PTE_PA(*pte);
    }
    return NULL;
}

/* Kernel page table */
static pagetable_t kernel_pagetable = NULL;

extern char etext[];  /* Defined in kernel.ld */

// 调试函数：检查地址映射
void debug_address_mapping(uint64_t va) {
    if (!kernel_pagetable) {
        printf("DEBUG: Kernel page table not initialized\n");
        return;
    }
    
    pte_t* pte = walk_lookup(kernel_pagetable, va);
    if (pte && (*pte & PTE_V)) {
        uint64_t pa = PTE_PA(*pte);
        uint32_t perm = *pte & 0xFF;
        printf("DEBUG: VA %p -> PA %p perm=0x%x\n", 
               (void*)va, (void*)pa, perm);
    } else {
        printf("DEBUG: VA %p -> NOT MAPPED\n", (void*)va);
    }
}

// 修复的内核虚拟内存初始化
void kvminit(void) {
    printf("VMM: Starting kernel page table initialization\n");
    
    // 防止重复初始化
    if (kernel_pagetable != NULL) {
        printf("VMM: kernel page table already initialized\n");
        return;
    }
    
    kernel_pagetable = create_pagetable();
    if(!kernel_pagetable) {
        printf("VMM: ERROR: Failed to create page table\n");
        return;
    }
    printf("VMM: Page table created at %p\n", kernel_pagetable);
    
    /* 精确的内存映射 - 修复权限问题 */
    
    // 1. 映射内核代码段 (R+X权限)
    uint64_t kernel_base = 0x80000000;
    uint64_t code_end = (uint64_t)etext;
    
    printf("VMM: mapping kernel code [%p, %p) R+X\n", 
           (void*)kernel_base, (void*)code_end);
    
    for (uint64_t va = PGROUNDDOWN(kernel_base); va < PGROUNDUP(code_end); va += PAGE_SIZE) {
        if (map_page(kernel_pagetable, va, va, PTE_R | PTE_X) < 0) {
            printf("VMM: ERROR: Failed to map code page at %p\n", (void*)va);
            return;
        }
    }
    
    // 2. 映射内核数据段 (R+W权限)
    uint64_t data_start = PGROUNDUP(code_end);
    uint64_t data_end = PHYSTOP;
    
    printf("VMM: mapping kernel data [%p, %p) R+W\n",
           (void*)data_start, (void*)data_end);
    
    for (uint64_t va = data_start; va < data_end; va += PAGE_SIZE) {
        if (map_page(kernel_pagetable, va, va, PTE_R | PTE_W) < 0) {
            printf("VMM: ERROR: Failed to map data page at %p\n", (void*)va);
            return;
        }
    }
    
    // 3. 映射设备 (R+W权限)
    uint64_t uart_base = 0x10000000;
    printf("VMM: mapping UART device [%p, %p) R+W\n", 
           (void*)uart_base, (void*)(uart_base + PAGE_SIZE));
    
    if (map_page(kernel_pagetable, uart_base, uart_base, PTE_R | PTE_W) < 0) {
        printf("VMM: ERROR: Failed to map UART device\n");
    }

    // 4. 映射CLINT设备
    uint64_t clint_base = 0x2000000;
    printf("VMM: mapping CLINT device [%p, %p) R+W\n", 
           (void*)clint_base, (void*)(clint_base + 0x10000));
    
    // 映射CLINT的多个页
    for (uint64_t va = clint_base; va < clint_base + 0x10000; va += PAGE_SIZE) {
        if (map_page(kernel_pagetable, va, va, PTE_R | PTE_W) < 0) {
            printf("VMM: ERROR: Failed to map CLINT page at %p\n", (void*)va);
        }
    }
    
    printf("VMM: Kernel page table initialization completed\n");
    
    // 调试：检查关键地址的映射
    printf("\nVMM: Debug address mappings:\n");
    debug_address_mapping(0x80000000);  // 内核基地址
    debug_address_mapping(0x80001000);  // 内核代码
    debug_address_mapping(0xa00001882); // 问题地址
    debug_address_mapping((uint64_t)etext); // 代码结束
    printf("\n");
}

// 启用虚拟内存
void kvminithart(void) {
    if(!kernel_pagetable) {
        printf("VMM: kernel page table not initialized\n");
        return;
    }
    
    // 检查页表地址对齐
    if ((uint64_t)kernel_pagetable % PAGE_SIZE != 0) {
        printf("VMM: ERROR: page table not page-aligned: %p\n", kernel_pagetable);
        return;
    }
    
    /* SATP register format for Sv39 */
    uint64_t satp = (8L << 60) | ((uint64_t)kernel_pagetable >> 12);
    
    printf("VMM: Enabling virtual memory with satp=%p\n", (void*)satp);
    
    // 写入SATP寄存器并刷新TLB
    asm volatile("csrw satp, %0" : : "r"(satp));
    asm volatile("sfence.vma");
    
    printf("VMM: Virtual memory enabled successfully\n");
}