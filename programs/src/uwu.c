#include <stdint.h>

#include "syscalls.h"

uint16_t *text_buffer = (uint16_t *) 0xb8000;
int i = 0;

void _start() {
    int j = i + 5;
    for (; i < j; i++) {
        putch(0, '0' + i);
    }

    flush(0);

    while (1);
}
