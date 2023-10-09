memos1:
	as --32 -o memos1.o memos1.s
	ld -T memos1.ld memos1.o -o memos1
	dd bs=1 if=memos1 of=memos1.bin skip=4096 count=512
	