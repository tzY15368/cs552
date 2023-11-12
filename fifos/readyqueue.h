#define READYQUEUE_H 1

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

static ready_queue_t ready_queue;


void dump_ready_queue(){
    ready_queue_t* queue = &ready_queue;
    tprintf("dump_ready_queue: size: %d, head: %d, tail: %d\n", queue->size, queue->queue_head, queue->queue_tail);
    for(int i=0;i<N_THREADS;i++){
        if(queue->queue[i] == NULL){
            tprintf("ready queue: idx: %d: NULL\n", i);
            continue;
        }
        tprintf("ready queue: idx: %d @ %d, id: %d state: %d\n", i, queue->queue[i], queue->queue[i]->id, queue->queue[i]->state);
    }
}



thread_ctl_blk_t* ready_queue_get(){
    ready_queue_t* queue = &ready_queue;
    thread_ctl_blk_t* tcb = NULL;
    if(queue->size == 0){
        return tcb;
    }
    queue->size--;
    tcb = queue->queue[queue->queue_head];
    queue->queue[queue->queue_head] = NULL;
    queue->queue_head = (queue->queue_head + 1) % N_THREADS;
    
    return tcb;
}

int ready_queue_add( thread_ctl_blk_t* tcb){
    
    ready_queue_t* queue = &ready_queue;
    if(queue->size == N_THREADS){
        return -1;
    }
    tcb->state = READY;
    queue->size++;
    queue->queue[queue->queue_tail] = tcb;
    queue->queue_tail = (queue->queue_tail + 1) % N_THREADS;

    return 0;
}


void ready_queue_init(){
    ready_queue_t* queue = &ready_queue;
    queue->size = 0;
    queue->queue_head = 0;
    queue->queue_tail = 0;
    for(int i=0;i<N_THREADS;i++){
        queue->queue[i] = NULL;
    }
}