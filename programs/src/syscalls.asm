section .text

global putch
putch:
    push ebp
    mov ebp, esp
    push ebx

    mov ecx, [ebp + 12]
    mov ebx, [ebp +  8]
    xor eax, eax
    int 0x69

    mov ebx, [ebp - 4]
    leave
    ret

global flush
flush:
    push ebp
    mov ebp, esp
    push ebx

    mov ebx, [ebp + 8]
    mov eax, 1
    int 0x69

    mov ebx, [ebp - 4]
    leave
    ret
