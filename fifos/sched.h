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

void sched(){
    tprintf("--------------- sched: loop ----------------\n");
    tprintf("readyqueue addr: %d\n", &ready_queue);
    // dump_ready_queue(&ready_queue);
    thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
    // dump_ready_queue(&ready_queue);
    if(tcb == NULL){
      tprintf("sched: tcb is null\n");
      halt();
    }
    tprintf("sched: after exec: %d\n", tcb->id);
    dump_thread_pool(&thread_pool);
}

void start_sched(){
  // TODO: Prepare ready queue and stack
  // dump_ready_queue(&ready_queue);
  // dump_thread_pool(&thread_pool);
  while(1){
    sched();
  }
}