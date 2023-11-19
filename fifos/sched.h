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

extern void ctx_switch(context_t* *ctx_old, context_t *ctx_new);

// static uint32_t dummy_stack[STACK_SIZE];
static thread_ctl_blk_t dummy_tcb;

void noop(){
  while(1){
    tprintf("noop: sleeping 1000");
    sleep(1000);
  }
}

// if must is true, then it will halt if no running or idle threads
static thread_ctl_blk_t* current_tcb;

thread_ctl_blk_t* get_current_tcb(bool must){
    if(must && current_tcb == NULL){
        tprintf("get_current_tcb: no running threads\n");
        halt();
    }
    return current_tcb;
}

void set_current_tcb(thread_ctl_blk_t* tcb){
    current_tcb = tcb;
}

static int SCHED_CNT = 0;

void sched(){
    SCHED_CNT += 1;
    // tprintf("<-->sched: %d<-->", SCHED_CNT);
    // dump_ready_queue(&ready_queue);
    thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
    // dump_ready_queue(&ready_queue);
    if(tcb == NULL){
      tprintf("sched: tcb is null\n");
      set_current_tcb(NULL);
      halt();
    }
    // tprintf("[%d]: sched: before exec: %d\n", SCHED_CNT, tcb->id);
    thread_ctl_blk_t* prev_tcb = get_current_tcb(FALSE);

    tcb->state = RUNNING;
    set_current_tcb(tcb);

    if(prev_tcb == NULL){

      // tprintf("[%d]: %d -> %d (PREV=NULL)\n", SCHED_CNT, dummy_tcb.id, tcb->id);
      // IRQ_clear_mask(0);
      ctx_switch(&dummy_tcb.ctx, tcb->ctx);

    } else {
      // dump_thread_pool();
      // tprintf("prev tcb: %d state %d\n", prev_tcb->id, prev_tcb->state);
      if(prev_tcb->state != TERMINATED){
        prev_tcb->state = READY;
      }
      
      if(prev_tcb!=tcb){
        // IRQ_clear_mask(0);
        // tprintf("[%d]: %d -> %d\n", SCHED_CNT, prev_tcb->id, tcb->id);
        ctx_switch(&prev_tcb->ctx, tcb->ctx);
      }
    }

    // tprintf("sched: after exec: %d, prev: %d\n", tcb->id, prev_tcb->id);
    return;
    // dump_thread_pool(&thread_pool);
}

void start_sched(){
  uint32_t* dummy_stack_ptr = (uint32_t*) dummy_tcb.stack + STACK_SIZE - 1;
  *((dummy_stack_ptr)-0) = 0;

  dummy_tcb.id = -1;
  dummy_tcb.bp = (uint32_t) ((uint32_t*)dummy_stack_ptr -1);
  dummy_tcb.func = (uint32_t) noop;
  dummy_tcb.state = READY;
  context_t* stack = (context_t*) dummy_stack_ptr - sizeof(context_t);
  dummy_tcb.ctx = stack;

  dummy_tcb.ctx->eip = dummy_tcb.func;
  dummy_tcb.ctx->ebp = dummy_tcb.bp;
	dummy_tcb.ctx->ebx = 0;
	dummy_tcb.ctx->esi = 0;
	dummy_tcb.ctx->edi = 0;
	dummy_tcb.ctx->gs = 0x10;
	dummy_tcb.ctx->fs = 0x10;
	dummy_tcb.ctx->es = 0x10;
	dummy_tcb.ctx->ds = 0x10;
  dummy_tcb.esp = (uint32_t) (((uint32_t *) stack));

  while(1){
    sched();
  }
}

