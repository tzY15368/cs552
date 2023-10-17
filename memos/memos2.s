	.code32
	.globl _start

_start:
	jmp real_start
	.align 4
	.long 0x1BADB002 /* Multiboot magic number */
	.long 0x00000003 /* Align modules to 4KB, req. mem size */
	.long 0xE4524FFB /* Checksum */

stack_space:
	.space 0x1000

real_start:
	
	mov $stack_space, %esp
	push %ebx
	call c_handle_boot

	hlt
	jmp .-1
