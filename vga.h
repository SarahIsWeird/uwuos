#ifndef UWUOS_VGA_H
#define UWUOS_VGA_H

#include <stdint.h>

void set_color(uint16_t color);
void clear();
void putch(char c);
void print(const char *str, size_t len);

#endif
