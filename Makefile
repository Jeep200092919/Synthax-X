# Synthax X build system

AS       := nasm
CC       := x86_64-elf-gcc
LD       := x86_64-elf-ld

SRC_DIR  := src
INC_DIR  := include
BIN_DIR  := bin
ISO_DIR  := iso

KERNEL   := $(BIN_DIR)/synthax-x.bin
ISO      := $(BIN_DIR)/synthax-x.iso
LINKER   := linker.ld

ASFLAGS  := -f elf64
CFLAGS   := -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone \
            -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel \
            -Wall -Wextra -O2 -std=gnu11 -I$(INC_DIR)
LDFLAGS  := -n -nostdlib -T $(LINKER)

C_SRCS   := $(shell find $(SRC_DIR) -name '*.c')
ASM_SRCS := $(shell find $(SRC_DIR) -name '*.asm')
OBJS     := $(C_SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o) \
            $(ASM_SRCS:$(SRC_DIR)/%.asm=$(BIN_DIR)/%.o)

.PHONY: all iso clean run tools

all: $(KERNEL)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL): $(OBJS) $(LINKER)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

iso: $(ISO)

$(ISO): $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/synthax-x.bin
	grub-mkrescue -o $@ $(ISO_DIR) 2>/dev/null || \
	    xorriso -as mkisofs -R -b boot/grub/i386-pc/eltorito.img \
	        -no-emul-boot -boot-load-size 4 -boot-info-table \
	        -o $@ $(ISO_DIR)

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO)

tools:
	@bash scripts/check_tools.sh

clean:
	rm -rf $(BIN_DIR) $(ISO_DIR)/boot/synthax-x.bin
