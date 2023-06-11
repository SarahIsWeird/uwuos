MB_ALIGN    equ 1 << 0
MB_MEMINFO  equ 1 << 0
MB_FLAGS    equ MB_ALIGN | MB_MEMINFO
MB_MAGIC    equ 0x1badb002
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

section .multiboot
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

section .rodata
uwuos_sig:
    db "UwU OS by Sahara <3", 0x0a, 0
.len equ $ - uwuos_sig

section .bss
align 4
stack_bottom:
    resb 16384
stack_top:

section .text
; gdt.asm
extern load_gdt
extern load_idt
extern idt
extern int_handler_32

; vga.asm
extern clear
extern putch
extern print
extern set_color
extern putn
extern dump
extern disable_cursor

; pic.asm
extern init_pic

; multitasking.asm
extern init_multitasking

; set for us by the linker
extern kernel_start
extern kernel_end

; mem/phys.c
extern phys_init

; timer/timer.c
extern init_timer

; term/term.c
extern term_init
extern term_print

; mem/virt.c
extern virt_init

global _start
_start:
    cli ; Remember kids, don't be silly, wrap your willy!

    mov esp, stack_top
    push ebx

    call load_gdt
    call load_idt
    call init_pic

    pop ebx
    push kernel_end
    push kernel_start
    push ebx
    call phys_init
    add esp, 12

    call virt_init

    call term_init

    push uwuos_sig
    push 0
    call term_print
    add esp, 8

    call init_timer

    push ebx
    call init_multitasking
    add esp, 4

    sti ; We do want kids at some point, though

    jmp $

    cli
.hang:
    hlt
    jmp .hang
.end:

