// main.c 文件顶部添加
#include "printf.h"
#include "console.h"
#include "mm.h"
#include <stddef.h>
// 外部符号声明
extern char etext[], end[];

// 添加 PMM 统计变量的外部声明
extern int total_pages;
extern int used_pages;

// 在这里定义 kernel_base
uint64_t kernel_base = 0x80000000;

// 简单的断言宏
#define assert(condition) \
    do { \
        if (!(condition)) { \
            printf("Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            while(1); \
        } \
    } while(0)

/* ==================== 物理内存分配器测试 ==================== */
void test_physical_memory(void) {
    printf("=== Physical Memory Manager Test ===\n");
    
    // 测试基本分配和释放
    printf("1. Testing basic allocation and free...\n");
    void *page1 = alloc_page();
    void *page2 = alloc_page();
    
    printf("   Allocated pages: page1=%p, page2=%p\n", page1, page2);
    
    if (page1 == NULL || page2 == NULL) {
        printf("   ERROR: Failed to allocate pages!\n");
        return;
    }
    
    assert(page1 != NULL);
    assert(page2 != NULL);
    assert(page1 != page2);
    assert(((uint64_t)page1 & 0xFFF) == 0);  // 页对齐检查
    assert(((uint64_t)page2 & 0xFFF) == 0);
    
    printf("   ✓ Page alignment check passed\n");

    // 测试数据写入和读取
    printf("2. Testing data read/write...\n");
    *(volatile uint32_t*)page1 = 0x12345678;
    uint32_t read_value = *(volatile uint32_t*)page1;
    printf("   Write test: wrote 0x%x, read 0x%x\n", 0x12345678, read_value);
    assert(read_value == 0x12345678);
    
    printf("   ✓ Data read/write test passed\n");

    // 测试释放和重新分配
    printf("3. Testing free and reallocation...\n");
    free_page(page1);
    void *page3 = alloc_page();
    
    printf("   Reallocated page: page3=%p (original page1=%p)\n", page3, page1);
    // page3 等于 page1（后进先出分配策略）
    
    if (page3) {
        // 测试重新分配的页面是清零的
        for (int i = 0; i < 4; i++) {
            uint64_t value = *(volatile uint64_t*)((char*)page3 + i * 64);
            if (value != 0) {
                printf("   ERROR: Page not cleared at offset %d: 0x%llx\n", i * 64, value);
            }
        }
        printf("   ✓ Page clearing after allocation verified\n");
    }

    // 清理
    free_page(page2);
    if (page3) free_page(page3);
    
    printf("✓ Physical memory test completed successfully!\n\n");
}

/* ==================== 页表功能测试 ==================== */
void test_pagetable(void) {
    printf("=== Page Table Functionality Test ===\n");
    
    printf("1. Creating page table...\n");
    pagetable_t pt = create_pagetable();
    if (pt == NULL) {
        printf("   ERROR: Failed to create page table!\n");
        return;
    }
    printf("   ✓ Page table created at %p\n", pt);

    // 分配物理页用于映射测试
    printf("2. Allocating physical pages for mapping...\n");
    uint64_t pa1 = (uint64_t)alloc_page();
    uint64_t pa2 = (uint64_t)alloc_page();
    uint64_t pa3 = (uint64_t)alloc_page();
    
    if (pa1 == 0 || pa2 == 0 || pa3 == 0) {
        printf("   ERROR: Failed to allocate physical pages!\n");
        return;
    }
    printf("   ✓ Physical pages allocated: %p, %p, %p\n", (void*)pa1, (void*)pa2, (void*)pa3);

    // 测试基本映射
    printf("3. Testing basic page mapping...\n");
    uint64_t va1 = 0x40000000;  // 测试虚拟地址1
    uint64_t va2 = 0x50000000;  // 测试虚拟地址2
    uint64_t va3 = 0x60000000;  // 测试虚拟地址3
    
    int result1 = map_page(pt, va1, pa1, PTE_R | PTE_W);
    int result2 = map_page(pt, va2, pa2, PTE_R | PTE_X);
    int result3 = map_page(pt, va3, pa3, PTE_R );
    
    printf("   Mapping results: %d, %d, %d\n", result1, result2, result3);
    
    if (result1 != 0 || result2 != 0 || result3 != 0) {
        printf("   ERROR: Failed to create page mappings!\n");
        return;
    }
    printf("   ✓ Page mappings created successfully\n");

    // 测试地址转换查找
    printf("4. Testing address translation...\n");
    pte_t *pte1 = walk_lookup(pt, va1);
    pte_t *pte2 = walk_lookup(pt, va2);
    pte_t *pte3 = walk_lookup(pt, va3);
    
    printf("   Lookup results: %p, %p, %p\n", pte1, pte2, pte3);
    
    if (pte1 == NULL || pte2 == NULL || pte3 == NULL) {
        printf("   ERROR: Failed to find page table entries!\n");
        return;
    }
    
    if (!(*pte1 & PTE_V) || !(*pte2 & PTE_V) || !(*pte3 & PTE_V)) {
        printf("   ERROR: Page table entries not valid!\n");
        return;
    }
    printf("   ✓ Page table entries found and valid\n");

    // 显示页表内容
    printf("5. Dumping page table contents...\n");
    dump_pagetable(pt);

    // 清理
    printf("6. Cleaning up...\n");
    free_page((void*)pa1);
    free_page((void*)pa2);
    free_page((void*)pa3);
    
    printf("✓ Page table test completed successfully!\n\n");
}

// 伙伴系统测试函数
void test_buddy_system(void) {
    printf("=== Buddy System Comprehensive Test ===\n");
    
    // 初始化伙伴系统
    printf("1. Initializing buddy system...\n");
    buddy_init();
    buddy_dump();
    
    // 测试不同阶数的分配
    printf("2. Testing allocations of different orders...\n");
    void *blocks[BUDDY_MAX_ORDER + 1] = {0};
    
    // 分配一些小阶数的块
    for (int order = 0; order <= 2; order++) {
        printf("   Allocating order %d...\n", order);
        blocks[order] = buddy_alloc(order);
        if (blocks[order]) {
            printf("   ✓ Allocated order %d (%d pages) at %p\n", 
                   order, 1 << order, blocks[order]);
        } else {
            printf("   ✗ Failed to allocate order %d\n", order);
        }
        buddy_dump();
    }
    
    // 测试释放和合并
    printf("3. Testing free and merge...\n");
    for (int order = 0; order <= 2; order++) {
        if (blocks[order]) {
            printf("   Freeing order %d at %p...\n", order, blocks[order]);
            buddy_free(blocks[order], order);
            printf("   ✓ Freed order %d\n", order);
            blocks[order] = NULL;
            buddy_dump();
        }
    }
    
    // 测试连续分配和释放
    printf("4. Testing continuous allocation pattern...\n");
    void *test_blocks[4];
    for (int i = 0; i < 4; i++) {
        test_blocks[i] = buddy_alloc(0);  // 分配4个单页
        if (test_blocks[i]) {
            printf("   ✓ Allocated single page %d at %p\n", i, test_blocks[i]);
        } else {
            printf("   ✗ Failed to allocate single page %d\n", i);
        }
    }
    buddy_dump();
    
    // 释放一些块创建碎片
    printf("5. Creating fragmentation...\n");
    buddy_free(test_blocks[1], 0);
    buddy_dump();
    buddy_free(test_blocks[3], 0);
    printf("   Freed pages 1 and 3 to create fragmentation\n");
    buddy_dump();
    
    // 分配一个2页的块
    printf("6. Testing merge by allocating 2-page block...\n");
    void *two_page_block = buddy_alloc(1);
    if (two_page_block) {
        printf("   ✓ Allocated 2-page block at %p (demonstrates merge)\n", two_page_block);
        buddy_dump();
        buddy_free(two_page_block, 1);
        printf("   ✓ Freed 2-page block\n");
    } else {
        printf("   ✗ Failed to allocate 2-page block\n");
    }
    buddy_dump();
    
    // 清理剩余块
    buddy_free(test_blocks[0], 0);
    buddy_dump();
    buddy_free(test_blocks[2], 0);
    buddy_dump();
    
    printf("✓ Buddy system test completed successfully!\n\n");
}
/* ==================== 主测试函数 ==================== */
// 在 main.c 的 run_all_tests 函数中添加更多调试信息
void run_all_tests(void) {
    // 显示系统信息
    printf("System Information:\n");
    printf("  Kernel text: 0x80000000 - %p\n", (void*)etext);
    printf("  Kernel data: %p - %p\n", (void*)etext, (void*)&end);
    printf("  Page size: %d bytes\n", PAGE_SIZE);
    
    printf("\n=== Phase 1: Physical Memory Test ===\n");
    test_physical_memory();
    
    printf("\n=== Phase 2: Virtual Memory Setup ===\n");
    printf("Initializing kernel page table...\n");
    kvminit();
    
    printf("Enabling virtual memory...\n");
    kvminithart();
    
    printf("\n=== Phase 3: Page Table Test ===\n");
    test_pagetable();
}

/* ==================== 主函数 ==================== */
int main(void) {
    // 初始化控制台
    console_init();
    
    printf("\n");
    printf("RV64 OS Booted Successfully!\n");
    printf("Memory management initialized by bootloader\n");
    
    // 运行所有测试
    run_all_tests();

    test_buddy_system();

    return 0;
}