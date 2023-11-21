# ; filename: isr_wrapper.s
.globl   isr_wrapper
.globl   timer_wrapper
.globl   setup_pit
.align   4
 
isr_wrapper:
    pushal
    cld    # ; C code following the sysV ABI requires DF to be clear on function entry
    call interrupt_handler
    popal
    iret

timer_wrapper:
    pushal
    cld    # ; C code following the sysV ABI requires DF to be clear on function entry
    call timer_handler
    popal
    iret

setup_pit:
    # 00110100
    movl $119318, %edx # 1193180 / 10 = 100ms
    movl $0x34, %eax # 00110100  =0x34
    out %al, $0x43

    movl %edx, %eax
    out %al, $0x40
    xchg %ah, %al
    out %al, $0x40