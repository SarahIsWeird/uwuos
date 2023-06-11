#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#include "../mem/virt.h"
#include "../interrupt_handler.h"

#define CANARY 0xcafebabe

typedef struct task_t task_t;

typedef struct task_t {
    uint32_t id;
    cpu_state_t *state;
    task_t *next;
    uint32_t *stack;
    uint32_t *user_stack;
    virt_context_t *virt_context;
} task_t;

void init_multitasking();
void init_task(void *fn, virt_context_t *virt_context);
cpu_state_t *schedule(cpu_state_t *current_state);

uint32_t *get_current_page_dir();

#endif
