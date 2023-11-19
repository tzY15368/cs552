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

// just a spinlock
typedef struct mutex {
    int locked;
} mutex_t;


void mutex_init(mutex_t* mu){
    mu->locked = 0;
}

void mutex_lock(mutex_t* mu){
    // use CAS with inline assembly to lock
    __asm__ volatile(
        "spin_lock:"
        "movl $1, %%eax "
        "xchg %%eax, %0 "
        "test %%eax, %%eax"
        "jnz spin_lock \n\t"
        : "=m" (mu->locked)
        :
        : "eax"
    );
}

void mutex_unlock(mutex_t* mu){
    mu->locked = 0;
}

typedef struct cond {
    mutex_t* mu;
    listqueue_t* waiters;
} cond_t;



