#ifndef SYSCALL_H
#define SYSCALL_H

#include "../interrupt_handler.h"

cpu_state_t *syscall(cpu_state_t *state);

#endif
