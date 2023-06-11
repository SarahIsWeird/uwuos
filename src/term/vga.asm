section .text

global vga_set_cursor_enabled
vga_set_cursor_enabled:
    push ebp
    mov ebp, esp

    mov ecx, [ebp + 8]

    mov edx, 0x3d4
    mov eax, 0xa
    out dx, al

    inc edx
    in al, dx
    
    test ecx, ecx
    jz .disable

    or al, 0b0100_0000 ; enable = 1, timing = 0
    and al, 0b1101_1111
    out dx, al
    jmp .done

.disable:
    and al, 0b1011_1111 ; enable = 0, timing = 1
    or al, 0b0010_0000
    out dx, al
    
.done:
    leave
    ret

global vga_set_cursor_pos
vga_set_cursor_pos:
    push ebp
    mov ebp, esp
    push ebx

    mov ecx, [ebp + 8] ; x
    mov eax, [ebp + 12] ; y

    xor edx, edx
    mov ebx, 80
    mul ebx
    add ecx, eax ; y * 80 + x

    mov dx, 0x3d4
    mov al, 0xf
    out dx, al

    inc dx
    mov al, cl
    out dx, al

    dec dx
    mov al, 0xe
    out dx, al

    inc dx
    mov al, ch
    out dx, al

    mov ebx, [ebp - 4]
    leave
    ret
