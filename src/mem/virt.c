#include "virt.h"

#include "../util.h"
#include "phys.h"
#include "../term/term.h"

#define PAGE_SIZE 4096

static volatile virt_context_t *kernel_ctx;

virt_context_t *get_kernel_ctx() {
    return (virt_context_t *) kernel_ctx;
}

void virt_init() {
    kernel_ctx = virt_new_context();

    activate_context(kernel_ctx->page_dir);
    enable_paging();
}

virt_context_t *virt_new_context() {
    virt_context_t *ctx = phys_kernel_alloc();

    ctx->page_dir = phys_kernel_alloc();
    memset(ctx->page_dir, 0, 4096); // only missing mapping in the beginning

    for (uint32_t i = KERNEL_LOWER; i < KERNEL_UPPER; i += PAGE_SIZE) {
        virt_map(ctx, i, i, P_PRESENT | P_WRITABLE);
    }

    return ctx;
}

void virt_map(virt_context_t *ctx, uint32_t virtual, uint32_t physical, uint32_t flags) {
    uint32_t page_dir_idx = (uint32_t) virtual >> 22;
    uint32_t page_table_idx = (uint32_t) virtual >> 12 & 0x3ff;

    uint32_t *page_dir_entry = &ctx->page_dir[page_dir_idx];

    if ((*page_dir_entry & P_PRESENT) == 0) {
        uint32_t page_entry_addr = (uint32_t) phys_kernel_alloc();
        memset((void *) page_entry_addr, 0, PAGE_SIZE); // by default, there's no mapping in the page dir

        *page_dir_entry = page_entry_addr | flags;
    }

    uint32_t *page_table = (uint32_t *) (*page_dir_entry & P_ADDRESS_MASK);
    uint32_t *page_table_entry = &page_table[page_table_idx];

    *page_table_entry = (uint32_t) physical | flags;

    invalidate_page((void *) virtual);
}

void *virt_kernel_alloc() {
    void *addr = phys_kernel_alloc();

    return addr;
}

void *virt_user_alloc(virt_context_t *ctx) {
    void *addr = phys_user_alloc();

    virt_map(ctx, (uint32_t) addr, (uint32_t) addr, P_PRESENT | P_WRITABLE | P_USER_ACCESSABLE);

    return addr;
}

void *virt_user_alloc_at(virt_context_t *ctx, void *virt_addr) {
    void *addr = phys_user_alloc();

    virt_map(ctx, (uint32_t) virt_addr, (uint32_t) addr, P_PRESENT | P_WRITABLE | P_USER_ACCESSABLE);

    return addr;
}
