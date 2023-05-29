nasm -f elf32 -g -o boot.o boot.asm
nasm -f elf32 -g -o vga.o vga.asm
nasm -f elf32 -g -o gdt.o gdt.asm
nasm -f elf32 -g -o pic.o pic.asm
nasm -f elf32 -g -o multitasking.o multitasking.asm
i686-elf-gcc -T linker.ld -o kernel.bin boot.o vga.o gdt.o pic.o multitasking.o -O2 -ffreestanding -nostdlib -lgcc

