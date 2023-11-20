#define MALLOC_H 1

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#define HEAP_SIZE 40960

typedef struct heap {
    uint32_t data[HEAP_SIZE];
    uint32_t* start;
    uint32_t* end;
} heap_t;

static heap_t local_heap;

void heap_init(){
    local_heap.start = &local_heap.data[0];
    local_heap.end = &local_heap.data[HEAP_SIZE];
};


// no gc whatsoever
void* malloc(uint32_t obj_size){
    if (obj_size > HEAP_SIZE){
        return 0;
    }
    if (local_heap.start + obj_size > local_heap.end){
        tprintf("Out of memory\n");
        halt();
        return NULL;
    }
    uint32_t* start = local_heap.start;
    local_heap.start += obj_size;
    return (void*) start;
};