#ifndef PIT_H
#define PIT_H

#include <stdint.h>

int pit_available();
uint32_t pit_timer_res();
int pit_init();

#endif
