//物理内存管理器
#include "mm.h"
#include "printf.h"
// 移除 #include "spinlock.h"

#define PMM_MAX_PAGES   512     /* Manage 2MB memory */

/* Declare the linker-provided kernel_base symbol and compute PHYSTOP as an address */
uint64_t kernel_base = 0x80000000;
#define PHYSTOP ((uint64_t)kernel_base + 128*1024*1024)

struct page {
    struct page* next;// 页结构体，仅包含指向下一页的指针
    // 这个结构体直接存储在物理页的开始处，用于构建空闲链表
};

static struct page* free_list = NULL;// 空闲页链表头指针
static uint64_t pmm_base;// 物理内存管理区域的起始地址
static uint64_t pmm_end;// 物理内存管理区域的结束地址
 int total_pages = 0;// 总页数
 int used_pages = 0;// 已使用页数

void pmm_init(void) {
    /* Available memory: from end of kernel to 0x80400000 */
    extern char end[]; // 声明外部变量end，这个变量在链接脚本kernel.ld中定义，表示内核的结束地址
    pmm_base = PGROUNDUP((uint64_t)&end);// 将内核结束地址向上页对齐，作为物理内存管理的起始地址
    pmm_end = PHYSTOP;// 设置物理内存管理的结束地址为PHYSTOP
    
    printf("PMM: initializing physical memory [%p, %p)\n", 
           (void*)pmm_base, (void*)pmm_end);
    
    /* Add all available pages to free list */
    uint64_t start = PGROUNDUP(pmm_base);
    for (uint64_t pa = start; pa + PAGE_SIZE <= pmm_end; pa += PAGE_SIZE)
    // 遍历从start到pmm_end的所有页
     {
        struct page* page = (struct page*)pa;// 将物理地址转换为页结构体指针
        page->next = free_list;// 将当前页的next指针指向当前的空闲链表头,所以表头的页是最后被添加的页，表头的页的地址最大
        free_list = page;
        total_pages++;
    }
    
    printf("PMM: initialized %d free pages\n", total_pages);
}

void* alloc_page(void) {
    if (!free_list) {// 如果空闲链表为空，返回NULL表示内存耗尽
        printf("PMM: out of memory!\n");
        return NULL;
    }
    
    struct page* page = free_list;// 从链表头部获取一页
    free_list = free_list->next;// 更新链表头指针到下一页
    used_pages++;// 增加已使用页数计数
    
    /* Clear the page - 使用 uint64_t 替代 size_t */
    for (uint64_t i = 0; i < PAGE_SIZE; i += sizeof(uint64_t)) {// 以64位为单位清零整个页，确保分配的内存是干净的
        *(volatile uint64_t*)((char*)page + i) = 0;
    }
    
    return (void*)page;// 返回分配页的指针
}

void free_page(void* page) {
    if ((uint64_t)page % PAGE_SIZE != 0) {// 检查地址是否页对齐，不对齐则报错返回
        printf("PMM: free_page: unaligned address %p\n", page);
        return;
    }
    
    struct page* p = (struct page*)page;
    p->next = free_list;// 将释放的页插入到空闲链表头部
    free_list = p;// 更新链表头指针
    used_pages--;// 减少已使用页数计数
}