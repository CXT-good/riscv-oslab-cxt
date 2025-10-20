// mm.h 文件内容
#ifndef _MM_H_
#define _MM_H_

#include "types.h"

#define PAGE_SIZE       4096    //页大小：4KB
#define PAGE_SHIFT      12      //页大小对应的位移数（2^12 = 4096）,因此Offset占12位

/* Page alignment macros */
#define PGROUNDUP(sz)   (((sz) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))  // 向上页对齐
#define PGROUNDDOWN(a)  ((a) & ~(PAGE_SIZE - 1))                     // 向下页对齐

//页表项标志位
#define PTE_V (1L << 0)   /* Valid */
#define PTE_R (1L << 1)   /* Read */
#define PTE_W (1L << 2)   /* Write */
#define PTE_X (1L << 3)   /* Execute */
#define PTE_U (1L << 4)   /* User */

//页表项操作宏
#define PTE_PPN_SHIFT   10  // 物理页号在PTE中的偏移
#define PTE_PA(pte)     (((pte) >> PTE_PPN_SHIFT) << PAGE_SHIFT)  // 从PTE提取物理地址

/* Virtual address to VPN extraction */
// 提取指定层级的虚拟页号
#define VA2VPN(va, level) (((va) >> (12 + 9 * (level))) & 0x1FF)

/* Physical Memory Manager */
//物理内存管理器函数声明
void pmm_init(void);             // 物理内存管理器初始化
void* alloc_page(void);          // 分配一页物理内存
void free_page(void* page);      // 释放一页物理内存

/* PMM 统计变量 - 外部声明 */
extern int total_pages;          //总页数
extern int used_pages;           //已使用页数

/* Virtual Memory Manager */
//虚拟内存管理器
typedef uint64_t* pagetable_t;    // 页表类型（指向页表基地址）
typedef uint64_t pte_t;           // 页表项类型

pagetable_t create_pagetable(void);   // 创建新页表
int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm);  // 映射虚拟地址到物理地址
pte_t* walk_lookup(pagetable_t pt, uint64_t va);        // 查找虚拟地址对应的PTE
void free_pagetable(pagetable_t pt);   // 释放页表
void dump_pagetable(pagetable_t pt);   // 打印页表内容

/* Kernel Virtual Memory */
//内核虚拟内存函数
void kvminit(void);    // 初始化内核虚拟内存空间
void kvminithart(void);  // 为当前CPU hart初始化内核页表

#endif