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

#ifndef THREADPOOL_H
#include "threadpool.h"
#endif

#ifndef READYQUEUE_H
#include "readyqueue.h"
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

  thread_pool_init();
  ready_queue_init();
  sched_init();

  tprintf("thread pool size: %d\n", thread_pool.size);
  
  thread_create(f1);
  thread_create(f2);


  start_sched();
}

