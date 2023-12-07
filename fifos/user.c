#define USER_C 1

#ifndef UTILS_H
#include "utils.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#define MAX_FILES 1023
#define BLK_SZ 256		/* Block size */
#define DIRECT 8		/* Direct pointers in location attribute */
#define PTR_SZ 4		/* 32-bit [relative] addressing */
#define PTRS_PB  (BLK_SZ / PTR_SZ) /* Pointers per index block */

static char pathname[80];

static char data1[DIRECT*BLK_SZ]; /* Largest data directly accessible */
static char data2[PTRS_PB*BLK_SZ];     /* Single indirect data size */
static char data3[PTRS_PB*PTRS_PB*BLK_SZ]; /* Double indirect data size */
static char addr[PTRS_PB*PTRS_PB*BLK_SZ+1]; /* Scratchpad memory */


void test1_create(){
  for(int i=0;i<MAX_FILES+1;i++){
    sprintf(pathname, "/file%d", i);
    int fd = rd_creat(pathname);
    if(fd == -1){
      tprintf("rd_creat failed: %d\n",i);
      if(i!=MAX_FILES){
        halt();
      }
    }
    memset(pathname, 0, 80);
  }
  tprintf("rd_creat ok\n");
  for(int i=0;i<MAX_FILES;i++){
    sprintf(pathname, "/file%d", i);
    int fd = rd_unlink(pathname);
    if(fd == -1){
      tprintf("unlink failed: %d\n",i);
      halt();
    }
    memset(pathname, 0, 80);
  }
  tprintf("unlink ok\n");
}

void test3_big_file_read(int fd);

// test big file write
void test2_big_file_write(){
  int r = rd_creat("/bigfile");
  if(r == -1){
    panic("rd_creat failed\n");
  }
  int fd = rd_open("/bigfile");
  if(fd == -1){
    panic("rd_open failed\n");
  }
  int w = rd_write(fd, data1, sizeof(data1));
  if(w == -1){
    panic("rd_write failed\n");
  } else {
    tprintf("write direct ok");
  }

  w = rd_write(fd, data2, sizeof(data2));
  if(w == -1){
    panic("rd_write failed\n");
  } else {
    tprintf("write single indirect ok");
  }

  w = rd_write(fd, data3, sizeof(data3));
  if(w == -1){
    panic("rd_write failed\n");
  } else {
    tprintf("write double indirect ok");
  }

  test3_big_file_read(fd);
}

// test big file read
void test3_big_file_read(int fd){
  int r = rd_lseek(fd, 0);
  if(r == -1){
    panic("rd_lseek failed\n");
  }
  r = rd_read(fd, addr, sizeof(data1));
  if(r == -1){
    panic("rd_read failed\n");
  } else {
    tprintf("read direct ok: %s\n", addr);
  }

  r = rd_read(fd, addr, sizeof(data2));
  if(r == -1){
    panic("rd_read failed\n");
  } else {
    tprintf("read single indirect ok: %s\n", addr);
  }

  r = rd_read(fd, addr, sizeof(data3));
  if(r == -1){
    panic("rd_read failed\n");
  } else {
    tprintf("read double indirect ok: %s\n", addr);
  }

  r = rd_close(fd);
  if(r == -1){
    panic("rd_close failed\n");
  }

  r = rd_unlink("/bigfile");
  if(r == -1){
    panic("rd_unlink failed\n");
  }
}


static dir_entry_t ent;
// test dirs
void test4_dirs(){
  int r = rd_mkdir("/dir1");
  if(r == -1){
    panic("rd_mkdir1 failed\n");
  }

  r = rd_mkdir("/dir1/dir2");
  if(r == -1){
    panic("rd_mkdir2 failed\n");
  }

  r = rd_mkdir("/dir1/dir3");
  if(r == -1){
    panic("rd_mkdir3 failed\n");
  }

  const fd = rd_open("/dir1");
  int _fd = fd;
  if(fd == -1){
    panic("rd_open4 failed\n");
  }
  tprintf("--%d--", fd);
  while(1){
    tprintf("fd:%d", _fd);
    r = rd_readdir(_fd, (char*)&ent);
    if(r == -1){
      panic("rd_readdir failed\n");
    } else if (r == 0) {
      tprintf("readdir: EOF\n");
      break;
    } else {
      tprintf("readdir ok: %s @ %d\n", ent.filename, ent.inode_num);
    }
  }
  
  r = rd_close(_fd);
  if(r == -1){
    panic("rd_close failed\n");
  }
}

int fork(){
  return 1;
}

// test fork
void test5_fork(){
  int r = fork();
  if(r == -1){
    panic("fork failed\n");
  } else if(r == 0){
    tprintf("<child>\n");
    for(int i=0;i<10;i++){
      sprintf(pathname, "/file-c-%d", i);
      int ret = rd_creat(pathname);
      if(ret == -1){
        panic("rd_creat failed\n");
      }
    }
  } else {
    tprintf("<parent>\n");
    for(int i=0;i<10;i++){
      sprintf(pathname, "/file-p-%d", i);
      int ret = rd_creat(pathname);
      if(ret == -1){
        panic("rd_creat failed\n");
      }
    }
  }
}

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
  // tprintf("df1\n");
  // listqueue_t* lq = path_to_list("/dir1");
  // tprintf("q len:%d\n", lq->size);
  // int size = lq->size;
  // for(int i=0;i<size;i++){
  //   char* buf = listqueue_get(lq);
  //   tprintf("/%s", buf);
  // }
  test4_dirs();
  // int r = rd_mkdir("/dir1");
  // tprintf("mkdir res:%d\n", r);

  // r = rd_mkdir("/dir1/dir2");
  // tprintf("mkdir res2:%d\n", r);

  // r = rd_mkdir("/dir100");
  // tprintf("mkdir res3:%d\n", r);

  // int fd = rd_open("/");
  // tprintf("open res:%d\n", fd);
  // tprintf("root inode size:%d", root_inode->size);
  // dir_entry_t ent;
  // r = rd_readdir(fd, (char*)&ent);
  // tprintf("readdir res:%d -- /%s %d\n", r, ent.filename, ent.inode_num);


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