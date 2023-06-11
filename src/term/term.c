#include "term.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "../mem/virt.h"
#include "../util.h"
#include "vga.h"

#define GET_VT(id) if (id >= vt_count) return 1; \
    volatile vt_t *vt = vts[id]

#define TERM_COLOR (((uint16_t) vt->settings.text_color.combined) << 8)

#define CURRENT_IDX (vt->settings.y * TERM_WIDTH + vt->settings.x)

// here's a lesson in trickery
// we just disable the flag so term_set_cursor_position doesn't frfr update the
// vga cursor position if it was enabled before, because it's hella slow
#define DISABLE_CURSOR_UPDATES() volatile int cursor = vt->settings.cursor; \
    vt->settings.cursor = 0

#define ENABLE_CURSOR_UPDATES() vt->settings.cursor = cursor; \
    if (cursor) vga_set_cursor_pos(vt->settings.x, vt->settings.y)

typedef union text_color_t {
    uint8_t combined;
    struct {
        uint8_t fg : 4;
        uint8_t bg : 4;
    } __attribute__ ((__packed__));
} text_color_t;

typedef struct vt_t {
    int id;
    struct {
        text_color_t text_color;
        uint8_t x;
        uint8_t y;
        uint8_t cursor : 1;
        uint8_t reserved : 7;
    } __attribute__ ((__packed__)) settings;
    uint16_t buffer[TERM_WIDTH * TERM_HEIGHT];
} vt_t;

static char *digits = "0123456789abcdef";

volatile uint16_t *text_buffer = (uint16_t *) 0xb8000;

static volatile vt_t **vts;
static volatile int vt_count = 0;
static volatile int active_vt;

void scroll(volatile vt_t *vt) {
    volatile uint16_t *dst = vt->buffer;
    volatile uint16_t *src = vt->buffer + TERM_WIDTH;

    for (int i = 0; i < TERM_WIDTH * (TERM_HEIGHT - 1); i++) {
        *(dst++) = *(src++);
    }

    for (int i = TERM_WIDTH * (TERM_HEIGHT - 1); i < TERM_WIDTH * TERM_HEIGHT; i++) {
        *(dst++) = TERM_COLOR | ' ';
    }
}

void advance_line(volatile vt_t *vt) {
    vt->settings.x = 0;
    vt->settings.y++;

    if (vt->settings.y < TERM_HEIGHT) return;

    vt->settings.y--;

    scroll(vt);
}

void advance_pos(volatile vt_t *vt) {
    vt->settings.x++;

    if (vt->settings.x < TERM_WIDTH) return;

    advance_line(vt);
}

void term_init() {
    vts = (volatile vt_t **) virt_kernel_alloc();

    term_new_vt(LIGHT_MAGENTA, WHITE, 0);
    term_set_active(0);
}

int term_new_vt(term_color_t bg, term_color_t fg, int cursor_enabled) {
    if (vt_count >= MAX_TERMS) return -1;
    
    int id = vt_count++;
    volatile vt_t *vt = (vt_t *) virt_kernel_alloc();
    vts[id] = vt;

    vt->id = id;

    term_set_color(id, bg, fg);
    term_set_cursor_enabled(id, cursor_enabled);
    term_clear(id);

    return id;
}

int term_set_active(int id) {
    GET_VT(id);

    active_vt = id;
    term_flush_active();

    vga_set_cursor_enabled(vt->settings.cursor);
    if (vt->settings.cursor) vga_set_cursor_pos(vt->settings.x, vt->settings.y);

    return 0;
}

int term_set_color(int id, term_color_t bg, term_color_t fg) {
    GET_VT(id);

    vt->settings.text_color.bg = bg;
    vt->settings.text_color.fg = fg;

    return 0;
}

int term_clear(int id) {
    GET_VT(id);

    uint16_t clear_val = TERM_COLOR | ' ';
    for (int i = 0; i < TERM_WIDTH * TERM_HEIGHT; i++) {
        vt->buffer[i] = clear_val;
    }

    term_set_cursor_position(id, 0, 0);

    return 0;
}

int term_flush(int id) {
    if (id >= vt_count) return 1;
    if (id != active_vt) return 0;

    term_flush_active();

    return 0;
}

void term_flush_active() {
    volatile uint16_t *dst = text_buffer;
    volatile uint16_t *src = vts[active_vt]->buffer;

    for (int i = 0; i < TERM_WIDTH * TERM_HEIGHT; i++) {
        *(dst++) = *(src++);
    }
}

int term_set_cursor_enabled(int id, int enabled) {
    GET_VT(id);

    vt->settings.cursor = enabled;

    if (id != active_vt) return 0;
    vga_set_cursor_enabled(enabled);

    if (!enabled) return 0;
    vga_set_cursor_pos(vt->settings.x, vt->settings.y);

    return 0;
}

int term_set_cursor_position(int id, int x, int y) {
    if (x >= TERM_WIDTH) return 1;
    if (y >= TERM_HEIGHT) return 1;

    GET_VT(id);

    vt->settings.x = x;
    vt->settings.y = y;

    if ((id != active_vt) || !vt->settings.cursor) return 0;

    vga_set_cursor_pos(x, y);

    return 0;
}

int term_putch(int id, char c) {
    GET_VT(id);

    switch (c) {
        case '\n':
            advance_line(vt);
            if (id == active_vt) term_flush_active();
            break;
        default:
            vt->buffer[CURRENT_IDX] = TERM_COLOR | c;
            advance_pos(vt);
            break;
    }

    term_set_cursor_position(id, vt->settings.x, vt->settings.y);

    return 0;
}

int term_printn(int id, uint32_t n, unsigned int base) {
    GET_VT(id);
    DISABLE_CURSOR_UPDATES();

    int digit = n % base;
    int rest = n / base;

    if (rest > 0) term_printn(id, rest, base); // we print the other digits before this one, because we take the last with %
    term_putch(id, digits[digit]);

    ENABLE_CURSOR_UPDATES();
    return 0;
}

int term_dumpdword(int id, uint32_t n) {
    GET_VT(id);
    DISABLE_CURSOR_UPDATES();

    // how many digits are there
    int n1 = n;
    int i = 0;
    do {
        i++;
        n1 /= 16;
    } while (n1);

    i = 8 - i; // figure out the missing digits to make it 8

    // print them
    while (i--) {
        term_putch(id, '0');
    }

    term_printn(id, n, 16);

    ENABLE_CURSOR_UPDATES();
    return 0;
}

int term_print(int id, const char *str) {
    GET_VT(id);
    DISABLE_CURSOR_UPDATES();

    while (*str) {
        term_putch(id, *(str++));
    }

    ENABLE_CURSOR_UPDATES();
    return 0;
}

int term_printf(int id, const char *fmt, ...) {
    GET_VT(id);
    DISABLE_CURSOR_UPDATES();

    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        char c = *(fmt++);

        // end of string, we're done
        if (!(*fmt)) {
            term_putch(id, c);
            break;
        }

        // not a format
        if (c != '%') {
            term_putch(id, c);
            continue;
        }

        c = *(fmt++);

        switch (c) {
            case 'c':
                term_putch(id, (char) va_arg(args, int));
                continue;
            case 'd':
            case 'i':
                int sn = va_arg(args, int);
                if (sn < 0) {
                    term_putch(id, '-');
                    sn = -sn;
                }

                term_printn(id, (uint32_t) sn, 10);
                continue;
            case 'n':
                continue;
            case 'o':
                int so = va_arg(args, int);
                if (so < 0) {
                    term_putch(id, '-');
                    so = -so;
                }

                term_printn(id, (uint32_t) so, 8);
                continue;
            case 'p':
                term_dumpdword(id, va_arg(args, uint32_t));
                continue;
            case 's':
                term_print(id, va_arg(args, const char *));
                continue;
            case 'u':
                term_printn(id, va_arg(args, unsigned int), 10);
                continue;
            case 'x':
            case 'X': // idc get fucked
                term_printn(id, va_arg(args, uint32_t), 16);
                continue;
            case '%':
                term_putch(id, '%');
                continue;
            default:
                break;
        }
    }

    va_end(args);

    ENABLE_CURSOR_UPDATES();
    return 0;
}
