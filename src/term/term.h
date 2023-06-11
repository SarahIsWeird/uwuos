#ifndef TERM_H
#define TERM_H

#include <stdint.h>

#define TERM_WIDTH 80
#define TERM_HEIGHT 25
#define MAX_TERMS 4096

typedef enum term_color_t {
    BLACK = 0x0,
    BLUE = 0x1,
    GREEN = 0x2,
    CYAN = 0x3,
    RED = 0x4,
    MAGENTA = 0x5,
    BROWN = 0x6,
    LIGHT_GRAY = 0x7,
    GRAY = 0x8,
    LIGHT_BLUE = 0x9,
    LIGHT_GREEN = 0xa,
    LIGHT_CYAN = 0xb,
    LIGHT_RED = 0xc,
    LIGHT_MAGENTA = 0xd,
    YELLOW = 0xe,
    WHITE = 0xf,
} term_color_t;

void term_init();

int term_new_vt(term_color_t bg, term_color_t fg, int cursor_enabled);
int term_set_active(int id);
void term_flush_active();

int term_set_color(int id, term_color_t bg, term_color_t fg);

int term_clear(int id);
int term_flush(int id);

int term_set_cursor_enabled(int id, int enabled);
int term_set_cursor_position(int id, int x, int y);

int term_putch(int id, char c);
int term_printn(int id, uint32_t n, unsigned int base);
int term_dumpdword(int id, uint32_t n);
int term_print(int id, const char *str);
int term_printf(int id, const char *fmt, ...);

#endif
