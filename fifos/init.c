// #include "multiboot.h"
#ifndef TYPES_H
#include "types.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#ifndef GDT_H
#include "gdt.h"
#endif

#ifndef THREAD_H
#include "thread.h"
#endif


void f1(){
  tprintf("f1\n");
  print_esp();
  print_eip();
  tprintf("end of f1\n");
}

void f2(){
  tprintf("f2\n");
  print_esp();
  print_eip();
  f1();
  tprintf("end of f2\n");
}

void sched(){
  while(1){
    tprintf("--------------- sched: loop ----------------\n");
    tprintf("readyqueue addr: %d\n", &ready_queue);
    dump_ready_queue(&ready_queue);
    thread_ctl_blk_t* tcb = ready_queue_get(&ready_queue);
    // dump_ready_queue(&ready_queue);
    if(tcb == NULL){
      tprintf("sched: tcb is null\n");
      return;
    }
    thread_exec(tcb, &sched_tcb);
    tprintf("sched: after exec: %d\n", tcb->id);
    dump_thread_pool(&thread_pool);
  }
}

void start_sched(){
  // TODO: Prepare ready queue and stack
  thread_create(&thread_pool, &ready_queue, NULL, f1);
  thread_create(&thread_pool, &ready_queue, NULL, f2);
  // dump_ready_queue(&ready_queue);
  // dump_thread_pool(&thread_pool);
  sched();
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

  init_descriptor_tables();

  thread_pool_init(&thread_pool);
  ready_queue_init(&ready_queue);
  sched_init(&sched_tcb);
  tprintf("thread pool size: %d\n", thread_pool.size);
  start_sched();
}

