// mm.h 文件内容
#ifndef _MM_H_
#define _MM_H_

#include "types.h"

#define PAGE_SIZE       4096
#define PAGE_SHIFT      12

/* Page alignment macros */
#define PGROUNDUP(sz)   (((sz) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PGROUNDDOWN(a)  ((a) & ~(PAGE_SIZE - 1))

/* Page table entry flags */
#define PTE_V (1L << 0)   /* Valid */
#define PTE_R (1L << 1)   /* Read */
#define PTE_W (1L << 2)   /* Write */
#define PTE_X (1L << 3)   /* Execute */
#define PTE_U (1L << 4)   /* User */

/* Extract physical address from PTE */
#define PTE_PPN_SHIFT   10
#define PTE_PA(pte)     (((pte) >> PTE_PPN_SHIFT) << PAGE_SHIFT)

/* Virtual address to VPN extraction */
#define VA2VPN(va, level) (((va) >> (12 + 9 * (level))) & 0x1FF)

/* Physical Memory Manager */
void pmm_init(void);
void* alloc_page(void);
void free_page(void* page);

/* PMM 统计变量 - 外部声明 */
extern int total_pages;
extern int used_pages;

/* Virtual Memory Manager */
typedef uint64_t* pagetable_t;    /* Page table type */
typedef uint64_t pte_t;           /* Page table entry type */

pagetable_t create_pagetable(void);
int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm);
pte_t* walk_lookup(pagetable_t pt, uint64_t va);
void free_pagetable(pagetable_t pt);
void dump_pagetable(pagetable_t pt);

/* Kernel Virtual Memory */
void kvminit(void);
void kvminithart(void);

#endif