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
    bool did_boot;
    // stack pointer
    // instruction pointer
    char stack[STACK_SIZE];
    uint32_t esp;
    uint32_t eip;
    

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


static thread_pool_t thread_pool;
static ready_queue_t ready_queue;
static thread_ctl_blk_t sched_tcb;

void sched_init(thread_ctl_blk_t* _sched_tcb){
    _sched_tcb->id = -1;
    _sched_tcb->func = NULL;
}

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
        tcb.esp = (uint32_t)tcb.stack;
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


void thread_terminate(thread_ctl_blk_t* tcb, thread_pool_t* pool){
    // set thread to terminated, add to ready queue
    tcb->state = TERMINATED;
    thread_pool_add_idle(pool, tcb->id);
    tprintf("thread_terminate: tcb id: %d\n", tcb->id);
}

/*
eip, cs, css, ds, fs, gs, eax, ...

*/

void thread_func_wrapper(thread_ctl_blk_t* tcb){
    tprintf("thread_func_wrapper: tcb id: %d\n", tcb->id);
    tcb->func(NULL);
    // jmp back to the scheduler
    thread_terminate(tcb, &thread_pool);
    tprintf("jumping back: tcb id: %d, %d\n", tcb->id, sched_tcb.eip);
    // restore stack pointer with tcb->esp
    // __asm__("movl %0, %%esp;" : : "r" (sched_tcb.esp));
    // halt();
    // jmp back to thread exec
    __asm__("jmp *%0;" : : "r" (sched_tcb.eip));
}

int thread_create(thread_pool_t* pool, ready_queue_t* queue, void* stack, void* func){
    thread_ctl_blk_t* tcb = thread_pool_get_idle(pool);
    if(tcb == NULL){
        return -1;
    }
    // reset the tcb
    tcb->func = func;
    // tcb eip points to the function
    tcb->eip = (uint32_t)thread_func_wrapper;
    // tcb esp points to the stack    
    tcb->esp = (uint32_t)(tcb->stack + STACK_SIZE - sizeof(uint32_t));

    // push the jmp_back_addr to the stack
    // *(((uint32_t *)stack)-0) = (uint32_t) jmp_back_addr


    // push tcb to the stack
    *(((uint32_t *)tcb->esp)-0) = (uint32_t) tcb;

    // Update the esp to point to the new top of the stack
    tcb->esp = (uint32_t)(tcb->esp - sizeof(uint32_t));

    // put exit thread on top of thread stack so that user thread can return
    // *(((uint32_t *)stack)-0) = (uint32_t) exit_thread
    // stack = (void *) (((uint32_t *)stack)-1);

    ready_queue_add(queue, tcb);
    return 0;
}

void thread_exec(thread_ctl_blk_t* tcb, thread_ctl_blk_t* sched_tcb){
    // setup execution env: stack, instruction pointer
    // set stack pointer
    // pusha
    tprintf("thread_exec: tcb id: %d\n", tcb->id);
    print_esp();
    // set thread to running
    tcb->state = RUNNING;
    // back up the stack pointer
    int cur_esp = 0;
    // halt();
    // __asm__("movl %%esp, %0;" : "=r" (cur_esp));
    __asm__("pusha");
    __asm__("movl %0, %%esp;" : : "r" (tcb->esp));
    // sched_tcb->esp = cur_esp;
    unsigned int eeip;\
    __asm__("call 1f\n1: pop %0" : "=r" (eeip));\
    tprintf("before jmp: %d\n", eeip);
    sched_tcb->eip = eeip+48;                  //42

    // call thread_func_wrapper -- jmp to current tcb instruction pointer
    __asm__("jmp *%0;" : : "r" (tcb->eip));
    // no-op as placeholder
    // __asm__("nop");
    
    // print_eip();
    __asm__("popa");
    tprintf("thread_exec: after jmp\n");
    // halt();
}

void thread_yield(thread_ctl_blk_t* tcb, ready_queue_t* queue){
     // set thread to ready, add to ready queue
    tcb->state = READY;
    ready_queue_add(queue, tcb);
}


void ready_queue_init(ready_queue_t* queue){
    queue->size = 0;
    queue->queue_head = 0;
    queue->queue_tail = 0;
}

