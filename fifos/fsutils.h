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
    return (bitmap[byte_num] & (1 << bit_num)) != 0;
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
            ramfs->inode[i].ref_cnt = 0;
            ramfs->superblock.free_inodes--;
            return i;
        }
    }
    return -1;
}

listqueue_t* path_to_list(char* pathname){
    char pathBuf[16] = {'\0'};
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
        if(blk_num != NULL){
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
        } else {
            // tprintf("get_blk_ls stop on loc[%d]\n", i);
            break;
        }
    }
    // single indirect
    enumerate_single_indirect_blk(inode->location[8], blk_list);
    // double indirect
    enumerate_double_indirect_blk(inode->location[9], blk_list);
    // tprintf("[blk_list=%d] ", blk_list->size);
    return blk_list;
}


int inode_alloc_blocks(inode_t* inode, uint32_t add_size){
    if(add_size <= 0){
        return 0;
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

    int cur_blks = inode->size / RAMDISK_BLK_SIZE + (inode->size % RAMDISK_BLK_SIZE == 0 ? 0 : 1);
    int new_tot_blks = new_size_total / RAMDISK_BLK_SIZE + (new_size_total % RAMDISK_BLK_SIZE == 0 ? 0 : 1);
    int blks_needed = new_tot_blks - cur_blks;
    if(new_tot_blks > 8 + pointers_per_blk + pointers_per_blk * pointers_per_blk){
        tprintf("inode_alloc_blocks: new_tot_blks > 8 + pointers_per_blk + pointers_per_blk * pointers_per_blk\n");
        return -1;
    }

    if(new_size_total > (l0_size + l1_size + l2_size)){
        return -1;
    }
    // tprintf("inode_alloc_blocks(%d): cur_blks=%d, new_tot_blks=%d, blks_needed=%d\n", new_size_total, cur_blks, new_tot_blks, blks_needed);
    // l0
    if(blks_needed > 0){
        
        int new_l0_blks_max_idx = min(new_tot_blks, 8);
        
        for(int i=cur_blks;i<new_l0_blks_max_idx && blks_needed > 0;i++){
            int blk_num = get_free_block();
            if(blk_num == -1){
                tprintf("inode_alloc_blocks l0: get_free_block failed\n");
                return -1;
            }
            inode->location[i] = &ramfs->blocks[blk_num];
            // tprintf("alloc(%d) ", blk_num);
            blks_needed--;
        }
        // tprintf("\n");
    }
    // l1
    if(blks_needed > 0){
        tprintf("(l1 needed)");
        block_t* l1_blk = inode->location[8];
        if(l1_blk == NULL){
            int blk_num = get_free_block();
            if(blk_num == -1){
                tprintf("inode_alloc_blocks l1-1: get_free_block failed\n");
                return -1;
            }
            l1_blk = &ramfs->blocks[blk_num];
            inode->location[8] = l1_blk;
            tprintf("alloc-l1(%d) ", blk_num);
        }
        int new_l1_blks_max_idx = min(new_tot_blks, 8 + pointers_per_blk);
        tprintf("new_l1_blks_max_idx=%d\n", new_l1_blks_max_idx);
        for(uint32_t i=0;i<new_l1_blks_max_idx-8 && blks_needed > 0;i++){
            int blk_num = get_free_block();
            if(blk_num == -1){
                tprintf("inode_alloc_blocks l1-2: get_free_block failed\n");
                return -1;
            }
            // tprintf("AL1(%d)", blk_num);
            l1_blk->data_byte[i*4] = blk_num;
            blks_needed--;
        }
        tprintf("\n");
    }
    // l2
    if(blks_needed > 0){
        tprintf("(l2 needed)");
        block_t* l2_blk = inode->location[9];
        if(l2_blk == NULL){
            int blk_num = get_free_block();
            if(blk_num == -1){
                tprintf("inode_alloc_blocks l2-1: get_free_block failed\n");
                return -1;
            }
            l2_blk = &ramfs->blocks[blk_num];
            inode->location[9] = l2_blk;
            tprintf("alloc-l2-0(%d) ", blk_num);
        }
        int new_l2_blks_max_idx = new_tot_blks;
        for(uint32_t i=8+pointers_per_blk;i<new_l2_blks_max_idx && blks_needed > 0;i++){
            // first calculate which l1 block this block should be in
            int l1_blk_idx = (i - 8 - pointers_per_blk) / pointers_per_blk;
            // make sure there's a l1 block
            if(l2_blk->data_byte[l1_blk_idx*4] == NULL){
                int blk_num = get_free_block();
                if(blk_num == -1){
                    tprintf("inode_alloc_blocks l2-2: get_free_block failed\n");
                    return -1;
                }
                l2_blk->data_byte[l1_blk_idx*4] = blk_num;
                tprintf("alloc-l2-1(%d) ", blk_num);
            }
            // get the l1 block
            int l1_blk_num = l2_blk->data_byte[l1_blk_idx*4];
            block_t* l1_blk = &ramfs->blocks[l1_blk_num];
            // calculate the index of the block in the l1 block
            int l1_blk_offset = (i - 8 - pointers_per_blk) % pointers_per_blk;
            // set the block number in the l1 block
            int blk_num = get_free_block();
            if(blk_num == -1){
                tprintf("inode_alloc_blocks l2-3: get_free_block failed\n");
                return -1;
            }
            tprintf("alloc-l2-2(%d), l1-blk-idx=%d, l1-blk-offsest=%d\n", blk_num, l1_blk_idx, l1_blk_offset);
            l1_blk->data_byte[l1_blk_offset*4] = blk_num;
            blks_needed--;
        }
    }
    if(blks_needed > 0){
        tprintf("inode_alloc_blocks: blks_needed > 0\n");
        return -1;
    }
    tprintf("\n");
    return 0;
}

// is_write: TRUE for write, FALSE for read, assumes we have enough blocks for rw, and size is valid
int inode_rw_bytes(inode_t* inode, uint32_t pos, bool is_write, char* write_buf, char* read_buf, uint32_t size){
    if((is_write && write_buf==NULL) || (!is_write && read_buf==NULL)){
        tprintf("inode_rw_bytes: invalid params\n");
        return -1;
    }
    if(pos > inode->size){
        tprintf("inode_rw_bytes: pos %d > inode->size %d\n", pos, inode->size);
        return -1;
    }

    listqueue_t* blk_list = get_blk_list(inode);
    int start_blk_idx = pos / RAMDISK_BLK_SIZE;
    int start_blk_offset = pos % RAMDISK_BLK_SIZE;
    int end_blk_idx = (pos+size) / RAMDISK_BLK_SIZE + ((pos+size) % RAMDISK_BLK_SIZE == 0 ? 0 : 1) - 1;
    int end_blk_offset = (pos + size) % RAMDISK_BLK_SIZE;
    // skip initial locations...
    for(int i=0; i<start_blk_idx-1; i++){
        block_t* blk = listqueue_get(blk_list);
        if(blk == NULL){
            tprintf("inode_read_bytes: prepare: blk is null\n");
            return -1;
        }
    }
    tprintf("inode[%s]: start_idx=%d, start_off=%d, end_idx=%d, end_off=%d\n",
     is_write?"WR":"RD", start_blk_idx, start_blk_offset, end_blk_idx, end_blk_offset);

    int target_offset = 0;
    // tprintf("write buf:%s\n", write_buf);
    // rw from start_blk_idx
    for(int i=start_blk_idx;i<=end_blk_idx;i++){
        block_t* blk = listqueue_get(blk_list);
        if(blk == NULL){
            tprintf("inode_read_bytes: read: blk is null\n");
            return -1;
        }
        int blk_start_offset = (i == start_blk_idx ? start_blk_offset : 0);
        int blk_end_offset = (i == end_blk_idx ? end_blk_offset : RAMDISK_BLK_SIZE-1);
        for(int j=blk_start_offset; j<=blk_end_offset; j++){
            if(is_write){
                blk->data_byte[j] = write_buf[target_offset];
                target_offset++;
            } else {
                read_buf[target_offset++] = blk->data_byte[j];
            }
        }
    }
    if(is_write){
        inode->size = max(inode->size, pos+size);
    }
    return 0;
}

void dump_inode_loc(inode_t* inode){
    // tprintf("inode->location: ");
    // for(int i=0;i<10;i++){
    //     tprintf("%d ", inode->location[i]);
    // }
    // tprintf("\n");
}

// TODO: read
int inode_read_bytes(inode_t* inode, uint32_t pos, char* read_buf, uint32_t size){
    uint32_t actual_size = min(size, max(inode->size - pos, 0));
    tprintf("inode_read_bytes: actual_size=%d, inodesize=%d\n", actual_size, inode->size);
    return inode_rw_bytes(inode, pos, FALSE, NULL, read_buf, actual_size);
}


// TODO: returns bytes written, if -1 then fail
int inode_write_bytes(inode_t* inode, uint32_t pos, char* write_buf, uint32_t size){
    if(pos!=-1 && pos > inode->size){
        tprintf("inode_write_bytes: pos > inode->size\n");
        return -1;
    }
    if(pos==-1){
        pos = inode->size;
    }
    // tprintf("write info:%s", write_buf);
    // if pos+size > inode->size, then we need to allocate more blocks
    int r = inode_alloc_blocks(inode, pos+size-inode->size);
    if(r == -1) {panic(  "inode_write_bytes: inode_alloc_blocks failed\n");} 
    // else tprintf("alloc ok\n");
    dump_inode_loc(inode);
    int rw = inode_rw_bytes(inode, pos, TRUE, write_buf, NULL, size);
    if(rw == -1){panic(  "inode_write_bytes: inode_rw_bytes failed\n");} 
    // else tprintf("rw ok\n");

    return 0;
}

// returns fd, if successful will add this fd to the fd table
int get_new_fd(inode_t* inode){
    thread_ctl_blk_t* tcb = get_current_tcb(TRUE);
    for(int i=1; i < FDS_PER_THREAD; i++){
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

// if create is false, ignore create_type, returns: -1 for not found, >=0 for inode_num
int dir_walk(char* pathname, bool create, int create_type){
    listqueue_t* pathQueue = path_to_list(pathname);
    int sz = pathQueue->size;
    // tprintf("dir_walk: pathlist size: %d\n", sz);
    inode_t* cur_inode = root_inode;
    if(strcmp(pathname, "/", 2) == TRUE){
        // tprintf("reading on root, returning root inode");
        return 0;
    }
    for(int i=0; i<sz; i++){
        char* cur_path = listqueue_get(pathQueue);
        // (((inode_t*) cur_inode - root_inode) / sizeof(inode_t))
        // tprintf("dir_walk: inum: %d, type:%d, path: %s\n", cur_inode, cur_inode->type, cur_path);
        if(i!=sz-1 && cur_inode->type != INODE_TYPE_DIR){
            tprintf("dir_walk: not a dir\n");
            return -1;
        }

        listqueue_t* blk_list = get_blk_list(cur_inode);
        int blk_list_sz = blk_list->size;
        int max_iter = cur_inode->size;
        // tprintf("dir_walk: blk_list size on path %s : %d\n", cur_path, blk_list_sz);
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
                        // tprintf("===dir_walk: last one: %d===", dir->inode_num);
                        return dir->inode_num;
                    }
                    found = TRUE;
                    // tprintf("--nxt num: %d--", dir->inode_num);
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
        tprintf("<dir_walk: found=%d> ", found);
        if(found == FALSE && i == sz-1){
            if(!create){
                tprintf("dir_walk: not found, not creating\n");
                return -1;
            }
            if(create_type == INODE_TYPE_DIR || create_type==INODE_TYPE_REG){
                // create new dir
                dir_entry_t* new_dir = (dir_entry_t*) malloc(sizeof(dir_entry_t));
                new_dir->inode_num = get_free_inode(create_type);
                tprintf("got new inode num: %d for %s\n", new_dir->inode_num, cur_path);
                // setup the inode
                strcpy(cur_path, new_dir->filename, 16);
                // append new dir to cur_inode
                int r = inode_write_bytes(cur_inode, -1, (char*) new_dir, sizeof(dir_entry_t));
                if(r != -1) {
                    // cur_inode->size += 1;
                    tprintf("dir_walk: create dir ok on inode-%d, newsz=%d\n", cur_inode, cur_inode->size);
                }
                return r==-1? -1: new_dir->inode_num;
            } else {
                tprintf("bad create_type\n");
                return -1;
            }
        } else if(found == FALSE){
            tprintf("dir_walk: not found in dir: %s\n", cur_path);
            return -1;
        }
    }
    tprintf("bad state\n");
    return -1;
}

int dir_inode_unlink(char* parent_name, char* fname){
    // int parent_dir_inode = dir_walk(parent_name, FALSE, -1);
    // if(parent_dir_inode == -1) return -1;
    // // first pass finds the last entry and removes it from the block
    // inode_t* parent_dir = &ramfs->inode[parent_dir_inode];
    // listqueue_t* parent_dir_blk_list = get_blk_list(parent_dir);
    // int parent_dir_blk_list_sz = parent_dir_blk_list->size;
    // // move the last entry to the deleted entry
    // block_t* last_blk;
    // dir_entry_t* last_entry;
    // for(int i=0;i<parent_dir_blk_list_sz;i++){
    //     last_blk = (block_t*)listqueue_get(parent_dir_blk_list);
    // }
    // int entry_index = (parent_dir->size / sizeof(dir_entry_t)) % (sizeof(block_t) / sizeof(dir_entry_t)) - 1;
    // last_entry = (dir_entry_t*)last_blk->data_byte + entry_index * sizeof(dir_entry_t);
    // // second pass finds the deleted entry if it's not the last one, and move the last entry to the deleted entry
    // // if()
}