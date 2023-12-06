#define USER_C 1

#ifndef UTILS_H
#include "utils.h"
#endif


void test_inode_rw(){

  inode_t testinode;
  testinode.type = INODE_TYPE_DIR;
  testinode.size = 0;
  testinode.in_use = TRUE;
  for(int i=0;i<10;i++){
    testinode.location[i] = NULL;
  }
  // int b1no = get_free_block();
  // if(b1no==-1) panic("no free block\n");
  // testinode.location[0] = &ramfs->blocks[b1no];
  // int b2no = get_free_block();
  // if(b2no==-1) panic("no free block\n");
  // testinode.location[1] = &ramfs->blocks[b2no];

  // listqueue_t* blk_list = get_blk_list(&testinode);
  // tprintf("blk_list len:%d\n", blk_list->size);

  // tprintf("blk1: %d / %d", &ramfs->blocks[b1no], listqueue_get(blk_list));
  // tprintf("blk2: %d / %d", &ramfs->blocks[b2no], listqueue_get(blk_list));

  char writeBuf[16] = "hello123";
  char readBuf[16];
  int ret = inode_write_bytes(&testinode, 0, writeBuf, 16*sizeof(char));
  if(ret != 0){
    panic("inode_write_bytes failed\n");
  }
  tprintf("write ok\n");
  // halt();
  int r = inode_read_bytes(&testinode, 0, readBuf, 16*sizeof(char));
  if(r != 0){
    panic("inode_read_bytes failed\n");
  }
  tprintf("readBuf: %s\n", readBuf);
}

void discosf1(){
  tprintf("df1\n");
  listqueue_t* lq = path_to_list("/dir1");
  tprintf("q len:%d\n", lq->size);
  int size = lq->size;
  for(int i=0;i<size;i++){
    char* buf = listqueue_get(lq);
    tprintf("/%s", buf);
  }

  int r = rd_mkdir("/dir1");
  tprintf("mkdir res:%d\n", r);

  // test bitmap getset
  // uint8_t* bitmap[2];
  // bitmap[0] = 0;
  // bitmap[1] = 0;
  
  // // int blk_num = 0;
  // // bool b = __bitmap_get(bitmap, blk_num);
  // // tprintf("b:%d\n", b);
  // tprintf("\n");
  // for(int i=0;i<16;i++){
  //   __bitmap_set(bitmap, i, TRUE);
  //   // tprintf("bitmap: %d %d\n", bitmap[0], bitmap[1]);
  //   // tprintf("bitmap: %b %b -- %d %d\n", bitmap[0], bitmap[1], bitmap[0], bitmap[1]);
  //   // __bitmap_set(bitmap, i, FALSE);
  // }
  // // bitmap should be now 16 1s
  // tprintf("bitmap: %d %d\n", bitmap[0], bitmap[1]);
  // int res = rd_mkdir("/dir1");
  // tprintf("mkdir res:%d\n", res);
  // __bitmap_set(bitmap, 0, TRUE);
  // b = __bitmap_get(bitmap, blk_num);  
  // tprintf("b2:%d\n", b);
  // test_inode_rw();
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