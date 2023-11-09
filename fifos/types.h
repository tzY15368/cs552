#define TYPES_H 1


#define STACK_SIZE 4096 // 4kb stack
#define N_THREADS 2

#define FALSE 0
#define TRUE 1

#define NULL 0
typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef signed char sint8, s8;
typedef signed short int sint16, s16;
typedef signed long int sint32, s32;
typedef signed long long int sint64, s64;

#ifndef _SIZE_T
typedef int size_t;
#define _SIZE_T 1
#endif

typedef signed char bool;

typedef unsigned long uint;
typedef signed long sint;

#ifndef _STDINT_
#define _STDINT_
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif

typedef struct context{
	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
//	uint32_t flags;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
} context_t;

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
    size_t tp_idx;
    size_t id;
    enum thread_state state;
    uint32_t stack[STACK_SIZE];
    uint32_t bp;
    uint32_t esp;
    uint32_t func;
    context_t* ctx;

} thread_ctl_blk_t;

typedef struct thread_pool{
    size_t size;
    size_t idle_cnt;
    thread_ctl_blk_t threads[N_THREADS];
    int next_tid;
} thread_pool_t;

typedef struct ready_queue{
    size_t size;
    thread_ctl_blk_t* queue[N_THREADS];
    int queue_head;
    int queue_tail;
} ready_queue_t;