#include "interrupt_handler.h"

#include <stdbool.h>

#include "term/term.h"
#include "timer/timer.h"
#include "scheduling/scheduler.h"
#include "syscalls/syscall.h"
#include "mem/virt.h"

extern void update_tss(uint32_t esp);
extern void ack_irq();

static volatile bool last_int_was_irq = false;

cpu_state_t *handle_interrupt(cpu_state_t *current_state) {
    if (current_state->interrupt <= 0x1f) { // exception
        term_set_active(0);
        term_set_color(0, YELLOW, BLACK);
        term_printf(0, "Exception 0x%x, error code %u\n", current_state->interrupt, current_state->error_code);

        while (1);
    }

    last_int_was_irq = (current_state->interrupt >= 0x20) && (current_state->interrupt <= 0x2f);

    cpu_state_t *old_state = current_state;

    if (current_state->interrupt == 0x20) {
        advance_timers();

        current_state = schedule(current_state);
        update_tss((uint32_t) (current_state + 1));
    } else if (current_state->interrupt == 0x69) {
        current_state = syscall(current_state);
    }

    if (current_state != old_state) {
        uint32_t *current_page_dir = get_current_page_dir();
        activate_context(current_page_dir);
    }

    if (last_int_was_irq) {
        ack_irq();
    }

    return current_state;
}
