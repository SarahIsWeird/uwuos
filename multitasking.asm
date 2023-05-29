section .bss
stack1:
    resb 4096
stack2:
    resb 4096

section .data
states:
    dd 0
    dd 0
current_task:
    dd -1
tasks:
    dd 0

section .text
; vga.asm
extern putch

task1:
    push "A"
.loop:
    call putch
    jmp .loop

task2:
    push "B"
.loop:
    call putch
    jmp .loop

global init_multitasking
init_multitasking:
    push ebp
    mov ebp, esp

    push task1
    push stack1
    call init_task
    add esp, 8

    push task2
    push stack2
    call init_task
    add esp, 8

    pop ebp
    ret

global init_task
init_task:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8] ; task stack
    mov ecx, [ebp + 12] ; task entry

    mov dword [eax     ], 0x202 ; eflags
    mov dword [eax +  4], 0x08 ; cs
    mov dword [eax +  8], ecx ; eip
    mov dword [eax + 12], 0 ; error code
    mov dword [eax + 16], 0 ; interrupt number
    mov dword [eax + 20], 0 ; edi
    mov dword [eax + 24], 0 ; esi
    mov dword [eax + 28], 0 ; ebp
    mov dword [eax + 32], 0 ; esp
    mov dword [eax + 36], 0 ; ebx
    mov dword [eax + 40], 0 ; edx
    mov dword [eax + 44], 0 ; ecx
    mov dword [eax + 48], 0 ; eax

    inc dword [tasks]

    pop ebp
    ret

global schedule
schedule:
    mov eax, dword [current_task]
    cmp eax, 0
    jl .first_scheduling
    mov dword [states + eax], esp
.first_scheduling:

    inc eax
    xor edx, edx
    div dword [tasks]
    mov dword [current_task], edx
    
    mov esp, dword [states + edx]

    popad
    add esp, 8
    iret





















