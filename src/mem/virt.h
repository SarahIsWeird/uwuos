#ifndef VIRT_H
#define VIRT_H

#include <stdint.h>

#define P_PRESENT 0x001
#define P_WRITABLE 0x002
#define P_USER_ACCESSABLE 0x004
#define P_WRITE_THROUGH 0x008
#define P_DISABLE_CACHE 0x010
#define P_ACCESSED 0x020
#define P_DIRTY 0x040
#define P_SIZE 0x080
#define P_PAT 0x080
#define P_GLOBAL 0x100

#define P_ADDRESS_MASK (~0x3ff)

typedef struct virt_context_t {
    uint32_t *page_dir;
} virt_context_t;

void virt_init();

virt_context_t *virt_new_context();
void virt_map(virt_context_t *ctx, uint32_t virtual, uint32_t physical, uint32_t flags);
void *virt_kernel_alloc();
void *virt_user_alloc(virt_context_t *ctx);
void *virt_user_alloc_at(virt_context_t *ctx, void *virt_addr);

virt_context_t *get_kernel_ctx();

extern void enable_paging();
extern void activate_context(uint32_t *page_dir);
extern void invalidate_page(void *addr);

#endif
