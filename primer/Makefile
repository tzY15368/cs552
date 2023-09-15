obj-m += pmod.o

build_module: pmod.c
	make -C /usr/src/linux-`uname -r` SUBDIRS=$(PWD) modules
# make -j 8 -C ../lab1-lkm/linux-5.19 SUBDIRS=$(PWD) modules

u:
	rm u; gcc -o u puser.c

load: build_module
	insmod pmod.ko

unload:
	rmmod pmod

clean:
	make -C /usr/src/linux-`uname -r` SUBDIRS=$(PWD) clean
	rm -f u