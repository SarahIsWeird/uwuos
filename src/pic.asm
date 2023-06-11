PIC1_CMD  equ 0x20
PIC1_DATA equ 0x21
PIC2_CMD  equ 0xa0
PIC2_DATA equ 0xa1

ICW1_ICW4      equ 0x01
ICW1_SINGLE    equ 0x02
ICW1_INTERVAL4 equ 0x04
ICW1_LEVEL     equ 0x08
ICW1_INIT      equ 0x10
ICW4_8086      equ 0x01

section .text
global init_pic
init_pic:
    mov al, ICW1_INIT | ICW1_ICW4
    out PIC1_CMD, al
    out PIC2_CMD, al

    mov al, 0x20 ; main pic IRQs right after exceptions
    out PIC1_DATA, al
    mov al, 0x28 ; sub pic IRQs right after those
    out PIC2_DATA, al

    mov al, 0x04 ; set IRQ for pics to communicate over bit mask for main pic (-> IRQ 2)
    out PIC1_DATA, al
    mov al, 0x02 ; same same, but as the actual number this time
    out PIC2_DATA, al

    mov al, ICW4_8086
    out PIC1_DATA, al
    out PIC2_DATA, al

    ; mask all interrupts, just as a test
    mov al, 0b1111_1110
    out PIC1_DATA, al
    mov al, 0xff
    out PIC2_DATA, al

    ret

global ack_irq
ack_irq:
    mov al, 0x20
    out PIC1_CMD, al
    ret

