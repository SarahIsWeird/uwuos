CC := i686-elf-gcc
AS := nasm
LD := i686-elf-gcc

CFLAGS := -c -std=c99 -ffreestanding -O2 -Wall -Wextra $(CFLAGS)
ASFLAGS := -felf32 -g $(ASFLAGS)
LDFLAGS := -T linker.ld -O2 -ffreestanding -nostdlib -lgcc
QEMU_FLAGS := $(QEMU_FLAGS) -initrd programs/uwu.bin,programs/uwu.bin -m 4G

CSRC := $(shell find src -name '*.c')
ASMSRC := $(shell find src -name '*.asm')

COBJ := $(patsubst %.c,%.c.o,$(subst src/,build/,$(CSRC)))
ASMOBJ := $(patsubst %.asm,%.asm.o,$(subst src/,build/,$(ASMSRC)))

.PHONY: clean

all: kernel

$(COBJ): build/%.c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

$(ASMOBJ): build/%.asm.o: src/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

kernel: $(COBJ) $(ASMOBJ)
	$(LD) $(LDFLAGS) -o kernel.bin $(COBJ) $(ASMOBJ) -lgcc

iso: kernel grub/grub.cfg
	@mkdir -p build/iso/boot/grub

	cp kernel.bin build/iso/boot/kernel.bin
	cp grub/grub.cfg build/iso/boot/grub/grub.cfg

	grub-mkrescue -o uwuos.iso build/iso

run: kernel
	qemu-system-i386 -kernel kernel.bin $(QEMU_FLAGS)

runiso: iso
	qemu-system-i386 -cdrom uwuos.iso $(QEMU_FLAGS)

debug: kernel
	qemu-system-i386 -kernel kernel.bin -s -S $(QEMU_FLAGS)

clean:
	rm -r build/ uwu.bin uwuos.iso 2> /dev/null || true
