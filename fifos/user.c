#define USER_C 1

#ifndef UTILS_H
#include "utils.h"
#endif

void discosf1(){
  tprintf("df1\n");
  listqueue_t* lq = path_to_list("/dir1/dir2/dir3");
  tprintf("q len:%d\n", lq->size);
  int size = lq->size;
  for(int i=0;i<size;i++){
    char* buf = listqueue_get(lq);
    tprintf("/%s", buf);
  }
}

void discosf2(){
  tprintf("df2\n");
}

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