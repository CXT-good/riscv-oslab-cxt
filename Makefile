CC = riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
OBJCOPY = riscv64-unknown-elf-objcopy
CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -Iinclude

# 添加内存管理模块到源文件列表
SRCS = kernel/entry.S kernel/main.c kernel/uart.c kernel/console.c kernel/printf.c kernel/color_printf.c \
       kernel/mm/pmm.c kernel/mm/vmm.c

OBJS = $(SRCS:.S=.o)
OBJS := $(OBJS:.c=.o)
DEPS = $(OBJS:.o=.d)

kernel.elf: $(OBJS) kernel/kernel.ld
	$(CC) $(CFLAGS) -T kernel/kernel.ld -o $@ $(OBJS) -lgcc

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

qemu: kernel.elf
	qemu-system-riscv64 -machine virt -nographic -bios none -kernel kernel.elf

clean:
	rm -f *.elf $(OBJS) $(DEPS) kernel/mm/*.d

# Include dependencies
-include $(DEPS)

.PHONY: qemu clean