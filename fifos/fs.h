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

int get_free_inode(int inode_type){
    if(ramfs->superblock.free_inodes == 0){
        return -1;
    }
    for(int i = 0; i < N_INODES; i++){
        if(ramfs->inode[i].in_use == FALSE){
            ramfs->inode[i].type = inode_type;
            ramfs->inode[i].size = 0;
            for(int j = 0; j < 10; j++){
                ramfs->inode[i].location[j] = NULL;
            }
            ramfs->inode[i].in_use = TRUE;
            ramfs->superblock.free_inodes--;
            return i;
        }
    }
    return -1;
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
    int r = dir_walk(pathname, TRUE, INODE_TYPE_REG);
    return r == -1? -1: 0;
};
int rd_mkdir(char *pathname){
    int r = dir_walk(pathname, TRUE, INODE_TYPE_DIR);
    return r == -1? -1: 0;
}

int rd_open(char *pathname, int flags);
int rd_unlink(char *pathname);
int rd_close(int fd){
    if(fd < 0) return -1;
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) return -1;
    file_descriptor->in_use = FALSE;
    file_descriptor->fd_num = -1;
    file_descriptor->offset = 0;
    file_descriptor->inode = NULL;
    return 0;
}
int rd_read(int fd, char *address, int num_bytes){
    if(fd < 0) return -1;
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) return -1;
    int r = inode_read_bytes(file_descriptor->inode, file_descriptor->offset, address, num_bytes);
    return r;
}
int rd_write(int fd, char *address, int num_bytes){
    if(fd < 0) return -1;
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) return -1;
    int r = inode_write_bytes(file_descriptor->inode, file_descriptor->offset, address, num_bytes);
    return r;
}
int rd_lseek(int fd, int offset){
    if(fd < 0) return -1;
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) return -1;
    if(offset < 0 || offset > file_descriptor->inode->size) return -1;
    file_descriptor->offset = offset;
    return 0;
}
int rd_readdir(int fd, char *address){
    if(fd < 0) return -1;
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) return -1;
    // return 1 on ok, 0 on eof, -1 on error
    if(file_descriptor->offset >= file_descriptor->inode->size) return 0;
    int r = inode_read_bytes(file_descriptor->inode, file_descriptor->offset, address, sizeof(dir_entry_t));
    if(r == -1) return -1;
    file_descriptor->offset += 1;
    return 1;
}
