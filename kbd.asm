KBD_DATA equ 0x64
KBD_CMD  equ 0x60

KBD_BUFFER_SIZE equ 64

section .bss
kbd_buf:
    resb KBD_BUFFER_SIZE

section .data
kbd_avail:
    dd 0
kbd_read:
    dd kbd_buf
kbd_write:
    dd kbd_buf

section .text
global init_kbd
init_kbd:
    ret
