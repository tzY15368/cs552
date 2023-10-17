memos1:
	as --32 -o memos1.o memos1.s
	ld -T memos1.ld memos1.o -o memos1
	dd bs=1 if=memos1 of=memos1.bin skip=4096 count=512
	
memos2: memos2.s memos2.ld memos2.c
	gcc -c memos2.c -fno-builtin -nostdinc -o memos2c.o
	as --32 -o memos2.o memos2.s
	ld -T memos2.ld memos2.o memos2c.o -o memos2 

clean:
	rm -f *.o memos1 memos2
