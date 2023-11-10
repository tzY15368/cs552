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


// if must is true, then it will halt if no running or idle threads
static thread_ctl_blk_t* current_tcb;

thread_ctl_blk_t* get_current_tcb(bool must){
    
    // thread_pool_t* pool = &thread_pool;
    // if(pool->idle_cnt == pool->size){
    //     if(must){
    //         tprintf("get_current_tcb: no idle threads\n");
    //         halt();
    //     }
    //     return NULL;
    // }
    // for(size_t i = 0; i < pool->size; i++){
    //     if(pool->threads[i].state == RUNNING){
    //         return &pool->threads[i];
    //     }
    // }
    if(must && current_tcb == NULL){
        tprintf("get_current_tcb: no running threads\n");
        halt();
    }
    return current_tcb;
}

void set_current_tcb(thread_ctl_blk_t* tcb){
    current_tcb = tcb;
}

void sched(){
    tprintf("--------------- sched: loop ----------------\n");
    // dump_ready_queue(&ready_queue);
    thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
    // dump_ready_queue(&ready_queue);
    if(tcb == NULL){
      tprintf("sched: tcb is null\n");
      halt();
    }
    tprintf("sched: before exec: %d\n", tcb->id);
    thread_ctl_blk_t* prev_tcb = get_current_tcb(FALSE);

    tcb->state = RUNNING;
    set_current_tcb(tcb);

    if(prev_tcb == NULL){
		  context_t *dummy = (context_t *) (&dummy_stack[STACK_SIZE-1] - sizeof(context_t*));
      ctx_switch(dummy, tcb->ctx);
    } else {
      // dump_thread_pool();
      tprintf("prev tcb: %d state %d\n", prev_tcb->id, prev_tcb->state);
      if(prev_tcb->state != TERMINATED){
        prev_tcb->state = READY;
      }
      // halt();
      if(prev_tcb!=tcb){
      ctx_switch(prev_tcb->ctx, tcb->ctx);
      }
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

