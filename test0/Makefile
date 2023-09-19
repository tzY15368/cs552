obj-m += tmod.o

all:
	make -j 8 -C /lib/modules/`uname -r`/build M=$(shell pwd) modules EXTRA_CFLAGS="$(EXTRA_CFLAGS) -H"  2>&1 | tee out.txt