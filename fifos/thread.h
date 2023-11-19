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

#ifndef PIC_H
#include "pic.h"
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
    // dump_ready_queue();

    // halt();
    sched();
}

void thread_preempt(){
    PIC_sendEOI();
    tprintf("thread preempt\n");
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

    tcb->bp = (uint32_t) stack_ptr - 1;
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

