CC = gcc
AS := as --32
CFLAGS = -m32 -fno-builtin -fno-stack-protector -fno-strict-aliasing -fno-delete-null-pointer-checks -nostdinc -I. -g -Wall -std=c99
CPPFLAGS = -Wa,--32 -MMD
OBJS = boot.o init.o gdtflush.o ctxswitch.o int_handler.o isr_wrapper.o
PROGS = memos-2
MNT_POINT=/mnt/

all: $(PROGS)

memos-2: $(OBJS)
	$(LD) -m elf_i386 -T memos.ld -o $@ $^

%: %.c

install: $(PROGS)
	cp $(PROGS) $(MNT_POINT)/boot
	sync

clean:
	-rm *.o *.d $(PROGS) *~

boot:
	qemu-system-i386 -no-reboot -no-shutdown -kernel memos-2

dbg:
	qemu-system-i386 -S -s -no-reboot -no-shutdown -kernel memos-2

-include *.d
