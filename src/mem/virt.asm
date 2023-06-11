section .text

extern term_dumpdword
extern term_flush

global enable_paging
enable_paging:
    push ebp
    mov ebp, esp

    push .done
    push 0
    call term_dumpdword
    push 0
    call term_flush

    ; mov eax, [ebp + 8] ; the page directory
    ; mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000 ; enable paging
    mov cr0, eax

.done:
    leave
    ret

global activate_context
activate_context:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8] ; page_dir_entry_t *
    mov cr3, eax

    leave
    ret

global invalidate_page
invalidate_page:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    invlpg [eax]

    leave
    ret
