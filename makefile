CC         = x86_64-elf-gcc
LD 	 	   = x86_64-elf-ld
AS  	   = nasm

ASMFILES  := $(shell find src -name '*.asm')
CFILES    := $(shell find src -name '*.c') 

OBJ = $(patsubst %.c, $(BUILD_DIRECTORY)/%.c.o, $(CFILES)) \
        $(patsubst %.asm, $(BUILD_DIRECTORY)/%.asm.o, $(ASMFILES))

TARGET = $(BUILD_DIRECTORY)/kernel.elf
ISO = bestOS.iso
MEMORY = 256

CHARDFLAGS := -nostdlib \
			-g 	\
			-O0                                                   \
			-fno-stack-protector			\
			-Wall							\
			-Wextra							\
			-Werror						\
			-ffreestanding					\
			-std=gnu99						\
			-mcmodel=kernel					\
			-Isrc/kernel					\
			-Isrc 							\
			-fno-pic						\
			-mno-red-zone					\
			-mno-sse						\
			-mno-sse2						\

LDHARDFLAGS := \
	-nostdlib                 \
	-static                   \
	-z max-page-size=0x1000  \
	-T src/link.ld

NASMFLAGS := -felf64

BUILD_DIRECTORY := build
DIRECTORY_GUARD = @mkdir -p $(@D)

.DEFAULT_GOAL = $(ISO)

$(ISO): $(TARGET)
	$(SHELL) meta/scripts/make-image.sh > /dev/null 2>&1

run: $(ISO)
	qemu-system-x86_64 -cdrom $< -enable-kvm -serial stdio -rtc base=localtime -m $(MEMORY) -no-shutdown -no-reboot


debug: $(ISO)
	qemu-system-x86_64 -cdrom $< -d int -serial stdio -rtc base=localtime -m $(MEMORY) -M smm=off -no-shutdown -no-reboot

gdb: $(ISO)
	qemu-system-x86_64 -cdrom $< -s -S -serial stdio -m $(MEMORY) -no-shutdown -no-reboot

$(BUILD_DIRECTORY)/%.c.o: %.c
	$(DIRECTORY_GUARD)
	$(CC) $(CHARDFLAGS) -c $< -o $@

$(BUILD_DIRECTORY)/%.asm.o: %.asm
	$(DIRECTORY_GUARD)
	nasm $(NASMFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(LD) $(LDHARDFLAGS) $(OBJ) -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIRECTORY) $(TARGET) $(ISO)
