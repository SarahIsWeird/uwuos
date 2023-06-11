#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>

typedef long int ssize_t;

void memcpy(void *dst, size_t size, void *src);
void memset(void *mem, uint8_t value, size_t size);
size_t strlen(const char *str);
int strncmp(const char *str1, const char *str2, size_t n);

#endif
