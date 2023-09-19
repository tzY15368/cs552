obj-m += sol.o

build_module: sol.c
	make -C /usr/src/linux-`uname -r` SUBDIRS=$(PWD) modules

load: build_module
	insmod sol.ko

unload:
	rmmod sol.ko


