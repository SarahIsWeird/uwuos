#include "timer.h"

#include "../term/term.h"
#include "../util.h"
#include "pit.h"

#define MAX_TIMERS 128
#define DEFAULT_TIMER_TYPE TIMER_PIT

static timer_type_t timer_type = TIMER_NONE;
static volatile timer_t timers[MAX_TIMERS]; // TODO: dynamically allocate memory
static size_t timer_count = 0;

void init_timer() {
    if (set_timer_type(DEFAULT_TIMER_TYPE)) {
        term_set_active(0);
        term_set_color(0, LIGHT_RED, WHITE);
        term_print(0, "Failed to set the timer type!");
        
        while (1);
    }
}

int is_timer_available(timer_type_t timer_type) {
    switch (timer_type) {
        case TIMER_PIT:
            return pit_available();
        default:
            return 0;
    }
}

int set_timer_type(timer_type_t new_timer_type) {
    if (!is_timer_available(new_timer_type)) return -1;
    if (timer_type != TIMER_NONE) return -1; // we don't allow reconfiguration at runtime yet

    timer_type = new_timer_type;

    switch (timer_type) {
        case TIMER_PIT:
            return pit_init();
        default:
            return -1;
    }
}

uint32_t get_timer_res() {
    switch (timer_type) {
        case TIMER_PIT:
            return pit_timer_res();
        default:
            return 0;
    }
}

int get_timer(int us) {
    timer_t timer = {
        .id = timer_count++,
        .interval = us,
        .current = 0,
    };

    timers[timer.id] = timer;

    return timer.id;
}

int timer_elapsed(int id) {
    volatile timer_t *timer = &timers[id];

    if (timer->current < timer->interval) return 0;

    timer->current = 0;
    return 1;
}

void advance_timers() {
    uint32_t us = get_timer_res();

    for (size_t i = 0; i < timer_count; i++) {
        timers[i].current += us;
    }
}
