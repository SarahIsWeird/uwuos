#include "scheduler.h"

#include <stdbool.h>

#include "../mem/phys.h"
#include "../mem/virt.h"
#include "../util.h"
#include "../term/term.h"
#include "../timer/timer.h"
#include "../multiboot.h"

#include "elf.h"

static int tasks = 0;
static task_t *first_task = NULL;
static task_t *current_task = NULL;
static int timer_id;

void putch(char);
void print(const char *, uint32_t);

void load_elf(void *addr, size_t length) {
    elf_header_t *header = (elf_header_t *) addr;

    const uint8_t *ident = header->ident;

    bool magic_correct = true;

    if (ident[EI_MAG0] != ELFMAG0) magic_correct = false;
    if (ident[EI_MAG1] != ELFMAG1) magic_correct = false;
    if (ident[EI_MAG2] != ELFMAG2) magic_correct = false;
    if (ident[EI_MAG3] != ELFMAG3) magic_correct = false;

    if (!magic_correct) {
        term_printf(0, "Invalid ELF magic!\n");
        return;
    }

    if (ident[EI_CLASS] != ELFCLASS32) {
        term_printf(0, "Only 32-bit ELF files are supported (Expected class %d, but got %d)\n", ELFCLASS32, ident[EI_CLASS]);
        return;
    }

    if (ident[EI_DATA] != ELFDATA2LSB) {
        term_printf(0, "Only LSB ELF files are suppoted (Expected data %d, but got %d)\n", ELFDATA2LSB, ident[EI_DATA]);
        return;
    }

    if (header->type != ET_EXEC) {
        term_printf(0, "Only executable ELF files can be loaded (Expected type %d, but got %d)\n", ET_EXEC, header->type);
        return;
    }

    if (header->machine != EM_386) {
        term_printf(0, "Only i386 ELF files can be loaded (Expected type %d, but got %d)\n", EM_386, header->machine);
        return;
    }

    if (header->version != EV_CURRENT) {
        term_printf(0, "The ELF file has an unsupported version (Expected version %d, but got %d)\n", EV_CURRENT, header->version);
        return;
    }
    if (header->entry == 0) {
        term_printf(0, "The ELF file has an entry of 0!\n");
        return;
    }

    if (header->phoff == 0 || header->phnum == 0) {
        term_printf(0, "The ELF file has no program header!\n");
        return;
    }

    virt_context_t *virt_context = virt_new_context();

    void *ph_addr = addr + header->phoff;
    for (uint16_t i = 0; i < header->phnum; i++) {
        elf_program_header_t *ph = (elf_program_header_t *) ph_addr;
        ph_addr += header->phentsize; // prepare addr for next iteration

        if (ph->type != PT_LOAD) { // FIXME: handle other types
            term_printf(0, "Unknown segment type: %u\n", ph->type);
            continue;
        }

        // TODO: check alignment somehow?

        if (ph->filesz > ph->memsz) {
            term_printf(0, "The file size (%u) is bigger than the memory size (%u)!\n", ph->filesz, ph->memsz);
            return;
        }

        if ((ph->offset + ph->filesz) > length) {
            term_printf(0, "The ELF file points to outside of itself! (Offset %u + size %u, but actual size is only %u)\n", ph->offset, ph->filesz, length);
            return;
        }

        virt_context_t *kernel_ctx = get_kernel_ctx();

        size_t written = 0;
        void *last_addr = (void *) ph->vaddr;

        while (written < ph->filesz) {
            size_t remaining = ph->filesz - written;

            if (remaining > 4096) remaining = 4096;

            uint32_t virt_dest = ph->vaddr + written;
            void *uaddr = virt_user_alloc_at(virt_context, (void *) virt_dest);
            virt_map(kernel_ctx, (uint32_t) uaddr, (uint32_t) uaddr, P_PRESENT | P_WRITABLE);
            last_addr = (void *) virt_dest;

            memcpy(uaddr, remaining, addr + ph->offset + written);
            written += remaining;

            if (written >= ph->filesz) {
                memset(uaddr + remaining, 0, 4096 - remaining);
            }
        }

        if (ph->filesz < ph->memsz) {
            uint32_t zero_count = ph->memsz - written;

            for (size_t i = 0; i < zero_count; i += 4096) {
                void *virt_dest = last_addr + 4096 + i;
                void *uaddr = virt_user_alloc_at(virt_context, virt_dest);
                virt_map(kernel_ctx, (uint32_t) uaddr, (uint32_t) uaddr, P_PRESENT | P_WRITABLE);

                memset(uaddr + i, 0, 4096);
            }
        }
    }

    init_task((void *) header->entry, virt_context);
}

void load_binary_module(mb_info_t *mb_info) {
    mod_t *modules = mb_info->mods.addr;
    
    size_t mod0_length = modules->end - modules->start;
    void *dst = (void *) 0x200000;

    for (size_t i = 0; i < mod0_length; i += 4096) {
        phys_mark_used(dst + i);
    }

    phys_mark_used(mb_info);

    memcpy(dst, mod0_length, modules->start);
    init_task(dst, virt_new_context());
}

void init_multitasking(mb_info_t *mb_info) {
    timer_id = get_timer(10000); // 10ms

    if (mb_info->mods.count == 0) {
        term_set_active(0);
        term_set_color(0, LIGHT_RED, WHITE);
        term_print(0, "No multiboot modules provided :(\n");

        while (1);
    }

    mod_t *mod = mb_info->mods.addr;

    for (uint32_t i = 0; i < mb_info->mods.count; i++) {
        load_elf(mod->start, (uint32_t) mod->end - (uint32_t) mod->start);
        mod++;
    }

    current_task = NULL;
}

void place_canary(void *addr) {
    *((uint32_t *) addr) = CANARY;
}

int check_canary(void *addr) {
    return *((uint32_t *) addr) == CANARY;
}

void on_canary_check_failed(uint32_t task_id, const char *stack_type) {
    term_set_active(0);

    term_set_color(0, LIGHT_RED, WHITE);

    term_print(0, "fatal error: stack overflow detected! task id: ");
    term_printn(0, task_id, 10);
    term_print(0, ", stack type: ");
    term_print(0, stack_type);

    while (1);
}

void init_task(void *fn, virt_context_t *virt_context) {
    task_t *task = (task_t *) phys_kernel_alloc();
    task->id = tasks++;

    task->virt_context = virt_context;
    task->stack = (uint32_t *) phys_kernel_alloc();
    task->user_stack = (uint32_t *) virt_user_alloc(task->virt_context);

    if (first_task == NULL) {
        first_task = task;
    }

    if (current_task != NULL) {
        current_task->next = task;
    }
    
    task->next = first_task;
    current_task = task;

    place_canary(task->stack);
    // place_canary(task->user_stack);

    uint32_t esp = (uint32_t) task->stack + 4096 - sizeof(cpu_state_t);
    uint32_t esp_before_pushad = (uint32_t) task->stack + 4096 - sizeof(uint32_t) * 8;

    cpu_state_t new_state = {
        .edi = 0,
        .esi = 0,
        .ebp = 0,
        .esp = esp_before_pushad,
        .ebx = 0,
        .edx = 0,
        .ecx = 0,
        .eax = 0,

        .interrupt = 0,
        .error_code = 0,

        .eip = (uint32_t) fn,
        .cs = 0x18 | 0x03,
        .eflags = 0x202,
        .user_esp = (uint32_t) task->user_stack + 4096,
        .ss = 0x20 | 0x03,
    };

    cpu_state_t *state = (cpu_state_t *) esp;
    *state = new_state;
    task->state = (cpu_state_t *) esp;
}

void check_canaries(task_t *task) {
    if (!check_canary(task->stack)) {
        on_canary_check_failed(current_task->id, "kernel stack");
    }

    // if (!check_canary(task->user_stack)) {
    //     on_canary_check_failed(current_task->id, "user stack");
    // }
}

cpu_state_t *schedule(cpu_state_t *current_state) {
    if (!timer_elapsed(timer_id)) return current_state;

    if (current_task != NULL) {
        check_canaries(current_task);

        current_task->state = current_state;
        current_task = current_task->next;
    } else {
        current_task = first_task;
    }

    current_state = current_task->state;

    return current_state;
}

uint32_t *get_current_page_dir() {
    return current_task->virt_context->page_dir;
}
