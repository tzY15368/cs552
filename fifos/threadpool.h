#define THREADPOOL_H 1

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

static thread_pool_t thread_pool;

// takes addr of base address to init the pool
thread_pool_t* thread_pool_init(){
    thread_pool_t* pool = &thread_pool;
    pool->size = N_THREADS;
    pool->idle_cnt = N_THREADS;
    pool->next_tid = 0;
    for(size_t i = 0; i < N_THREADS; i++){
        pool->threads[i].state = IDLE;
        pool->threads[i].tp_idx = i;

        // clear the stack
        for(size_t j = 0; j < STACK_SIZE; j++){
            pool->threads[i].stack[j] = 0;
        }
    }
    return pool;
}

thread_ctl_blk_t* thread_pool_get_idle(){
    thread_pool_t* pool = &thread_pool;
    thread_ctl_blk_t* tcb = NULL;
    if(pool->idle_cnt == 0){
        return NULL;
    }
    for(size_t i = 0; i < N_THREADS; i++){
        if(pool->threads[i].state == IDLE){
            pool->idle_cnt--;
            tcb = &pool->threads[i];
            tcb->id = pool->next_tid++;
            return tcb;
        }
    }
    return tcb;
}

thread_ctl_blk_t* get_current_tcb(){
    
    thread_pool_t* pool = &thread_pool;
    if(pool->idle_cnt == pool->size){
        return NULL;
    }
    for(size_t i = 0; i < pool->size; i++){
        if(pool->threads[i].state == RUNNING){
            return &pool->threads[i];
        }
    }
    return NULL;
}


void thread_pool_add_idle(thread_ctl_blk_t* tcb){
    
    thread_pool_t* pool = &thread_pool;
    pool->idle_cnt++;
    pool->threads[tcb->tp_idx].state = IDLE;
    for(size_t i = 0; i < STACK_SIZE; i++){
        pool->threads[tcb->tp_idx].stack[i] = 0;
    }
}

void dump_thread_pool(){
    
    thread_pool_t* pool = &thread_pool;
    tprintf("dump_thread_pool @ %d: size: %d\n", pool, pool->size);
    for(int i=0;i<N_THREADS;i++){
        tprintf("thread pool: tid: %d state: %d\n", pool->threads[i].id, pool->threads[i].state);
    }
}