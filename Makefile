ifeq ($(origin CC),default)
CC := x86_64-elf-gcc
endif

BUILD_DIR := build
KERNEL := $(BUILD_DIR)/kernel.elf
ISO_ROOT := $(BUILD_DIR)/iso
ISO := $(BUILD_DIR)/leptoos.iso
GRUB_CFG := grub/grub.cfg
GRUB_MKRESCUE := grub-mkrescue
QEMU ?= qemu-system-x86_64
QEMUFLAGS ?= -cdrom $(ISO)
ARCH_FLAGS ?= -m64

C_SOURCES := $(wildcard *.c)
ASM_SOURCES := $(wildcard *.S)
C_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS := $(patsubst %.S,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
KERNEL_OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)

CFLAGS := $(ARCH_FLAGS) -std=gnu11 -O2 -Wall -Wextra \
	-ffreestanding -fno-builtin -fno-stack-protector \
	-fno-pic -fno-pie -mno-red-zone -MMD -MP
ASFLAGS := $(ARCH_FLAGS)
LDFLAGS := $(ARCH_FLAGS) -nostdlib -no-pie -Wl,-T,linker.ld -Wl,-e,start

.PHONY: all iso run clean

all: $(KERNEL)

iso: $(ISO)

run: $(ISO)
	$(QEMU) $(QEMUFLAGS)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(KERNEL_OBJECTS) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(KERNEL_OBJECTS)

$(ISO): $(KERNEL) $(GRUB_CFG) Makefile
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL) $(ISO_ROOT)/boot/kernel.elf
	cp $(GRUB_CFG) $(ISO_ROOT)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $@ $(ISO_ROOT)

clean:
	rm -rf $(BUILD_DIR)

-include $(C_OBJECTS:.o=.d)
