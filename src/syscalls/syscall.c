#include "syscall.h"

#include "../term/term.h"

cpu_state_t *syscall(cpu_state_t *state) {
    switch (state->eax) {
        case 0:
            state->eax = (uint32_t) term_putch(state->ebx, state->ecx & 0xff);
            break;
        case 1:
            state->eax = (uint32_t) term_flush(state->ebx);
            break;
    }

    return state;
}
