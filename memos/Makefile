memos1:
	as --32 -o memos1.o memos1.s
	ld -T memos1.ld memos1.o -o memos1
	dd bs=1 if=memos1 of=memos1.bin skip=4096 count=512

memos2:
	as --32 memos2.s -o memos2.o
	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -c -o memos2c.o -I ./include memos2.c
	ld -m elf_i386 -T memos2.ld -o memos2 memos2.o memos2c.o
	sudo cp memos2 ./mdev/boot/memos2
mount:
	sudo mount t5.img ./mdev -text2 -o loop,offset=32256