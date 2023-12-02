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

#ifndef PIC_H
#include "pic.h"
#endif

#ifndef IDT_H
#include "idt.h"
#endif

#ifndef SYNC_H
#include "sync.h"
#endif

#ifndef SYNCHROS_H
#include "synchros.h"
#endif

#ifndef FS_H
#include "fs.h"
#endif

static mutex_t* global_mutex;
static cond_t* global_cond;


void f1(){
  tprintf("f1\n");
  // print_esp();
  // sleep(2000);
  // print_eip();
  for(int i=0;i<5;i++){
    // mutex_lock(global_mutex);
    tprintf("<1-%d>",i*2);
    sleep(1000);
    // if(i > 1){
    //   cond_signal(global_cond);
    // }
    // thread_yield();
    // mutex_unlock(global_mutex);
  }
  // tprintf("end of f1\n");
}

void f2(){
  // print_esp();
  // print_eip();
  // mutex_lock(global_mutex);
  // cond_wait(global_cond);
  for(int i=0;i<5;i+=1){
    tprintf("<2-%d>", 1+i*2);
    sleep(1000);
    // thread_yield();
  }
  // mutex_unlock(global_mutex);
  // thread_yield();
  // f1();
  tprintf("end of f2\n");
}

void f3(){
  for(int i=0;i<3;i++){
    tprintf("<3-%d>", i);
    sleep(500);
  }
}

void f4(){
  for(int i=0;i<4;i++){
    tprintf("<4-%d>", i);
    sleep(500);
  }
}

void discosf1(){
  tprintf("df1\n");
}

void discosf2(){
  tprintf("df2\n");
}

void init( multiboot* pmb ) {
 
   memory_map_t *mmap;
   unsigned int memsz = 0;		/* Memory size in MB */

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

  terminal_initialize();
  if(memsz < 64){
    tprintf("Not enough memory: %dMB\n", memsz);
    return;
  }
  
  init_descriptor_tables();

  heap_init();

  idt_init();
  
  pic_init();
  init_pit();
  // int i = 0;
  // i = 100 / i;
  mutex_init(global_mutex);
  global_cond = cond_init(global_mutex);

  global_rb = ringbuf_init();

  thread_pool_init();
  ready_queue_init();
  sched_init();

  tprintf("TP=%d\n", thread_pool.size);
  tprintf("Preemption=%d",PREEMPTION_ON);
  ramdisk_init();

  thread_create(discosf1);
  thread_create(discosf2);
  // thread_create(producer);  
  // thread_create(producer);

  // thread_create(consumer);
  // thread_create(consumer);
  
  // thread_create(f2);
  // thread_create(f1);
  // thread_create(f3);
  // thread_create(f4);

  start_sched();
}

