#ifndef FSUTILS_H
#define FSUTILS_H 1
#endif

#ifndef LISTQUEUE_H
#include "listqueue.h"
#endif

#ifndef MALLOC_H
#include "malloc.h"
#endif

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef UTILS_H
#include "utils.h"
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

listqueue_t* path_to_list(char* pathname){
    char pathBuf[16];
    int pathIdx = 0;
    listqueue_t* pathQueue = listqueue_init();
    for(int i=0;i<16;i++){
        if(pathname[i] == '/'){
            if(pathBuf[0] != '\0'){
                char* realPath = malloc(16 * sizeof(char));
                for(int j=0;j<16;j++){
                    realPath[j] = pathBuf[j];
                }
                listqueue_put(pathQueue, realPath);
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
    if(pathBuf[0] != '\0'){
        char* realPath = malloc(16 * sizeof(char));
        for(int j=0;j<16;j++){
            realPath[j] = pathBuf[j];
        }
        listqueue_put(pathQueue, realPath);
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


int inode_alloc_blocks(inode_t* inode, uint32_t add_size){
    if(add_size <= 0){
        return -1;
    }
    int new_size_total = inode->size + add_size;
    // if size is smaller than current size, then we need to free blocks
    // direct
    int l0_size = 8 * RAMDISK_BLK_SIZE;
    // lv1 indirect
    int pointers_per_blk = RAMDISK_BLK_SIZE / sizeof(uint32_t);
    int l1_size = pointers_per_blk * RAMDISK_BLK_SIZE;
    // lv2 indirect
    int l2_size = pointers_per_blk * pointers_per_blk * RAMDISK_BLK_SIZE;

    int cur_blks = inode->size / RAMDISK_BLK_SIZE + inode->size % RAMDISK_BLK_SIZE == 0 ? 0 : 1;
    int new_blks = new_size_total / RAMDISK_BLK_SIZE + new_size_total % RAMDISK_BLK_SIZE == 0 ? 0 : 1;
    int blks_needed = new_blks - cur_blks;


    if(new_size_total > (l0_size + l1_size + l2_size)){
        return -1;
    }
    // l0
    if(blks_needed > 0){
        
        int new_l0_blks_max_idx = min(new_blks, 8);
        
        for(int i=cur_blks;i<new_l0_blks_max_idx;i++){
            int blk_num = get_free_block();
            if(blk_num == -1){
                return -1;
            }
            inode->location[i] = &ramfs->blocks[blk_num];
            blks_needed--;
        }
    }
    // l1
    if(blks_needed > 0){
        block_t* l1_blk = inode->location[8];
        if(l1_blk == NULL){
            int blk_num = get_free_block();
            if(blk_num == -1){
                return -1;
            }
            l1_blk = &ramfs->blocks[blk_num];
            inode->location[8] = l1_blk;
        }
        int new_l1_blks_max_idx = min(new_blks, 8 + pointers_per_blk);
        for(uint32_t i=cur_blks-8;i<new_l1_blks_max_idx;i++){
            int blk_num = get_free_block();
            if(blk_num == -1){
                return -1;
            }
            l1_blk->data_byte[i*4] = blk_num;
            blks_needed--;
        }
    }
    // l2
    if(blks_needed > 0){
        block_t* l2_blk = inode->location[9];
        if(l2_blk == NULL){
            int blk_num = get_free_block();
            if(blk_num == -1){
                return -1;
            }
            l2_blk = &ramfs->blocks[blk_num];
            inode->location[9] = l2_blk;
        }
        int new_l2_blks_max_idx = min(new_blks, 8 + pointers_per_blk + pointers_per_blk * pointers_per_blk);
        for(uint32_t i=cur_blks-8-pointers_per_blk;i<new_l2_blks_max_idx;i++){
            // first calculate which l1 block this block should be in
            int l1_blk_idx = (i - 8 - pointers_per_blk) / pointers_per_blk;
            // make sure there's a l1 block
            if(l2_blk->data_byte[l1_blk_idx*4] == NULL){
                int blk_num = get_free_block();
                if(blk_num == -1){
                    return -1;
                }
                l2_blk->data_byte[l1_blk_idx*4] = blk_num;
            }
            // get the l1 block
            int l1_blk_num = l2_blk->data_byte[l1_blk_idx*4];
            block_t* l1_blk = &ramfs->blocks[l1_blk_num];
            // calculate the index of the block in the l1 block
            int l1_blk_offset = (i - 8 - pointers_per_blk) % pointers_per_blk;
            // set the block number in the l1 block
            int blk_num = get_free_block();
            if(blk_num == -1){
                return -1;
            }
            l1_blk->data_byte[l1_blk_offset*4] = blk_num;
            blks_needed--;
        }
    }
    if(blks_needed > 0){
        return -1;
    }
    return 0;
}

// TODO: read
int inode_read_bytes(inode_t* inode, uint32_t pos, char* in_buf, uint32_t size){
    return -1;
}

// TODO: returns bytes written, if -1 then fail
int inode_write_bytes(inode_t* inode, uint32_t pos, char* out_buf, uint32_t size){
    if(pos > inode->size){
        return -1;
    }
    if(pos==-1){
        pos = inode->size;
    }
    // if pos+size > inode->size, then we need to allocate more blocks
    // TODO: implement
    return -1;
}

// returns fd, if successful will add this fd to the fd table
int get_new_fd(inode_t* inode){
    thread_ctl_blk_t* tcb = get_current_tcb(TRUE);
    for(int i=0; i < FDS_PER_THREAD; i++){
        if(tcb->fds[i].in_use == FALSE){
            tcb->fds[i].in_use = TRUE;
            tcb->fds[i].inode = inode;
            tcb->fds[i].offset = 0;
            tcb->fds[i].fd_num = i;
            return i;
        }
    }
    return -1;
}

// /abc/def/c 

// if create is false, ignore create_type, returns: -1 for not found, >0 for inode_num
int dir_walk(char* pathname, bool create, int create_type){
    listqueue_t* pathQueue = path_to_list(pathname);
    int sz = pathQueue->size;
    inode_t* cur_inode = root_inode;
    for(int i=0; i<sz; i++){
        if(i!=sz-1 && cur_inode->type != INODE_TYPE_DIR){
            return -1;
        }
        char* cur_path = listqueue_get(pathQueue);

        listqueue_t* blk_list = get_blk_list(cur_inode);
        int blk_list_sz = blk_list->size;
        int max_iter = cur_inode->size;
        bool found = FALSE;
        for(int j=0; j<blk_list_sz; j++){
            block_t* blk = listqueue_get(blk_list);
            bool shouldBreak = FALSE;
            for(int k=0; k<RAMDISK_BLK_SIZE; k+=sizeof(dir_entry_t)){
                max_iter -= 1;
                
                dir_entry_t* dir = (dir_entry_t*) &blk->data_byte[k];
                if(strcmp(dir->filename, cur_path, 16) == TRUE){
                    // found
                    if(i == sz-1){
                        // last one
                        return dir->inode_num;
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
        if(found == FALSE && i == sz-1){
            if(!create){
                return -1;
            }
            if(create_type == INODE_TYPE_DIR){
                // create new dir
                dir_entry_t* new_dir = (dir_entry_t*) malloc(sizeof(dir_entry_t));
                new_dir->inode_num = get_free_inode(INODE_TYPE_DIR);
                // setup the inode
                strcpy(cur_path, new_dir->filename, 16);
                // append new dir to cur_inode
                int r = inode_write_bytes(cur_inode, -1, (char*) new_dir, sizeof(dir_entry_t));
                if(r == -1){
                    return -1;
                }
            } else if(create_type == INODE_TYPE_REG){

            } else {
                return -1;
            }
        } else if(found == FALSE){
            return -1;
        }
    }
    return -1;
}