#include "phys.h"

#include <stdint.h>
#include <stddef.h>

#include "../util.h"
#include "../term/term.h"

#define PAGE_SIZE 4096
#define PAGES (UINT32_MAX / PAGE_SIZE) // 4GB in PAGE_SIZE chunks
#define BITMAP_SIZE (PAGES / 32) // 32 pages per uint32_t

#define NTH_BIT(n) (1 << (n))
#define MASK(n) NTH_BIT(32 - n)
#define CLEAR_MASK(n) (~MASK(n))

static uint32_t mem_bitmap[BITMAP_SIZE];
static uint32_t free_pages = 0;

void memtest() {
    while (free_pages) {
        uint8_t *mem = (uint8_t *) phys_alloc(0, 0xffffffff);

        // Write some data

        for (uint32_t i = 0; i < PAGE_SIZE; i++) {
            mem[i] = i & 0xff;
        }

        // print("Wrote 4kB to ", 13);
        // dumpdword((uint32_t) mem);

        mem[1] = 0x01;
        if (mem[0] != 0x00) goto fail;

        for (uint32_t i = 1; i < PAGE_SIZE; i++) {
            mem[i - 1] = (i - 1) & 0xff;
            
            if (mem[i] != (i & 0xff)) goto fail;
        }

        // print(" and read it correctly\n", 23);
    }

    // print("Yay! uwu", 8);

    while (1);
fail:
    // print(", but read an incorrect value back :(", 37);

    while (1);
}

void phys_init(mb_info_t *mb_info, void *kernel_start, void *kernel_end) {
    uint32_t mmap_length = mb_info->mmap.length;
    mmap_addr_range_t *mmap_addr = mb_info->mmap.addr;

    memset(mem_bitmap, 0xff, BITMAP_SIZE);

    for (uint32_t i = 0; i < mmap_length;) {
        uint64_t base_addr = mmap_addr->base_addr;
        uint64_t length = mmap_addr->length;
        uint64_t limit_addr = base_addr + length;

        // *technically* skips some ram by casting to uint32_t, but only above like 17TB and we don't support that much anyways
        // we also ignore any partial pages, because again, we can't store that, and it's only a page or two at most in total
        uint32_t pages = (uint32_t) (length / PAGE_SIZE);

        if ((mmap_addr->type == AR_AVAILABLE) && (limit_addr <= 0xffffffff)) {
            void *page_addr = (void *) (uint32_t) base_addr;

            for (uint32_t page = 0; page < pages; page++) {
                phys_free(page_addr);
                page_addr += PAGE_SIZE;
            }
        }

        uint32_t actual_size = mmap_addr->size + 4; // +4 because the size itself is't part of the size. bruh
        i += actual_size;

        uint32_t next_loc = (uint32_t) mmap_addr + actual_size;
        mmap_addr = (mmap_addr_range_t*) next_loc;
    }

    while (kernel_start < kernel_end) {
        phys_mark_used(kernel_start);
        kernel_start += PAGE_SIZE;
    }

    // Reserve some multiboot structures we still need
    phys_mark_used(mb_info);

    if (mb_info->flags.mods) {
        phys_mark_used(mb_info->mods.addr); // We're assuming that there are at most 1024 multiboot modules.. which is reasonable imo

        mod_t *mod = mb_info->mods.addr;
        for (size_t i = 0; i < mb_info->mods.count; i++) {
            phys_mark_used(mod);

            for (void *addr = mod->start; addr < mod->end; addr += PAGE_SIZE) {
                phys_mark_used(addr);
            }

            mod++;
        }
    }

    phys_mark_used(0); // Make sure the NULL pointer can never be allocated
}

ssize_t find_free_memblock(uint32_t lower, uint32_t upper) {
    size_t l = lower >> 17; // page aligned and bitmap aligned
    size_t u = upper >> 17;

    for (size_t i = l; i <= u; i++) {
        if (mem_bitmap[i] != 0xffffffff) { // skip the entire block if no page is free
            return (ssize_t) i;
        }
    }

    return -1;
}

void *phys_alloc(uint32_t lower, uint32_t upper) {
    ssize_t free_block_i = find_free_memblock(lower, upper);
    if (free_block_i == -1) return NULL;

    uint32_t block = mem_bitmap[free_block_i];

    for (volatile int i = 0; i < 32; i++) { // we need volatile here, or GCC optimises out phys_mark_used.. what
        if (block & MASK(i)) continue;

        uint32_t addr = (free_block_i * 32 + i) * PAGE_SIZE;

        phys_mark_used((void *) addr);
        return (void *) addr;
    }

    // we shouldn't be here... this could happen if there's an interrupt i guess, but we'll see if it crops up
    term_set_active(0);
    term_set_color(0, LIGHT_RED, WHITE);
    term_print(0, "fatal error: no free page despite find_free_memblock succeeding! :(\n");

    while (1);
}

void *phys_kernel_alloc() {
    return phys_alloc(KERNEL_LOWER, KERNEL_UPPER);
}

void *phys_user_alloc() {
    return phys_alloc(USER_LOWER, USER_UPPER);
}

void phys_mark_used(void *mem) {
    uint32_t page_no = (uint32_t) mem / PAGE_SIZE;
    uint32_t bitmap_index = page_no / 32;
    int bit_index = page_no % 32;

    if (bitmap_index >= BITMAP_SIZE) return; // We can't allocate what doesn't exist for us

    mem_bitmap[bitmap_index] |= MASK(bit_index);
    free_pages--;
}

void phys_free(void *mem) {
    uint32_t page_no = (uint32_t) mem / PAGE_SIZE;
    uint32_t bitmap_index = page_no / 32;
    int bit_index = page_no % 32;

    if (bitmap_index >= BITMAP_SIZE) return; // We can't free what doesn't exist for us

    mem_bitmap[bitmap_index] &= CLEAR_MASK(bit_index);
    free_pages++;
}

uint32_t phys_get_avail() {
    return free_pages * PAGE_SIZE;
}
