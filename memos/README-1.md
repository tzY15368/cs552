# Memos-1

## build

Run `make -B memos1 && qemu-system-i386 -hda memos1.bin -m 10M`

This will build memos1 as binary, move it into the MBR section of memos1.bin disk image and use qemu to run that OS with 10MB RAM.