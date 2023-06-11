PRESENT               equ 0b1000_0000
DPL_KERNEL            equ 0b0000_0000
DPL_USER              equ 0b0110_0000

GDT_ACC_TYPE          equ 0b0001_0000
GDT_ACC_EXECUTABLE    equ 0b0000_1000
GDT_ACC_DIRECTION     equ 0b0000_0100
GDT_ACC_CODE_READABLE equ 0b0000_0010
GDT_ACC_DATA_WRITABLE equ 0b0000_0010

GDT_FLAGS_GRAN_4K   equ 0b1000 << 4
GDT_FLAGS_32_BIT    equ 0b0100 << 4
GDT_FLAGS_LONG_MODE equ 0b0010 << 4

IDT_TASK             equ 0x5
IDT_32_BIT_INTERRUPT equ 0xe
IDT_32_BIT_TRAP      equ 0xf

PIC_MAIN      equ 0x20
PIC_MAIN_CMD  equ PIC_MAIN + 0
PIC_MAIN_DATA equ PIC_MAIN + 1
PIC_SUB       equ 0xa0
PIC_SUB_CMD   equ PIC_SUB + 0
PIC_SUB_DATA  equ PIC_SUB + 1

%macro make_idt_entry 3
idt_entry_%1:
    dw 0
    dw 0x08
    db 0
    db PRESENT | %3 | %2
    dw 0
%endmacro

%macro idt_interrupt 1
    make_idt_entry %1, IDT_32_BIT_INTERRUPT, DPL_KERNEL
%endmacro

%macro idt_syscall 1
    make_idt_entry %1, IDT_32_BIT_INTERRUPT, DPL_USER
%endmacro

; %macro idt_interrupt_trap 1
;     make_idt_entry %1, IDT_32_BIT_INTERRUPT
; %endmacro

%macro idt_reserved 1
idt_entry_%1:
    dd 0
    dd 0
%endmacro

%macro int_handler 1
int_handler_%1:
    cli
    push 0
    push %1
    pushad

    mov eax, 0x10
    mov ds, ax
    mov es, ax

    push esp
    jmp interrupt_handler
%endmacro

%macro int_handler_errorcode 1
int_handler_%1:
    cli
    push %1
    pushad

    mov eax, 0x10
    mov ds, ax
    mov es, ax

    push esp
    jmp interrupt_handler
%endmacro

section .data
global gdt
gdt:

null_segment:
    dd 0
    dd 0

kernel_code_segment:
    dw 0xffff
    dw 0
    db 0
    db PRESENT | DPL_KERNEL | GDT_ACC_TYPE | GDT_ACC_EXECUTABLE | GDT_ACC_CODE_READABLE
    db GDT_FLAGS_GRAN_4K | GDT_FLAGS_32_BIT | 0xf
    db 0

kernel_data_segment:
    dw 0xffff
    dw 0
    db 0
    db PRESENT | DPL_KERNEL | GDT_ACC_TYPE | GDT_ACC_CODE_READABLE
    db GDT_FLAGS_GRAN_4K | GDT_FLAGS_32_BIT | 0xf
    db 0

user_code_segment:
    dw 0xffff
    dw 0
    db 0
    db PRESENT | DPL_USER | GDT_ACC_TYPE | GDT_ACC_EXECUTABLE | GDT_ACC_CODE_READABLE
    ; db 0xfa
    db GDT_FLAGS_GRAN_4K | GDT_FLAGS_32_BIT | 0xf
    db 0

user_data_segment:
    dw 0xffff
    dw 0
    db 0
    db PRESENT | DPL_USER | GDT_ACC_TYPE | GDT_ACC_CODE_READABLE
    ; db 0xf2
    db GDT_FLAGS_GRAN_4K | GDT_FLAGS_32_BIT | 0xf
    db 0

tss_segment:
    dw 32 * 4
    dw 0
    db 0
    db PRESENT | 0x9 | DPL_USER
    db 0
    db 0

GDT_SIZE equ $ - gdt

gdtr:
gdtr_limit:
    dw 0
gdtr_base:
    dd 0

global idt
idt:
    idt_interrupt 0 ; #DE divide by zero
    idt_interrupt 1 ; #DB debug
    idt_interrupt 2 ; #NMI non-maskable interrupt
    idt_interrupt 3 ; #BP breakpoint
    idt_interrupt 4 ; #OF overflow
    idt_interrupt 5 ; #BR bound range
    idt_interrupt 6 ; #UD invalid opcode
    idt_interrupt 7 ; #NM device not available
    idt_interrupt 8 ; #DF double fault
    idt_interrupt 9 ; coprocessor segment overrun
    idt_interrupt 10 ; #TS invalid TSS
    idt_interrupt 11 ; #NP segment not present
    idt_interrupt 12 ; #SS stack fault
    idt_interrupt 13 ; #GP general protection fault
    idt_interrupt 14 ; #PF page fault
    idt_reserved 15
    idt_interrupt 16 ; #MF x87 floating point
    idt_interrupt 17 ; #AC alignment check fault
    idt_interrupt 18 ; #MC machine check
    idt_interrupt 19 ; #XF SIMD floating point fault
    idt_reserved 20
    idt_reserved 21
    idt_reserved 22
    idt_reserved 23
    idt_reserved 24
    idt_reserved 25
    idt_reserved 26
    idt_reserved 27
    idt_reserved 28
    idt_reserved 29
    idt_interrupt 30 ; #SX security-sensitive event in host
    idt_reserved 31
%assign i 32
%rep 105 - 32
    idt_interrupt i
%assign i i+1
%endrep
    idt_syscall 105
%assign i 106
%rep 256 - 106
    idt_interrupt i
%assign i i+1
%endrep
IDT_SIZE equ $ - idt

idtr:
idtr_limit:
    dw 0
idtr_base:
    dd 0

tss:
    dd 0
    dd 0
    dd 0x10
    times 32 - 3 dd 0

int_text:
    db "Interrupt! :O", 0x0a
.len: equ $ - int_text

section .rodata
hex_prefix:
    db "0x", 0

section .text

; term/term.c
extern term_set_active
extern term_flush_active
extern term_set_color
extern term_printn
extern term_putch
extern term_print

; pic.asm
extern ack_irq

; multitasking.asm
extern schedule
extern get_current_page_dir

; timer/timer.c
extern advance_timers

; syscalls/syscall.c
extern syscall

global load_gdt
load_gdt:
    mov eax, tss
    mov byte [tss_segment + 2], al
    shr eax, 8
    mov byte [tss_segment + 3], al
    shr eax, 8
    mov byte [tss_segment + 4], al
    shr eax, 8
    mov byte [tss_segment + 7], al

    mov word [gdtr_limit], GDT_SIZE - 1
    mov dword [gdtr_base], gdt
    lgdt [gdtr]

    jmp 0x08:.reload_cs
.reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ax, 0x28
    ltr ax
    
    ret

int_handler 0
int_handler 1
int_handler 2
int_handler 3
int_handler 4
int_handler 5
int_handler 6
int_handler 7
int_handler_errorcode 8
int_handler 9
int_handler_errorcode 10
int_handler_errorcode 11
int_handler_errorcode 12
int_handler_errorcode 13
int_handler_errorcode 14

int_handler 16
int_handler_errorcode 17
int_handler 18
int_handler 19

int_handler_errorcode 30

%assign i 32
%rep 256 - 32
    int_handler i
%assign i i+1
%endrep

global update_tss
update_tss:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]
    mov dword [tss + 4], eax

    leave
    ret

extern handle_interrupt

interrupt_handler:
    call handle_interrupt
    mov esp, eax

    mov eax, 0x23
    mov ds, ax
    mov es, ax

    popad
    add esp, 8
    sti
    iret

; interrupt_handler:
;     mov ebx, [esp + 36]
;     mov esi, [esp + 40]
;     cmp ebx, 0x20
;     jl exception_handler

;     cmp ebx, 0x20 ; timer
;     jne .no_scheduling

;     call advance_timers

;     pop esi
;     push esi
;     call schedule
; .restore_state:
;     mov esp, eax
;     add eax, 60 ; sizeof(cpu_struct_t)
;     mov dword [tss + 4], eax

;     cmp esp, esi
;     je .no_pagedir_reload

;     call get_current_page_dir
;     mov cr3, eax

; .no_pagedir_reload:

;     call ack_irq
;     jmp .ret
; .no_scheduling:
;     cmp ebx, 0x69
;     je .syscall

;     mov ebx, [esp + 32]
;     cmp ebx, 0x20
;     jl .done
;     cmp ebx, 0x2f
;     jg .done
;     call ack_irq
; .done:
;     cmp ebx, 0x20
;     jge .ret
; .hang:
;     hlt
;     jmp .hang
; .ret:
;     mov eax, 0x23
;     mov ds, ax
;     mov es, ax

;     popad
;     add esp, 8
;     sti
;     iret
; .syscall:
;     call syscall
;     jmp .restore_state

exception_handler:
    push 0
    call term_set_active
    add esp, 4

    push 0x0
    push 0xe
    push 0
    call term_set_color
    add esp, 12

    push hex_prefix
    push 0
    call term_print
    add esp, 4

    push 16
    push ebx
    push 0
    call term_printn
    add esp, 12

    push " "
    push 0
    call term_putch
    add esp, 8

    push 10
    push esi
    push 0
    call term_printn
    add esp, 12

    call term_flush_active

    ; push "0"
    ; call putch
    ; mov byte [esp], "x"
    ; call putch
    ; add esp, 4

    ; push 16
    ; push ebx
    ; call putn
    ; add esp, 8

    ; push " "
    ; call putch
    ; add esp, 8

    ; push 10
    ; push esi
    ; call putn
    ; add esp, 8

    cli
.hang:
    hlt
    jmp .hang

%macro set_handler_address 2
%assign i %1
%rep %2
    mov eax, int_handler_%[i]
    mov edx, idt_entry_%[i]
    mov word [edx], ax
    shr eax, 16
    mov word [edx + 6], ax
%assign i i+1
%endrep
%endmacro

global int_handler_32

global load_idt
load_idt:
    set_handler_address 0, 15
    set_handler_address 16, 4
    set_handler_address 30, 1
    set_handler_address 32, 256 - 32

    mov word [idtr_limit], IDT_SIZE - 1
    mov dword [idtr_base], idt
    lidt [idtr]
    ret
