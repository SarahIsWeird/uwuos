#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stddef.h>

void putch(int fd, char c);
void flush(int fd);

#endif
