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
    // tprintf("thread before yield: tid=%d\n", tcb->id);
    // dump_ready_queue();
    // halt();
    sched();
    // tprintf("thread after yield: tid=%d\n", tcb->id);
}

void thread_exit(){
    thread_ctl_blk_t* tcb = get_current_tcb(TRUE);
    tcb->state = TERMINATED;
    tprintf("[X%d]", tcb->id);

    // halt();
    sched();
}

void thread_preempt(){
    PIC_sendEOI(0);
    thread_ctl_blk_t* tcb = get_current_tcb(TRUE);
    tcb->state = READY;
    ready_queue_add(tcb);
    // tprintf("TP(%d)", tcb->id);
    sched();
}

int thread_create(void* func){
    thread_ctl_blk_t* tcb = thread_pool_get_idle();
    if(tcb == NULL){
        tprintf("thread_create: no idle thread\n");
        return -1;
    }
    tprintf("thread(%d);", tcb->id);
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


    ready_queue_add(tcb);
    return 0;
}

