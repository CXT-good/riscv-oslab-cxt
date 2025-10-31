// kernel/mm/buddy.c - 修复版本
#include "mm.h"
#include "printf.h"
#include "buddy.h"

// 伙伴系统全局实例
static struct buddy_pool buddy_system;

// 将阶数转换为对应的页数
static inline int order_to_pages(int order) {
    return 1 << order;
}

// 计算地址在内存池中的页索引
static inline uint64_t addr_to_page_index(uint64_t addr) {
    if (addr < buddy_system.pool_start) return (uint64_t)-1;
    return (addr - buddy_system.pool_start) / PAGE_SIZE;
}

// 计算页索引对应的地址
static inline uint64_t page_index_to_addr(uint64_t index) {
    return buddy_system.pool_start + index * PAGE_SIZE;
}

// 获取伙伴块的页索引(异或)
static inline uint64_t get_buddy_index(uint64_t index, int order) {
    return index ^ (1 << order);
}

// 检查伙伴块是否空闲且可合并
int is_buddy_free(uint64_t index, int order) {
    uint64_t buddy_index = get_buddy_index(index, order);
    
    // 检查伙伴索引是否有效
    if (buddy_index >= buddy_system.total_pages) {
        return 0;
    }
    
    // 检查伙伴块是否在对应阶的空闲链表中
    struct list_head *pos;
    // uint64_t buddy_addr = page_index_to_addr(buddy_index);
    
    list_for_each(pos, &buddy_system.free_lists[order]) {
        uint64_t pos_addr = (uint64_t)pos;
        uint64_t pos_index = addr_to_page_index(pos_addr);
        
        if (pos_index == buddy_index) {
            return 1;
        }
    }
    
    return 0;
}

// 完整的伙伴系统初始化
void buddy_init(void) {
    printf("Buddy: starting initialization...\n");
    
    // 初始化空闲链表
    for (int i = 0; i <= BUDDY_MAX_ORDER; i++) {
        INIT_LIST_HEAD(&buddy_system.free_lists[i]);
    }
    
    // 设置内存池范围
    extern char end[];
    buddy_system.pool_start = PGROUNDUP((uint64_t)&end);
    
    // 使用固定的 PHYSTOP 定义
    #define BUDDY_PHYSTOP 0x88000000UL
    buddy_system.pool_size = BUDDY_PHYSTOP - buddy_system.pool_start;
    buddy_system.total_pages = buddy_system.pool_size / PAGE_SIZE;
    buddy_system.used_pages = 0;
    
    printf("Buddy: pool [%p, %p) size=%dKB, pages=%d\n",
           (void*)buddy_system.pool_start, 
           (void*)BUDDY_PHYSTOP,
           (int)(buddy_system.pool_size / 1024),
           (int)buddy_system.total_pages);
    
    // 暂时禁用位图
    buddy_system.bitmap = NULL;
    buddy_system.bitmap_size = 0;
    
    // 第一个可用地址就是 pool_start
    uint64_t data_start = buddy_system.pool_start;
    
    // 将整个可用内存作为一个大块添加到最大阶的空闲链表
    uint64_t available_size = BUDDY_PHYSTOP - data_start;
    uint64_t available_pages = available_size / PAGE_SIZE;
    
    printf("Buddy: available memory: %d pages from %p\n", 
           (int)available_pages, (void*)data_start);
    
    // 找到适合的最大阶数
    int max_order = BUDDY_MAX_ORDER;
    while (max_order >= 0) {
        uint64_t order_pages = order_to_pages(max_order);
        if (available_pages >= order_pages) {
            break;
        }
        max_order--;
    }
    
    if (max_order < 0) {
        printf("Buddy: ERROR: not enough memory for initialization\n");
        return;
    }
    
    // 将可用内存添加到对应阶的空闲链表
    struct list_head *block = (struct list_head*)data_start;
    INIT_LIST_HEAD(block);
    list_add(block, &buddy_system.free_lists[max_order]);
    
    printf("Buddy: initialized with max_order=%d, first block at %p (%d pages)\n", 
           max_order, (void*)data_start, order_to_pages(max_order));
    printf("Buddy: initialization completed successfully\n");
}

// 修复的分配函数
void* buddy_alloc(int order) {
    if (order < BUDDY_MIN_ORDER || order > BUDDY_MAX_ORDER) {
        printf("Buddy: invalid order %d\n", order);
        return NULL;
    }
    
    int current_order = order;
    
    // 寻找合适阶数的空闲块
    while (current_order <= BUDDY_MAX_ORDER) {
        if (!list_empty(&buddy_system.free_lists[current_order])) {
            break;
        }
        current_order++;
    }
    
    if (current_order > BUDDY_MAX_ORDER) {
        printf("Buddy: out of memory for order %d\n", order);
        return NULL;
    }
    
    // 从找到的链表中取出第一个块
    struct list_head *block = buddy_system.free_lists[current_order].next;
    list_del(block);
    
    uint64_t block_addr = (uint64_t)block;
    uint64_t block_index = addr_to_page_index(block_addr);
    
    // 如果找到的块比需要的大，进行分裂
    while (current_order > order) {
        current_order--;
        
        // 计算伙伴块
        uint64_t buddy_index = get_buddy_index(block_index, current_order);
        uint64_t buddy_addr = page_index_to_addr(buddy_index);
        
        // 检查伙伴索引是否有效
        if (buddy_index >= buddy_system.total_pages) {
            break;
        }
        
        // 将伙伴块添加到对应阶的空闲链表
        struct list_head *buddy = (struct list_head*)buddy_addr;
        INIT_LIST_HEAD(buddy);
        list_add(buddy, &buddy_system.free_lists[current_order]);
    }
    
    buddy_system.used_pages += order_to_pages(order);
    
    return (void*)block_addr;
}

// 修复的释放函数
void buddy_free(void* addr, int order) {
    if (addr == NULL || order < BUDDY_MIN_ORDER || order > BUDDY_MAX_ORDER) {
        printf("Buddy: invalid free parameters: addr=%p, order=%d\n", addr, order);
        return;
    }
    
    uint64_t block_addr = (uint64_t)addr;
    uint64_t current_index = addr_to_page_index(block_addr);
    
    // 验证地址有效性
    if (current_index >= buddy_system.total_pages) {
        printf("Buddy: ERROR: invalid address %p\n", addr);
        return;
    }
    
    int current_order = order;
    uint64_t merge_index = current_index;
    uint64_t merge_addr = block_addr;
    
    // 只要当前阶数小于最大阶数，就尝试合并
    while (current_order < BUDDY_MAX_ORDER) {
        uint64_t buddy_index = get_buddy_index(merge_index, current_order);
        
        // 检查伙伴索引是否有效
        if (buddy_index >= buddy_system.total_pages) {
            break;
        }
        
        // 检查伙伴块是否空闲且可合并
        if (!is_buddy_free(merge_index, current_order)) {
            break;
        }
        
        uint64_t buddy_addr = page_index_to_addr(buddy_index);
        
        // 从空闲链表中移除伙伴块
        struct list_head *buddy = (struct list_head*)buddy_addr;
        list_del(buddy);
        
        // 计算合并后的块地址（取两个块中较小的地址）
        if (buddy_index < merge_index) {
            merge_index = buddy_index;
            merge_addr = buddy_addr;
        }
        
        current_order++;
    }
    
    // 将合并后的块添加到对应阶的空闲链表
    struct list_head *block = (struct list_head*)merge_addr;
    INIT_LIST_HEAD(block);
    list_add(block, &buddy_system.free_lists[current_order]);
    
    buddy_system.used_pages -= order_to_pages(order);
}

// 简化的伙伴系统状态转储
void buddy_dump(void) {
    printf("=== Buddy System Status ===\n");
    printf("Pool: [%p, %p) total_pages=%d, used_pages=%d\n",
           (void*)buddy_system.pool_start, 
           (void*)(buddy_system.pool_start + buddy_system.pool_size),
           (int)buddy_system.total_pages, (int)buddy_system.used_pages);
    
    for (int i = 0; i <= BUDDY_MAX_ORDER; i++) {
        int count = 0;
        struct list_head *pos;
        list_for_each(pos, &buddy_system.free_lists[i]) {
            count++;
        }
        printf("Order %d (%d pages): %d free blocks\n", 
               i, order_to_pages(i), count);
    }
    printf("===========================\n");
}

// 获取总页数
uint64_t buddy_get_total_pages(void) {
    return buddy_system.total_pages;
}

// 获取已使用页数
uint64_t buddy_get_used_pages(void) {
    return buddy_system.used_pages;
}