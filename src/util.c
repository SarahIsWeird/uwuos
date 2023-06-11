#include "util.h"

size_t strlen(const char *str) {
    size_t size = 0;

    while (*(str++)) {
        size++;
    }

    return size;
}

void memset(void *dest, uint8_t value, size_t size) {
    for (size_t i = 0; i < size; i++) {
        ((uint8_t *) dest)[i] = value;
    }
}

void memcpy(void *dst, size_t size, void *src) {
    uint8_t *byte_dst = (uint8_t *) dst;
    uint8_t *byte_src = (uint8_t *) src;

    for (size_t i = 0; i < size; i++) {
        *(byte_dst++) = *(byte_src++);
    }
}

int strncmp(const char *str1, const char *str2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int diff = (int) *str1 - (int) *str2;

        if (diff != 0) return diff;

        str1++;
        str2++;
    }

    return 0;
}
