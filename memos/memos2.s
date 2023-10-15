.globl _start

_start:
        jmp real_start

        /* Multiboot header -- Safe to place this header in 1st page of memory for GRUB */
        .align 4
        .long 0x1BADB002 /* Multiboot magic number */
        .long 0x00000003 /* Align modules to 4KB, req. mem size */
                         /* See 'info multiboot' for further info */
        .long 0xE4524FFB /* Checksum */

real_start:
    sub $16384, %esp
    push %eax
        # This is where the rest of your program goes
    push %ebx
    call cmain
    hlt