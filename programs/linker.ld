ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x10000000;

    .text : {
        *(.text)
    }

    .data : ALIGN(4K) {
        *(.data)
    }

    .rodata : ALIGN(4K) {
        *(.rodata)
    }

    .bss : ALIGN(4K) {
        *(.bss)
    }
}
