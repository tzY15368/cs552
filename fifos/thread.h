#ifndef N_THREADS

#include "utils.h"
#include "types.h"

#endif

enum thread_state {
    IDLE,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

// Example state information includes copies of machine register values
// such as the stack and instruction pointers (possibly others too), and a thread ID (TID).
typedef struct thread_ctl_blk {
    size_t id;
    void* (*func)(void*);
    enum thread_state state;
} thread_ctl_blk_t;

typedef struct thread_pool{
    size_t size;
    size_t idle_cnt;
    thread_ctl_blk_t threads[N_THREADS];
} thread_pool_t;

typedef struct ready_queue{
    size_t size;
    thread_ctl_blk_t* queue[N_THREADS];
    int queue_head;
    int queue_tail;
} ready_queue_t;


// takes addr of base address to init the pool
thread_pool_t* thread_pool_init(thread_pool_t* pool){
    pool->size = N_THREADS;
    pool->idle_cnt = N_THREADS;
    terminal_writeln("thread_pool_init: ");
    for(size_t i = 0; i < N_THREADS; i++){
        thread_ctl_blk_t tcb;
        tcb.id = i;
        tcb.func = NULL;
        tcb.state = IDLE;
        pool->threads[i] = tcb;
        tprintf("thread id: %d\n", i);
    }
    return pool;
}

thread_ctl_blk_t* thread_pool_get_idle(thread_pool_t* pool){
    thread_ctl_blk_t* tcb = NULL;
    if(pool->idle_cnt == 0){
        return tcb;
    }
    for(size_t i = 0; i < pool->size; i++){
        if(pool->threads[i].state == IDLE){
            pool->idle_cnt--;
            tcb = &pool->threads[i];
            return tcb;
        }
    }
    return tcb;
}

void thread_pool_add_idle(thread_pool_t* pool, size_t tid){
    pool->idle_cnt++;
    pool->threads[tid].state = IDLE;
}

thread_ctl_blk_t* ready_queue_get(ready_queue_t* queue){
    thread_ctl_blk_t* tcb = NULL;
    if(queue->size == 0){
        return tcb;
    }
    queue->size--;
    tcb = queue->queue[queue->queue_head];
    queue->queue_head = (queue->queue_head + 1) % N_THREADS;
    
    return tcb;
}

int ready_queue_add(ready_queue_t* queue, thread_ctl_blk_t* tcb){
    if(queue->size == N_THREADS){
        return -1;
    }
    tcb->state = READY;
    queue->size++;
    queue->queue[queue->queue_tail] = tcb;
    queue->queue_tail = (queue->queue_tail + 1) % N_THREADS;

    return 0;
}

int thread_create(thread_pool_t* pool, ready_queue_t* queue, void* stack, void* func){
    thread_ctl_blk_t* tcb = thread_pool_get_idle(pool);
    if(tcb == NULL){
        return -1;
    }
    // reset the tcb
    tcb->func = func;
    ready_queue_add(queue, tcb);
    return 0;
}

void thread_yield(thread_ctl_blk_t* tcb, ready_queue_t* queue){
    // set thread to ready, add to ready queue
    tcb->state = READY;
    ready_queue_add(queue, tcb);
}

void thread_terminate(thread_ctl_blk_t* tcb, thread_pool_t* pool){
    // set thread to terminated, add to ready queue
    thread_pool_add_idle(pool, tcb->id);
}

void ready_queue_init(ready_queue_t* queue){
    queue->size = 0;
    queue->queue_head = 0;
    queue->queue_tail = 0;
}