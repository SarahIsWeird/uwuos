INTERNAL_PIT_FREQ equ 1_193_182
WANTED_FREQ       equ 100000

section .text

global pit_available
pit_available:
    mov eax, 1 ; PIT *should* be always available
    ret

global pit_timer_res
pit_timer_res:
    xor edx, edx
    mov ecx, INTERNAL_PIT_FREQ / WANTED_FREQ
    mov eax, INTERNAL_PIT_FREQ
    div ecx

    xor edx, edx
    mov ecx, eax
    mov eax, 1_000_000
    div ecx
    shr ecx, 1
    cmp edx, ecx
    jb .no_rounding
    inc eax
.no_rounding:

    ret

global pit_init
pit_init:
    mov ecx, INTERNAL_PIT_FREQ / WANTED_FREQ
    and cl, 0xfe ; clear the lowest bit, because the PIT does funky stuff with odd count values

    mov al, 0x34
    out 0x43, al
    mov al, cl
    out 0x40, al
    mov al, ch
    out 0x40, al

    xor eax, eax
    ret
