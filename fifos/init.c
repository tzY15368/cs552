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

#ifndef SCHED_H
#include "sched.h"
#endif

#ifndef IDT_H
#include "idt.h"
#endif

void f0(){
  tprintf("f0\n");
  // print_esp();
  // print_eip();
  tprintf("end of f0\n");
}

void f1(){
  tprintf("f1\n");
  int a = 2-2;
  int b = 2/a;
  a = b;
  thread_yield();
  f1();
  tprintf("end of f1\n");
}

void init( multiboot* pmb ) {
 
  memory_map_t *mmap;
  unsigned int memsz = 0;		/* Memory size in MB */
  static char memstr[10];

  for (mmap = (memory_map_t *) pmb->mmap_addr;
      (unsigned long) mmap < pmb->mmap_addr + pmb->mmap_length;
      mmap = (memory_map_t *) ((unsigned long) mmap + mmap->size + 4)) {
    if (mmap->type == 1)	/* Available RAM -- see 'info multiboot' */
      memsz += mmap->length_low;
  }

  /* Convert memsz to MBs */
  memsz = (memsz >> 20) + 1;

  itoa(memstr, 'd', memsz);

  terminal_initialize();
  init_descriptor_tables();
  idt_init();

  thread_pool_init();
  ready_queue_init();
  sched_init();

  tprintf("thread pool size: %d\n", thread_pool.size);
  thread_create(f0);
  thread_create(f1);

  start_sched();
}
