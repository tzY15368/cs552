// #include "multiboot.h"
#include "utils.h"
#include "thread.h"
// #include "types.h"

static thread_pool_t thread_pool;
static ready_queue_t ready_queue;

void f1(thread_ctl_blk_t* tcb){
  terminal_writestring("f1\n");
  tcb->state = TERMINATED;

}

void f2(thread_ctl_blk_t* tcb){
  terminal_writestring("f2\n");
  tcb->state = TERMINATED;
}

void start_sched(){
  // TODO: Prepare ready queue and stack
  thread_create(&thread_pool, &ready_queue, NULL, f1);
  thread_create(&thread_pool, &ready_queue, NULL, f2);
  for(int i=0;i<ready_queue.size;i++){
    tprintf("ready queue: id: %d state: %d\n", ready_queue.queue[i]->id, ready_queue.queue[i]->state);
  }
  for(int i=0;i<thread_pool.size;i++){
    tprintf("thread pool: id: %d state: %d\n", thread_pool.threads[i].id, thread_pool.threads[i].state);
  }
  sched:
    tprintf("sched: loop\n");
    thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
    if(tcb == NULL){
      tprintf("sched: tcb is null\n");
      goto end;
    }

    tprintf("thread id: %d\n", tcb->id);

    tcb->func(tcb);
    if(tcb->state != TERMINATED && tcb->state != READY){
      goto error;
    }
    if(tcb->state==TERMINATED){
      thread_pool_add_idle(&thread_pool, tcb->id);
      goto sched;
    } else {
      ready_queue_add(&ready_queue, tcb);
      goto sched;
    }
  error:
    terminal_writeln("error");
    return;
  end:
    terminal_writeln("end");
    return;
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

  tprintf("MemOS: Welcome *** System memory is: %d MB\n", memsz);

  thread_pool_init(&thread_pool);
  ready_queue_init(&ready_queue);
  tprintf("thread pool size: %d %d %d\n", thread_pool.size, 1, 2);
  start_sched();
}

