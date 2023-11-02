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
    terminal_writestring("||ready queue: ");
    terminal_writeint(ready_queue.queue[i]->id);
    terminal_writestring("&&");
    terminal_writeint(ready_queue.queue[i]->state);
    terminal_writestring("\n");
  }
  terminal_writestring("===============");
  for(int i=0;i<thread_pool.size;i++){
    terminal_writestring("||thread pool: ");
    terminal_writeint(thread_pool.threads[i].id);
    terminal_writestring("&&");
    terminal_writeint(thread_pool.threads[i].state);
    terminal_writestring("\n");
  }
  sched:
    terminal_writestring("sched\n");
    thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
    if(tcb == NULL){
      goto end;
    }
    terminal_writestring("thread id: ");
    terminal_writeint(tcb->id);

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
    terminal_writestring("error\n");
    return;
  end:
    terminal_writestring("end\n");
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

  terminal_writestring("MemOS: Welcome *** System memory is:123123 ");
  terminal_writestring(memstr);
  terminal_writestring("MB");
  terminal_writestring("\n");

  thread_pool_init(&thread_pool);
  ready_queue_init(&ready_queue);
  start_sched();
}

