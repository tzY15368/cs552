#define SYNC_H 1
 
#ifndef MALLOC_H
#include "malloc.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#ifndef LISTQUEUE_H
#include "listqueue.h"
#endif

#ifndef THREAD_H
#include "thread.h"
#endif

// just a spinlock
typedef struct mutex {
    int locked;
} mutex_t;


void mutex_init(mutex_t* mu){
    mu->locked = 0;
}

mutex_t* mutex_new(){
    mutex_t* mu = (mutex_t*)malloc(sizeof(mutex_t));
    mutex_init(mu);
    return mu;
}

void mutex_print(mutex_t* mu){
    tprintf("mutex: %d\n", mu->locked);
}

void mutex_lock(mutex_t* mu){
    // use CAS with inline assembly to lock
    int loop = 0;
    while(1){
        if(loop > 0){
            __asm__ volatile("pause");
        }
        loop = 1;
        
        int expected = 0;
        int desired = 1;
        __asm__ volatile("lock cmpxchgl %1, %2\n\t"
                         : "=a" (expected)
                         : "r" (desired), "m" (mu->locked), "0" (expected)
                         : "memory");
        if(expected == 0){
            break;
        }
    }
}

void mutex_unlock(mutex_t* mu){
    mu->locked = 0;
}

typedef struct cond {
    mutex_t* mu;
    listqueue_t* wait_queue;
} cond_t;

cond_t* cond_init(mutex_t* mu){
    cond_t* cond = (cond_t*)malloc(sizeof(cond_t));
    // halt();
    cond->mu = mu;
    cond->wait_queue = listqueue_init();
    tprintf("cond(%d);", cond->wait_queue->id);
    return cond;
}

void cond_wait(cond_t* cond){
    mutex_unlock(cond->mu);
    thread_ctl_blk_t* tcb = get_current_tcb(TRUE);
    tcb->state = WAITING;
    listqueue_put(cond->wait_queue, tcb);
    // tprintf("cond addr: %d", cond);
    // tprintf("q id: %d", cond->wait_queue->id);
    // tprintf("thread %d will sleep: sz: %d\n", tcb->id, cond->wait_queue->size);
    sched();
    mutex_lock(cond->mu);
}

void cond_signal(cond_t* cond){
    thread_ctl_blk_t* tcb_cur = get_current_tcb(TRUE);
    // tprintf("signal on thread %d", tcb_cur->id);

    thread_ctl_blk_t* tcb = (thread_ctl_blk_t*) listqueue_get(cond->wait_queue);
    if(tcb != NULL){
        tcb->state = READY;
        ready_queue_add(tcb);
        tprintf("thread %d will resume by %d:\n", tcb->id, tcb_cur->id);
    } else {
        // tprintf("no thread to resume\n");
    }
}


