
CC = riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
OBJCOPY = riscv64-unknown-elf-objcopy
CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -Iinclude

# 显式列出所有对象文件
OBJS = \
    kernel/entry.o \
    kernel/test_main.o \
    kernel/uart.o \
    kernel/console.o \
    kernel/printf.o \
    kernel/color_printf.o \
    kernel/mm/pmm.o \
    kernel/mm/vmm.o \
    kernel/mm/buddy.o \
	kernel/sbi.o \
    kernel/trap.o \
    kernel/trap_c.o  # 显式添加 trap_c.o

DEPS = $(OBJS:.o=.d)

kernel.elf: $(OBJS) kernel/kernel.ld
	$(CC) $(CFLAGS) -T kernel/kernel.ld -o $@ $(OBJS) -lgcc

# 显式规则
kernel/trap.o: kernel/trap.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/trap_c.o: kernel/trap.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 通用规则
%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

qemu: kernel.elf
	qemu-system-riscv64 -machine virt -nographic -bios none -kernel kernel.elf

clean:
	rm -f *.elf $(OBJS) $(DEPS) kernel/mm/*.d kernel/*.d

-include $(DEPS)

.PHONY: qemu clean
