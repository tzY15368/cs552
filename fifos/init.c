// #include "multiboot.h"
#include "utils.h"
#include "thread.h"
// #include "types.h"

static thread_pool_t thread_pool;
static ready_queue_t ready_queue;

int thread_create(void* stack, void* func){

  return 0;
}

void f1(){
  terminal_writestring("f1\n");
}

void f2(){
  terminal_writestring("f2\n");
}

void start_sched(){
  // TODO: Prepare ready queue and stack
  thread_create(&thread_pool, &ready_queue, NULL, f1);
  thread_create(&thread_pool, &ready_queue, NULL, f2);
  // thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
  // tcb->func();
  // tcb = ready_queue_get(&thread_pool.ready_queue);
  // tcb->func();
  while(1){
    ?????????????????????????????
  }
}

void init( multiboot* pmb ) {

   memory_map_t *mmap;
   unsigned int memsz = 0;		/* Memory size in MB */
   static char memstr[10];

  for (mmap = (memory_map_t *) pmb->mmap_addr;
       (unsigned long) mmap < pmb->mmap_addr + pmb->mmap_length;
       mmap = (memory_map_t *) ((unsigned long) mmap
				+ mmap->size + 4 /*sizeof (mmap->size)*/)) {
    
    if (mmap->type == 1)	/* Available RAM -- see 'info multiboot' */
      memsz += mmap->length_low;
  }

  /* Convert memsz to MBs */
  memsz = (memsz >> 20) + 1;	/* The + 1 accounts for rounding
				   errors to the nearest MB that are
				   in the machine, because some of the
				   memory is othrwise allocated to
				   multiboot data structures, the
				   kernel image, or is reserved (e.g.,
				   for the BIOS). This guarantees we
				   see the same memory output as
				   specified to QEMU.
				    */

  itoa(memstr, 'd', memsz);

  terminal_initialize();

  terminal_writestring("MemOS: Welcome *** System memory is:123123 ");
  terminal_writestring(memstr);
  terminal_writestring("MB");
  terminal_writestring("\n");
  thread_pool_init(&thread_pool);

  start_sched();
}

