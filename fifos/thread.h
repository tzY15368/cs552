#define THREAD_H 1

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#ifndef THREADPOOL_H
#include "threadpool.h"
#endif

#ifndef READYQUEUE_H
#include "readyqueue.h"
#endif

#ifndef SCHED_H
#include "sched.h"
#endif


void thread_yield(){
    thread_ctl_blk_t* tcb = get_current_tcb(TRUE);
    tcb->state = READY;
    ready_queue_add(tcb);
    tprintf("thread yield: tid=%d\n", tcb->id);
    // dump_ready_queue();
    // halt();
    sched();
}

void thread_exit(){
    thread_ctl_blk_t* tcb = get_current_tcb(TRUE);
    tcb->state = TERMINATED;
    tprintf("thread exit: tid=%d\n", tcb->id);
    sched();
}

int thread_create(void* func){
    thread_ctl_blk_t* tcb = thread_pool_get_idle();
    if(tcb == NULL){
        tprintf("thread_create: no idle thread\n");
        return -1;
    }
    tprintf("create thread %d\n", tcb->id);
    uint32_t stack_ptr = (uint32_t) tcb->stack + STACK_SIZE - 1;

    *(((uint32_t*) stack_ptr) - 0) = (uint32_t) thread_exit;

    // tcb->bp = (uint32_t) stack_ptr - 1;
    tcb->func = (uint32_t) func;

    /* Create a fake initial context for the process  */
    uint32_t stack = (uint32_t) stack_ptr - sizeof(context_t);
    tcb->ctx = (context_t*) stack;
    tcb->ctx->ds = 0x10;
    tcb->ctx->es = 0x10;
    tcb->ctx->fs = 0x10;
    tcb->ctx->gs = 0x10;

    tcb->ctx->eip = (uint32_t) func;
    tcb->ctx->ebp = tcb->bp;
    tcb->ctx->ebx = 0;
    tcb->ctx->esi = 0;
    tcb->ctx->edi = 0;

    tcb->esp = (uint32_t) (((uint32_t *) stack));

        // ---------------

    // // TODO: reset the tcb
    // tcb->func = func;
    // // tcb eip points to the function
    // tcb->eip = (uint32_t)thread_func_wrapper;
    // // tcb esp points to the stack    
    // tcb->esp = (uint32_t)(tcb->stack + STACK_SIZE - sizeof(uint32_t));


    // // push tcb to the stack
    // *(((uint32_t *)tcb->esp)-0) = (uint32_t) tcb;

    // // Update the esp to point to the new top of the stack
    // tcb->esp = (uint32_t)(tcb->esp - sizeof(uint32_t));

    // // put exit thread on top of thread stack so that user thread can return
    // // *(((uint32_t *)stack)-0) = (uint32_t) exit_thread
    // // stack = (void *) (((uint32_t *)stack)-1);

    ready_queue_add(tcb);
    return 0;
}

// void thread_exec(thread_ctl_blk_t* tcb, thread_ctl_blk_t* sched_tcb){
//     // setup execution env: stack, instruction pointer
//     // set stack pointer
//     // pusha
//     tprintf("thread_exec: tcb id: %d\n", tcb->id);
//     print_esp();
//     // set thread to running
//     tcb->state = RUNNING;
//     // back up the stack pointer
//     // halt();
    
//     // set eax to 123 for testing
//     __asm__("movl $123, %eax");

//     __asm__("pusha");
//     __asm__("movl %0, %%esp;" : : "r" (tcb->esp));
//     // sched_tcb->esp = cur_esp;
//     unsigned int eeip;\
//     __asm__("call 1f\n1: pop %0" : "=r" (eeip));\
//      tprintf("before jmp: %d\n", eeip);
//     sched_tcb->eip = eeip+17;                  //48

//     // call thread_func_wrapper -- jmp to current tcb instruction pointer
//     __asm__("jmp *%0;" : : "r" (tcb->eip));
//     // no-op as placeholder
//     // __asm__("nop");
    
//     // print_eip();
//     // tprintf("before pop\n");
//     __asm__("popa");

//     // load eax and see if its 123
//     unsigned int eax;
//     __asm__("movl %%eax, %0" : "=r" (eax));

//     tprintf("thread_exec: after jmp %d\n", eax);
//     // dump_thread_pool(&thread_pool);
//     // halt();
// }

// void thread_yield(thread_ctl_blk_t* tcb, ready_queue_t* queue){
//      // set thread to ready, add to ready queue
//     tcb->state = READY;
//     ready_queue_add(queue, tcb);
// }



