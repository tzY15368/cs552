# actual magic trick
# https://github.com/mit-pdos/xv6-public/blob/master/swtch.S
# Context switch
#
#   void swtch(uint32_t **old_stack, unit32_t *new_stack);
# 

By passing a pointer to a pointer (context_t* *ctx_old), 
the function can update the value of the pointer to the old context,
 effectively changing where it points. This allows the function 
 to switch between two contexts and update the calling code with 
 the new context.


# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

.globl ctx_switch
ctx_switch:
  movl 4(%esp), %eax
  movl 8(%esp), %edx

  # Save old callee-saved registers
  pushl %ebp
  pushl %ebx
 # pushl %eax
 # pushl %ecx
 # pushl %edx
  pushl %esi
  pushl %edi
#  pushf                #Push the eflags register to the stack
  pushw %gs
  pushw %fs
  pushw %es
  pushw %ds 

  # Switch stacks
  movl %esp, (%eax)
  movl %edx, %esp

  # Load new callee-saved registers
  popw %ds
  popw %es
  popw %fs
  popw %gs
 # popf 
  popl %edi
  popl %esi
#  popl %edx
#  popl %ecx
#  popl %eax
  popl %ebx
  popl %ebp

  ret                #Jump to the new thread's eip