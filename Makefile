# Compiler settings
CC = x86_64-elf-gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c
ASM = nasm
ASMFLAGS = -f elf32
LD = x86_64-elf-ld
LDFLAGS = -T src/kernel/linker.ld -m elf_i386

# Directories
SRC_DIR = src
BUILD_DIR = build

# Files
BOOT_SRC = $(SRC_DIR)/boot/boot.asm
KERNEL_ENTRY = $(SRC_DIR)/kernel/entry.asm
KERNEL_SRC = $(SRC_DIR)/kernel/kernel.c
BOOT_BIN = $(BUILD_DIR)/boot.bin
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
ENTRY_OBJ = $(BUILD_DIR)/entry.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/nox-os.img

# Build rules
all: $(OS_IMAGE)

$(BOOT_BIN): $(BOOT_SRC)
	$(ASM) -f bin $< -o $@

$(ENTRY_OBJ): $(KERNEL_ENTRY)
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) $< -o $@

$(KERNEL_BIN): $(ENTRY_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$(BOOT_BIN) of=$@ conv=notrunc
	# Ensure the kernel is properly aligned at sector 2
	dd if=$(KERNEL_BIN) of=$(OS_IMAGE) seek=1 conv=notrunc bs=512

run: $(OS_IMAGE)
	qemu-system-i386 -fda $(OS_IMAGE) -boot a -monitor stdio -d int -no-reboot

clean:
	rm -rf $(BUILD_DIR)/*
	mkdir -p $(BUILD_DIR)