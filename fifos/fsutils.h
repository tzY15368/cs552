#ifndef FSUTILS_H
#define FSUTILS_H 1
#endif

#ifndef FS_H
#include "fs.h"
#endif

#ifndef LISTQUEUE_H
#include "listqueue.h"
#endif

#ifndef MALLOC_H
#include "malloc.h"
#endif

listqueue_t* path_to_list(char* pathname){
    char pathBuf[16];
    int pathIdx = 0;
    listqueue_t* pathQueue = listqueue_init();
    for(int i=0;i<16;i++){
        if(pathname[i] == '/'){
            if(pathQueue->size!=0){
                char realPath[16] = malloc(16 * sizeof(char));
                for(int j=0;j<16;j++){
                    realPath[j] = pathBuf[j];
                }
            }
            // clear pathBuf
            for(int j=0;j<16;j++){
                pathBuf[j] = '\0';
            }
            pathIdx = 0;
        } else {
            pathBuf[pathIdx] = pathname[i];
            pathIdx++;
        }
    }
    return pathQueue;
}

void enumerate_single_indirect_blk(block_t* blk, listqueue_t* blk_list){
    if(blk == NULL){
        return;
    }
    for(int i=0; i<RAMDISK_BLK_SIZE; i+=4){
        int blk_num = (int) blk->data_byte[i];
        block_t* blk = &ramfs->blocks[blk_num];
        if(blk_num != 0){
            listqueue_put(blk_list, blk);
        }
    }
}

void enumerate_double_indirect_blk(block_t* blk, listqueue_t* blk_list){
    if(blk == NULL){
        return;
    }
    for(int i=0; i<RAMDISK_BLK_SIZE; i+=4){
        int single_indirect_blk_num = (int) blk->data_byte[i];
        block_t* single_indirect_blk = &ramfs->blocks[single_indirect_blk_num];
        enumerate_single_indirect_blk(single_indirect_blk, blk_list);
    }
}

listqueue_t* get_blk_list(inode_t* inode){
    listqueue_t* blk_list = listqueue_init();
    // direct
    for(int i=0;i<8;i++){
        if(inode->location[i] != NULL){
            listqueue_put(blk_list, inode->location[i]);
        }
    }
    // single indirect
    enumerate_single_indirect_blk(inode->location[8], blk_list);
    // double indirect
    enumerate_double_indirect_blk(inode->location[9], blk_list);
    
    return blk_list;
}

// TODO: implement
int inode_append_bytes(inode_t* inode, uint8_t* data, uint32_t size){
    return -1;
}