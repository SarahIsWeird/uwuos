VGA_MEM     equ 0xb8000
WIDTH       equ 80
HEIGHT      equ 25

section .rodata
digits:
    db "0123456789abcdef"
test:
    db 0x00
    db 0x01
    db 0x02
    db 0x03
    dw 0x0405
    dw 0x0607
    dd 0x08090a0b

section .data
term_row:
    dd 0
term_col:
    dd 0
current_color:
    db 0x0f

global set_color
set_color:
    push ebp
    mov ebp, esp
    mov ax, word [ebp + 8]
    mov word [current_color], ax
    pop ebp
    ret

global clear
clear:
    push ebp
    mov ebp, esp
    push edi
    mov edi, VGA_MEM
    mov ecx, WIDTH * HEIGHT / 2
    movzx edx, word [current_color]
    shl edx, 8
    add edx, " "
    mov eax, edx ; set upper 16 bits of eax
    shl eax, 16
    add eax, edx ; set lower 16 bits of eax
rep stosd
    pop edi
    pop ebp
    ret

ensure_space_on_screen:
    push ebp
    mov ebp, esp
    mov eax, dword [term_row]
    cmp eax, 25
    jl .done

    push edi ; scroll the terminal, i.e., move all content up by one row
    push esi

    mov edi, VGA_MEM ; copy screen contents to row above
    mov esi, VGA_MEM + WIDTH * 2
    mov ecx, WIDTH * (HEIGHT - 1) / 2
rep movsd

    mov edi, VGA_MEM + WIDTH * (HEIGHT - 1) * 2 ; clear last row
    mov ecx, WIDTH
    movzx eax, word [current_color]
    shl eax, 8
    add eax, " "
rep stosw

    mov dword [term_col], 0
    mov dword [term_row], 24

    pop esi
    pop edi
.done:
    pop ebp
    ret

global putch
putch:
    push ebp
    mov ebp, esp
    push ebx
    push esi

    mov dl, byte [ebp + 8]

    cmp dl, 0x0a ; newline handling
    jne .not_nl
    inc dword [term_row]
    mov dword [term_col], 0
    call ensure_space_on_screen
    jmp .done
.not_nl:
    call ensure_space_on_screen

    mov eax, dword [term_row]
    lea ebx, [eax + eax * 4]
    shl ebx, 4 ; * 5 * 20 = * 80
    mov ecx, dword [term_col]
    mov dh, byte [current_color]
    lea esi, [ebx + ecx]
    mov word [VGA_MEM + 2 * esi], dx

    cmp ecx, 79
    je .end_of_row
    inc ecx
    mov dword [term_col], ecx
    jmp .done
.end_of_row:
    mov dword [term_col], 0
    inc eax
    mov dword [term_row], eax
.done:
    pop esi
    pop ebx
    pop ebp
    ret

global print
print:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    
    mov esi, dword [ebp + 8]
    mov ebx, dword [ebp + 12]
.loop:
    movsx eax, byte [esi]
    push eax
    call putch
    add esp, 4
    inc esi
    dec ebx
    test ebx, ebx
    jnz .loop

    pop esi
    pop ebx
    pop ebp
    ret

global putn
putn:
    push ebp
    mov ebp, esp
    push ebx
    push esi

    mov ebx, dword [ebp + 8]
    mov esi, dword [ebp + 12]

    xor edx, edx
    mov eax, ebx
    div esi
    mov esi, edx
    test eax, eax
    jz .no_recursion
    mov ecx, dword [ebp + 12]
    push ecx
    push eax
    call putn
    add esp, 8
.no_recursion:
    mov al, byte [digits + esi]
    push eax
    call putch
    add esp, 4

    pop esi
    pop ebx
    pop ebp
    ret

digit_count:
    push ebp
    mov ebp, esp
    push ebx

    mov ebx, dword [ebp + 12] ; base
    mov eax, dword [ebp + 8] ; value
    xor ecx, ecx
.loop:
    xor edx, edx
    div ebx
    inc ecx
    cmp eax, 0
    jg .loop

    mov eax, ecx
    pop ebx
    pop ebp
    ret

dumpdword:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    
    mov ebx, dword [ebp + 8] ; the value to print
    push 16
    push ebx
    call digit_count
    add esp, 8

    mov esi, 8 ; we print 8 (length of a dword in hex) - the digits (in hex) zeroes
    sub esi, eax

    push "0" ; print the leading zeros
.print_zeros:
    cmp esi, 1
    jl .done_printing_zeros
    call putch
    dec esi
    jmp .print_zeros
.done_printing_zeros:
    add esp, 4

    mov eax, dword [ebp + 8] ; print the actual number
    push 16
    push eax
    call putn
    add esp, 8

    pop esi
    pop ebx
    pop ebp
    ret

dumpline:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov esi, dword [ebp + 12] ; the byte count
    mov ebx, dword [ebp + 8] ; the address
    xor edi, edi

    push "["
    call putch
    add esp, 4

    push ebx
    call dumpdword
    add esp, 4

    push "]"
    call putch
    add esp, 4

    push " "
    call putch
    add esp, 4

.loop:
    lea eax, [ebx + edi] ; 4 bytes per dword
    mov edx, dword [eax]

    push edx
    call dumpdword
    add esp, 4

    push " "
    call putch
    add esp, 4

    add edi, 4
    cmp edi, esi
    jl .loop

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

global dump
dump:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov edi, dword [ebp + 16] ; dwords per line
    mov esi, dword [ebp + 12] ; bytes to print
    mov ebx, dword [ebp + 8] ; address

    lea eax, [edi * 4]
    mov edi, eax

.loop:
    push edi
    push ebx
    call dumpline
    add esp, 4

    push 0x0a
    call putch
    add esp, 4

    add ebx, edi
    sub esi, edi
    cmp esi, 0
    jg .loop

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret






























