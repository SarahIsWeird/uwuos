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
    db "UwU OS by Sahara <3", 0x0a
.len equ $ - uwuos_sig
hello_world:
    db "Hello, world! uwu How are you today? <3", 0x0a
    db "you're such an amazing person omg hahaha don't kill yourself you're so sexy ahahaha"
.len equ $ - hello_world

section .bss
align 4
stack_bottom:
    resb 16384
stack_top:

section .data
test:
    dd 0x0eadbeef
    dd 0x10101010
    dd 0x21234567
    dd 0x39abcdef
    dd 0x4eadbeef
    dd 0x50101010
    dd 0x61234567
    dd 0x79abcdef
    dd 0x8eadbeef
    dd 0x90101010
    dd 0xa1234567
    dd 0xb9abcdef
    dd 0xceadbeef
    dd 0xd0101010
    dd 0xe1234567
    dd 0xf9abcdef
    dd 0x11111111
test2:
    dd 0x89abcdef
    db 0x67
    db 0x45
    db 0x23
    db 0x01

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

; pic.asm
extern init_pic

; multitasking.asm
extern init_multitasking

global _start
_start:
    mov esp, stack_top

    cli

    call load_gdt
    call load_idt
    call init_pic

    push 0xdf
    call set_color
    add esp, 4

    call clear

    push uwuos_sig.len
    push uwuos_sig
    call print
    add esp, 8

    push 16
    push idt
    call putn
    add esp, 8

    push 0x0a
    call putch
    add esp, 4

    push 16
    push int_handler_32
    call putn
    add esp, 8

    push 0x0a
    call putch
    add esp, 4

    call init_multitasking

    sti

    jmp $

    push "a"
    call putch
    add esp, 4

    cli
.hang:
    hlt
    jmp .hang
.end:

