// vmm.c 文件内容
#include "mm.h"
#include "printf.h"

// 内联的简单内存设置函数 - 使用 uint64_t 替代 size_t
static inline void simple_memset(void* dst, char value, uint64_t n) {
    char* ptr = (char*)dst;
    for (uint64_t i = 0; i < n; i++) {
        ptr[i] = value;
    }
}

// 修复 walk_create 函数
static pte_t* walk_create(pagetable_t pt, uint64_t va, int alloc) {
    pagetable_t current_pt = pt;
    
    for(int level = 2; level > 0; level--) {
        uint64_t vpn = VA2VPN(va, level);
        pte_t* pte = &current_pt[vpn];
        
        if(*pte & PTE_V) {
            current_pt = (pagetable_t)PTE_PA(*pte);
        } else {
            if(!alloc) 
                return NULL;
                
            pagetable_t new_pt = alloc_page();
            if(!new_pt) 
                return NULL;
                
            simple_memset(new_pt, 0, PAGE_SIZE);
            *pte = ((uint64_t)new_pt >> 2) | PTE_V;
            current_pt = new_pt;
        }
    }
    
    return &current_pt[VA2VPN(va, 0)];
}

pagetable_t create_pagetable(void) {
    pagetable_t pt = alloc_page();
    if(pt) {
        simple_memset(pt, 0, PAGE_SIZE);
    }
    return pt;
}

int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm) {
    if(va % PAGE_SIZE != 0 || pa % PAGE_SIZE != 0)
        return -1;
    
    pte_t* pte = walk_create(pt, va, 1);
    if(!pte || (*pte & PTE_V))
        return -1;
    
    *pte = (pa >> 2) | perm | PTE_V;
    return 0;
}

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

void dump_pagetable(pagetable_t pt) {
    printf("=== Page Table Dump ===\n");
    for(int i = 0; i < 10; i++) { /* Only show first 10 entries for brevity */
        if(pt[i] & PTE_V) {
            printf("PTE[%03d]: pa=0x%08llx perm=0x%llx\n",
                   i, PTE_PA(pt[i]), pt[i] & 0xFF);
        }
    }
}

/* Kernel page table - 只保留一个定义 */
static pagetable_t kernel_pagetable = NULL;

extern char etext[];  /* Defined in kernel.ld */

// 在 vmm.c 中修复 kvminit 函数
void kvminit(void) {
    kernel_pagetable = create_pagetable();
    if(!kernel_pagetable) {
        printf("VMM: failed to create kernel page table\n");
        return;
    }
    
    printf("VMM: kernel page table created at %p\n", kernel_pagetable);
    
    /* 简化映射：直接映射整个内核区域 */
    uint64_t kernel_start = 0x80000000;
    uint64_t kernel_end = 0x80400000;
    
    printf("VMM: mapping kernel region [%p, %p)\n", 
           (void*)kernel_start, (void*)kernel_end);
    
    int mapping_count = 0;
    for(uint64_t va = kernel_start; va < kernel_end; va += PAGE_SIZE) {
        // 对所有内核内存使用 RWX 权限以简化调试
        if(map_page(kernel_pagetable, va, va, PTE_R | PTE_W | PTE_X) == 0) {
            mapping_count++;
        } else {
            printf("VMM: failed to map page at va=%p\n", (void*)va);
            break;
        }
    }
    
    /* Map UART */
    if(map_page(kernel_pagetable, 0x10000000, 0x10000000, PTE_R | PTE_W) < 0) {
        printf("VMM: failed to map UART\n");
    } else {
        printf("VMM: UART mapped successfully\n");
    }
    
    printf("VMM: kernel page table initialized with %d mappings\n", mapping_count);
    
    // 调试：显示页表内容
    dump_pagetable(kernel_pagetable);
}

void kvminithart(void) {
    if(!kernel_pagetable) {
        printf("VMM: kernel page table not initialized\n");
        return;
    }
    
    /* SATP register format: MODE (4 bits) | ASID (16 bits) | PPN (44 bits) */
    /* MODE=8 for Sv39, PPN = physical page number >> 12 */
    uint64_t satp = (8L << 60) | ((uint64_t)kernel_pagetable >> 12);
    
    asm volatile("csrw satp, %0" : : "r"(satp));
    asm volatile("sfence.vma");
    
    printf("VMM: virtual memory enabled (satp=0x%llx)\n", satp);
}