#define FS_H 1

#ifndef FSTYPES_H
#include "fstypes.h"
#endif

#ifndef UTILS_H
#include "utils.h"
#endif

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef FSUTILS_H
#include "fsutils.h"
#endif



static partition_t* ramfs = (partition_t*)ramdisk; 				//Our partition
static inode_t* root_inode;

// TODO: test this
void __bitmap_set(uint8_t* bitmap, int block_num, bool bit_value){
    int byte_num = block_num / 8;
    int bit_num = block_num % 8;
    if(bit_value == TRUE){
        bitmap[byte_num] |= (1 << bit_num);
    }else{
        bitmap[byte_num] &= ~(1 << bit_num);
    }
}

// TODO: test this
bool __bitmap_get(uint8_t* bitmap, int block_num){
    int byte_num = block_num / 8;
    int bit_num = block_num % 8;
    return (bitmap[byte_num] >> bit_num) & 1;
}

int get_free_block(){
    if(ramfs->superblock.free_blocks == 0){
        return -1;
    }
    for(int i = 0; i < N_BLOCKS; i++){
        if(!__bitmap_get(ramfs->bitmap, i)){
            __bitmap_set(ramfs->bitmap, i, 1);
            ramfs->superblock.free_blocks--;
            return i;
        }
    }
    return -1;
}

int get_free_inode(){
    if(ramfs->superblock.free_inodes == 0){
        return -1;
    }
    for(int i = 0; i < N_INODES; i++){
        if(ramfs->inode[i].in_use == FALSE){
            ramfs->inode[i].in_use = TRUE;
            ramfs->superblock.free_inodes--;
            return i;
        }
    }
    return -1;
}

void file_init(int inode_num){
    inode_t* inode = &ramfs->inode[inode_num];
}

void ramdisk_init(){
    for(int i = 0; i < RAMDISK_SIZE; i++){
        ramdisk[i] = 0;
    }

    // first 256B (first block) is superblock
    // next 256 blocks: inode[]
    // 4 blocks for bitmap
    ramfs->superblock.free_inodes = N_INODES;
    ramfs->superblock.free_blocks = N_BLOCKS - SUPERBLOCK_BLOCKS - N_INODES - BITMAP_BLOCKS;

    // set all inode location to NULL
    for(int i = 0; i < N_INODES; i++){
        for(int j = 0; j < 10; j++){
            ramfs->inode[i].location[j] = NULL;
        }
    }

    // setup root inode
    root_inode = &ramfs->inode[0];
    root_inode->type = INODE_TYPE_DIR;
    root_inode->size = 0;
    root_inode->location[0] = &ramfs->blocks[0];
    // setup bitmap for block 0 to "using"
    int blk_0 = get_free_block();
    if(blk_0 != 0){
        tprintf("fatal: blk_0 = %d\n", blk_0);
        halt();
    }

    tprintf("RD@%d=%dKB\n", ramdisk, RAMDISK_SIZE / 1024);
}



int rd_creat(char *pathname){

};
int rd_mkdir(char *pathname){
    listqueue_t* pathQueue = path_to_list(pathname);

    tprintf("path queue size: %d\n", pathQueue->size);
    int sz = pathQueue->size;
    inode_t* cur_inode = root_inode;
    for(int i=0; i<sz; i++){
        char* cur_path = listqueue_get(pathQueue);
        
        // traverse cur_inode and see if cur_path exists
        listqueue_t* blk_list = get_blk_list(cur_inode);
        int blk_list_sz = blk_list->size;
        int max_iter = cur_inode->size;
        bool found = FALSE;
        for(int j=0; j<blk_list_sz; j++){
            block_t* blk = listqueue_get(blk_list);
            bool shouldBreak = FALSE;
            for(int k=0; k<RAMDISK_BLK_SIZE; k+=sizeof(dir_entry_t)){
                max_iter -= 1;
                
                dir_entry_t* dir = (dir_entry_t*) blk->data_byte[k];
                if(strcmp(dir->filename, cur_path, 16) == TRUE){
                    // found
                    if(i == sz-1){
                        // last one
                        return -1;
                    }
                    found = TRUE;
                    cur_inode = &ramfs->inode[dir->inode_num];
                    shouldBreak = TRUE;
                    break;
                }
                if(max_iter == 0){
                    // not found
                    shouldBreak = TRUE;
                    break;
                }
            }
            if(shouldBreak){
                break;
            }
        }
        if(found == FALSE){
            // create new dir
            dir_entry_t* new_dir = (dir_entry_t*) malloc(sizeof(dir_entry_t));
            new_dir->inode_num = get_free_inode();
            strcpy(new_dir->filename, cur_path);
            // append new dir to cur_inode
            int r = inode_append_bytes(cur_inode, new_dir, sizeof(dir_entry_t));
            if(r == -1){
                return -1;
            }
        }
    }
    return 0;
};
int rd_open(char *pathname, int flags);
int rd_close(int fd);
int rd_read(int fd, char *address, int num_bytes);
int rd_write(int fd, char *address, int num_bytes);
int rd_lseek(int fd, int offset);
int rd_unlink(char *pathname);
int rd_readdir(int fd, char *address);