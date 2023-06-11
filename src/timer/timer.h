#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef enum timer_type_t {
    TIMER_NONE,
    TIMER_PIT,
} timer_type_t;

typedef struct timer_t {
    int id;
    uint32_t interval;
    uint32_t current;
} timer_t;

void init_timer();

int is_timer_available(timer_type_t timer_type);
int set_timer_type(timer_type_t timer_type);

// in us
uint32_t get_timer_res();

int get_timer(int us);
int timer_elapsed(int id);
// TODO: void remove_timer(int timer_id);

void advance_timers();

#endif
