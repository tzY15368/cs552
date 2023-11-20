#define SYNCHROS_H 1

#ifndef THREAD_H
#include "thread.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#ifndef SYNC_H
#include "sync.h"
#endif

#ifndef THREAD_H
#include "thread.h"
#endif

#define RINGBUF_SIZE 10
#define N_PRODUCER 2
#define N_CONSUMER 2

#define CONSUMER_ID_BASE 1000
#define PRODUCER_ID_BASE 2000

#define N_MSG 5

static int consumer_id = CONSUMER_ID_BASE;
static int producer_id = PRODUCER_ID_BASE;

typedef struct triple {
    int producer_id;
    int msg;
    int consumer_id;
} triple_t;

typedef struct ringbuf {
    triple_t buf[RINGBUF_SIZE];
    int head; 
    int tail;
    int size;
    mutex_t* mutex;
    cond_t* cond_buf_not_full;
    cond_t* cond_buf_not_empty;
} ringbuf_t;

static ringbuf_t* global_rb;


ringbuf_t* ringbuf_init(){
    ringbuf_t* rb = (ringbuf_t*) malloc(sizeof(ringbuf_t));
    rb->head = 0;
    rb->tail = 0;
    rb->size = 0;
    rb->mutex = mutex_new();
    rb->cond_buf_not_full = cond_init(rb->mutex);
    rb->cond_buf_not_empty = cond_init(rb->mutex);
    return rb;
}

void ringbuf_add(ringbuf_t* rb, int pid, int msg, int cid){
    mutex_lock(rb->mutex);
    while(rb->size == RINGBUF_SIZE){
        cond_wait(rb->cond_buf_not_full);
    }
    rb->buf[rb->tail].producer_id = pid;
    rb->buf[rb->tail].msg = msg;
    rb->buf[rb->tail].consumer_id = cid;
    rb->size++;
    tprintf("P(%d) -> C(%d): %d\n",pid, cid, msg);
    rb->tail = (rb->tail + 1) % RINGBUF_SIZE;
    cond_signal(rb->cond_buf_not_empty);
    mutex_unlock(rb->mutex);
}

void ringbuf_get(ringbuf_t* rb, int consumer_id){
    mutex_lock(rb->mutex);
    while(rb->size == 0){
        cond_wait(rb->cond_buf_not_empty);
    }
    
    // if head.consumer_id != tcb->id, yield
    while(rb->buf[rb->head].consumer_id != consumer_id){
        tprintf("C(%d) yield\n", consumer_id);
        mutex_unlock(rb->mutex);
        thread_yield();
        mutex_lock(rb->mutex);
    }

    rb->size--;
    tprintf("C(%d) <- P(%d): %d\n", consumer_id, rb->buf[rb->head].producer_id, rb->buf[rb->head].msg);
    rb->head = (rb->head + 1) % RINGBUF_SIZE;
    cond_signal(rb->cond_buf_not_full);
    mutex_unlock(rb->mutex);
}

void producer(){
    int prod_id = producer_id++;
    for(int i=0;i<N_MSG;i++){
        int msg = prod_id + i*100000;
        int cid = msg%N_CONSUMER+CONSUMER_ID_BASE;
        ringbuf_add(global_rb, prod_id, msg, cid);
        sleep(200);
    }
}

void consumer(){
    int cid = consumer_id++;
    for(int i=0;i<N_MSG;i++){
        ringbuf_get(global_rb, cid);
        sleep(500);
    }
}