	.code32
	.globl _start

_start:
	jmp real_start
	.align 4
	.long 0x1BADB002 /* Multiboot magic number */
	.long 0x00000003 /* Align modules to 4KB, req. mem size */
	.long 0xE4524FFB /* Checksum */

gdt_start:
	.quad 0x0
gdt_code:
    .word 0xFFFF
    .word 0x00
    .byte 0x00
    .byte 0b10011010
    .byte 0b11001111
    .byte 0x00
gdt_data:
    .word 0xFFFF
    .word 0x00
    .byte 0x00
    .byte 0b10010010
    .byte 0b11001111
    .byte 0x00
gdt_end:

gdt_ptr:
    .word gdt_end - gdt_start - 1
    .long gdt_start

CODE_SEG = gdt_code - gdt_start
DATA_SEG = gdt_data - gdt_start

real_start:
	cli
	lgdt gdt_ptr
	mov %cr0, %eax
	or $0x1, %eax
	mov %eax, %cr0
	jmp $CODE_SEG, $protect_start

protect_start:

	mov $DATA_SEG, %ax
	mov %ax, %ds
	mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    mov $stack_space, %esp

	call print_memory_size
	add $4, %esp

	hlt
	jmp .-1

stack_space:
    .space 0x1000
