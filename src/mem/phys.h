#ifndef MEM_H
#define MEM_h

#include "../multiboot.h"

#define KERNEL_LOWER 0
#define KERNEL_UPPER 0xfffffff // 256 MiB
#define USER_LOWER (KERNEL_UPPER + 1)
#define USER_UPPER 0xffffffff

void phys_init(mb_info_t *mb_info, void *kernel_start, void *kernel_end);

void *phys_alloc(uint32_t lower, uint32_t upper);
void *phys_kernel_alloc();
void *phys_user_alloc();

void phys_mark_used(void *mem);
void phys_free(void *mem);
uint32_t phys_get_avail();

#endif
