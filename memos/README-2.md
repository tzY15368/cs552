# MEMOS2

U79759599 Tingzhou Yuan
cylinders = 10 + 99 = 109
heads = 16
sectorspertrack = 63

sectors=109 * 16 * 63 = 109872

## Build

Follow [BOCHS HOW TO](https://www.cs.bu.edu/fac/richwest/cs552_fall_2023/assignments/memos/disk-image-HOWTO) to build the disk image and install grub

`make -B memos2`

## Run

** Everything was tested and run in WSL Ubuntu 22.04, qemu-system-i386 **

`qemu-system-i386 t5.img -m 5M`