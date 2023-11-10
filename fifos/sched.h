#define SCHED_H 1

#ifndef THREADPOOL_H
#include "threadpool.h"
#endif

#ifndef READYQUEUE_H
#include "readyqueue.h"
#endif

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef THREAD_H
#include "thread.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

void sched_init(){
    
}

extern void ctx_switch(context_t *ctx_old, context_t *ctx_new);

static uint32_t dummy_stack[STACK_SIZE];

void sched(){
    tprintf("--------------- sched: loop ----------------\n");
    // dump_ready_queue(&ready_queue);
    thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
    // dump_ready_queue(&ready_queue);
    if(tcb == NULL){
      tprintf("sched: tcb is null\n");
      halt();
    }
    // tcb->state = RUNNING;
    if(tcb->id==0){
		  context_t *dummy = (context_t *) (&dummy_stack[STACK_SIZE-1] - sizeof(context_t*));
      ctx_switch(dummy, tcb->ctx);
    } else {
      thread_ctl_blk_t* prev_tcb = get_current_tcb();
      // prev_tcb->state = READY;
      ctx_switch(prev_tcb->ctx, tcb->ctx);
    }

    tprintf("sched: after exec: %d\n", tcb->id);
    return;
    // dump_thread_pool(&thread_pool);
}

void start_sched(){
  // TODO: Prepare ready queue and stack
  // dump_ready_queue(&ready_queue);
  // dump_thread_pool(&thread_pool);
  while(1){
    sched();
  }
}