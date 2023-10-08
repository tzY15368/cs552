    .globl _start
    .code16
_start:
    movw $0x9000, %ax
	movw %ax, %ss
	xorw %sp, %sp


	movw $0x7C0, %dx # Load the MBR ar 0x7C00
	movw %dx, %ds    # BIOS loads the MBR at 0x7C00
    
	# movl entrybufferbase, %es:(%di) 

    movb $0, [mementrycnt]

    call probe_mem

    movw $0, %di
    movw $0x1234, %ax
	addl %eax, %es:(%di)


	call print_mem_all

    #jmp end

    leaw nlstr, %si
    movw nlstr_len, %cx
    call print

    call print_mem_range

    jmp end

probe_mem:
    movw $0x2000, %di
    xorl %ebx, %ebx
    xorw %bp , %bp
do_e820_loop:
	movl $0xe820, %eax
	movl $0x0534D4150, %edx
	movl $1, %es:20(%di)
	movl  $24, %ecx
	int $0x15
    jc printerr
	cmpl $0x0534D4150, %eax
	jne printerr
	testl %ebx, %ebx
	je e820f

    # print stuff
    push %cx

    # increment integer at mementrycnt
    movb [mementrycnt], %al
    addb $1, %al 
    movb %al, [mementrycnt]

    # print di
    push %eax
    # movl %di, %edx   # Base address 4 MSBs
	mov %di, %ax
	call print_4_bytes
    leaw nlstr, %si
    movw nlstr_len, %cx
    call print
    pop %eax

    pop %cx
    # increment di by 24
    add $24, %di
    jne do_e820_loop

e820f:
    mov %es:8(%di), %eax
    
	# Save %di
	push %di
	mov $0x6000, %di
	addl %eax, %es:(%di)
	# Restore %di
	pop %di
    ret

print_mem_all:
    pusha

    # Print welcome message
	leaw msg, %si
	movw msg_len, %cx
	call print
    # set eax to 0x0100
    movw memcnt, %di
    movl %es:(%di), %eax
    # Save %eax
	push %eax
	shr $8, %eax           # Print MSB first
	call print_byte        
	
	pop %eax
	call print_byte        # Print LSB

    leaw mbstr, %si
    movw mbstr_len, %cx
    call print

    leaw nlstr, %si
    movw nlstr_len, %cx
    call print

    movb [mementrycnt], %al
    addb $48, %al
    call printch

    popa
    ret

print_mem_range:
    pusha

    # load loop counter
    movb [mementrycnt], %cl
    movw $0x2000, %di

range_loop:
    # print range txt
    push %ecx

    #leaw addr_range, %si
    #movw addr_range_len, %cx
    #call print

    # print di
    push %eax
    mov %di, %ax
    call print_4_bytes
    pop %eax


    # print start address

    #movl %es:4(%di), %eax
	#call print_4_bytes
    #movl %es:(%di), %eax 
    #call print_4_bytes

    # Save %eax
    pusha
    mov $':, %al
    call printch
    popa

    # print end address
    
	addl %es:8(%di), %eax  
	push %eax 	    
    adcl %es:12(%di), %edx
	mov %edx, %eax
	call print_4_bytes
    pop %eax 
	call print_4_bytes

    # print status
    leaw status, %si
    movw status_len, %cx
    call print

    # print status num
    mov %es:16(%di), %al
	call print_byte

    # print nl
    leaw nlstr, %si
    movw nlstr_len, %cx
    call print
    # increment %di by 20
    add $24, %di
    pop %ecx
    loop range_loop

    popa
    ret

# Print the contents of a register
print_4_bytes:
	push %eax
	pusha
	mov $4, %cx
	
loop_shifts:
	shld $8, %eax, %eax
	call print_byte
	loop loop_shifts
	popa
	pop %eax
	ret

print_byte:	
    pushw %dx
	movb %al, %dl
	shrb $4, %al
	cmpb $10, %al
	jge 1f
	addb $0x30, %al
	jmp 2f
1:	addb $55, %al		
2:	movb $0x0E, %ah
	movw $0x07, %bx
	int $0x10

	movb %dl, %al
	andb $0x0f, %al
	cmpb $10, %al
	jge 1f
	addb $0x30, %al
	jmp 2f
1:	addb $55, %al		
2:	movb $0x0E, %ah
	movw $0x07, %bx
	int $0x10
	popw %dx
	ret


# Print a whole string
print:
	lodsb                	# Load a byte from DS:SI
	call printch
	loop print
	ret

# Print 1 ascii character
printch:
	movb $0x0E, %ah 	# Write character to the screen
	int $0x10
	ret

printerr:
    leaw errmsg, %si
    movw errmsg_len, %cx
    call print
    jmp end

memcnt: .word 0x0
mementrycnt: .word 0x1000
entrybufferbase: .word 0x2000

errmsg: .asciz "err"
errmsg_len: .word . - errmsg -1

msg: .asciz "MemOS: Welcome *** System Memory is: "
msg_len: .word . - msg -1

addr_range:.asciz "Address range ["
addr_range_len:.word . - addr_range -1

status:.asciz "] status: "
status_len:.word . - status -1

mbstr: .asciz "MB"
mbstr_len:.word . - mbstr -1

nlstr: .asciz "\n\r"
nlstr_len:.word . - nlstr -1

end:
	hlt

# This is going to be in our MBR for Bochs, so we need a valid signature
	.org 0x1FE

	.byte 0x55
	.byte 0xAA