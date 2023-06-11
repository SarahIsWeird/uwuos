CC := i686-elf-gcc
AS := nasm
LD := i686-elf-gcc

CFLAGS := -c -std=c99 -ffreestanding -O2 -Wall -Wextra $(CFLAGS)
ASFLAGS := -felf32 -g $(ASFLAGS)
LDFLAGS := -T linker.ld -O2 -ffreestanding -nostdlib -lgcc

CSRC := $(shell find src -name '*.c')
ASMSRC := $(shell find src -name '*.asm')

COBJ := $(patsubst %.c,%.c.o,$(subst src/,build/,$(CSRC)))
ASMOBJ := $(patsubst %.asm,%.asm.o,$(subst src/,build/,$(ASMSRC)))

.PHONY: clean

all: program

$(COBJ): build/%.c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

$(ASMOBJ): build/%.asm.o: src/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

program: $(COBJ) $(ASMOBJ)
	$(LD) $(LDFLAGS) -o uwu.bin $(COBJ) $(ASMOBJ) -lgcc

clean:
	rm -r build/ uwu.bin 2> /dev/null || true
