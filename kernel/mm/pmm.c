#include "mm.h"
#include "printf.h"
// 移除 #include "spinlock.h"

#define PMM_MAX_PAGES   512     /* Manage 2MB memory */

struct page {
    struct page* next;
};

static struct page* free_list = NULL;
static uint64_t pmm_base;
static uint64_t pmm_end;
 int total_pages = 0;
 int used_pages = 0;

void pmm_init(void) {
    /* Available memory: from end of kernel to 0x80400000 */
    extern char end[];  /* Defined in kernel.ld */
    pmm_base = PGROUNDUP((uint64_t)&end);
    pmm_end = 0x80400000;
    
    printf("PMM: initializing physical memory [%p, %p)\n", 
           (void*)pmm_base, (void*)pmm_end);
    
    /* Add all available pages to free list */
    uint64_t start = PGROUNDUP(pmm_base);
    for (uint64_t pa = start; pa + PAGE_SIZE <= pmm_end; pa += PAGE_SIZE) {
        struct page* page = (struct page*)pa;
        page->next = free_list;
        free_list = page;
        total_pages++;
    }
    
    printf("PMM: initialized %d free pages\n", total_pages);
}

void* alloc_page(void) {
    if (!free_list) {
        printf("PMM: out of memory!\n");
        return NULL;
    }
    
    struct page* page = free_list;
    free_list = free_list->next;
    used_pages++;
    
    /* Clear the page - 使用 uint64_t 替代 size_t */
    for (uint64_t i = 0; i < PAGE_SIZE; i += sizeof(uint64_t)) {
        *(volatile uint64_t*)((char*)page + i) = 0;
    }
    
    return (void*)page;
}

void free_page(void* page) {
    if ((uint64_t)page % PAGE_SIZE != 0) {
        printf("PMM: free_page: unaligned address %p\n", page);
        return;
    }
    
    struct page* p = (struct page*)page;
    p->next = free_list;
    free_list = p;
    used_pages--;
}