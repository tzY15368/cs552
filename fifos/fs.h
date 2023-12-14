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
    root_inode->in_use = TRUE;
    root_inode->ref_cnt = 0;
    // setup bitmap for block 0 to "using"
    int blk_0 = get_free_block();
    if(blk_0 != 0){
        tprintf("fatal: blk_0 = %d\n", blk_0);
        halt();
    }

    tprintf("RD@%d=%dKB;", ramdisk, RAMDISK_SIZE / 1024);
}



int rd_creat(char *pathname){
    __asm__ __volatile__("cli");
    int r = dir_walk(pathname, TRUE, INODE_TYPE_REG);
    __asm__ __volatile__("sti");
    return r == -1? -1: 0;
};

int rd_mkdir(char *pathname){
    __asm__ __volatile__("cli");
    int r = dir_walk(pathname, TRUE, INODE_TYPE_DIR);
    __asm__ __volatile__("sti");
    return r == -1? -1: 0;
}

void dump_dir_inode_first_loc(int inode_num){
    tprintf("dumping inode %d\n", inode_num);
    inode_t* inode = &ramfs->inode[inode_num];
    block_t* dirs_blk = inode->location[1];
    int iters = inode->size / sizeof(dir_entry_t);
    for(int i=0;i<iters;i++){
        dir_entry_t* entries_ptr = (dir_entry_t*) dirs_blk->data_byte;
        dir_entry_t* dir = entries_ptr+i;
        tprintf("ent %s @ %d\n", dir->filename, dir->inode_num);
    }
    tprintf("EOF DIUMP");
}

int rd_open(char *pathname){
    __asm__ __volatile__("cli");
    int r = dir_walk(pathname, FALSE, -1);
    if(r < 0) {
        tprintf("rd_open: dir_walk failed\n");
        
        __asm__ __volatile__("sti");
        return -1;
    }
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    int new_fd = get_new_fd(&ramfs->inode[r]);
    if(new_fd == -1) {
        tprintf("rd_open: get_new_fd failed\n");
        
    __asm__ __volatile__("sti");
        return -1;
    }
    if(ramfs->inode[r].ref_cnt > 0) {
        tprintf("rd_open: inode in use\n");
        
    __asm__ __volatile__("sti");
        return -1;
    }
    ramfs->inode[r].ref_cnt++;
    cur_tcb->fds[new_fd].inode = &ramfs->inode[r];
    cur_tcb->fds[new_fd].in_use = TRUE;
    cur_tcb->fds[new_fd].fd_num = new_fd;
    cur_tcb->fds[new_fd].offset = 0;
    __asm__ __volatile__("sti");
    return new_fd;
}

int rd_unlink(char *pathname){
    __asm__ __volatile__("cli");
    if(pathname[0] != '/') return -1;
    if(pathname[1] == '\0') return -1;

    int inode_no = dir_walk(pathname, FALSE, -1);
    if(inode_no <= 0) {
        
        __asm__ __volatile__("sti");
        tprintf("not found for %s", pathname);
        return -1;
    };
    inode_t* inode = &ramfs->inode[inode_no];
    if(inode->type == INODE_TYPE_DIR && inode->size != 0){
        tprintf("dir not empty");
        __asm__ __volatile__("sti");
        return -1;
    }
    if(inode->ref_cnt != 0){
        tprintf("inode in use");
        __asm__ __volatile__("sti");
        return -1;
    }
    
    // free inode
    inode->in_use = FALSE;
    ramfs->superblock.free_inodes++;
    // free blocks
    listqueue_t* blk_list = get_blk_list(inode);
    int blk_list_sz = blk_list->size;
    for(int i=0; i<blk_list_sz; i++){
        block_t* blk = listqueue_get(blk_list);
        int blk_no = (blk - ramfs->blocks);
        __bitmap_set(ramfs->bitmap, blk_no, 0);
        ramfs->superblock.free_blocks++;
    }
    // reset inode
    inode->size = 0;
    for(int i=0; i<10; i++){
        inode->location[i] = NULL;
    }
    inode->ref_cnt = 0;

    char parentPath[16];
    char filename[16];
    int slash_cnt = 0;
    for(int i=0; i<16; i++){
        if(pathname[i] == '/') slash_cnt++;
    }
    // tprintf("rd-unlink: %s - slash-cnt:%d\n", pathname, slash_cnt);
    
    // TODO: finish this
    if(slash_cnt == 1){
        parentPath[0] = '\0';
        strcpy(pathname + 1, filename, 16);
    } else {
        int last_slash_idx = -1;
        for(int i=0; i<16; i++){
            if(pathname[i] == '/') last_slash_idx = i;
        }
        tprintf("last slash idx: %d", last_slash_idx);

        strcpy(pathname, parentPath, last_slash_idx);
        strcpy(pathname + last_slash_idx + 1, filename, 16);
    }
    __asm__ __volatile__("sti");
    return dir_inode_unlink(parentPath, filename);
}

int rd_close(int fd){
    __asm__ __volatile__("cli");
    if(fd < 0) return -1;
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    file_descriptor->inode->ref_cnt--;
    if(file_descriptor->in_use == FALSE) {
        
        __asm__ __volatile__("sti");
        return -1;
    }
    file_descriptor->in_use = FALSE;
    file_descriptor->fd_num = -1;
    file_descriptor->offset = 0;
    file_descriptor->inode = NULL;
    __asm__ __volatile__("sti");
    return 0;
}

int rd_read(int fd, char *address, int num_bytes){
    __asm__ __volatile__("cli");
    if(fd < 0){
        __asm__ __volatile__("sti");
        return -1;
    }
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) return -1;
    int r = inode_read_bytes(file_descriptor->inode, file_descriptor->offset, address, num_bytes);
    if(r == -1){
        __asm__ __volatile__("sti");
        return -1;
    }
    file_descriptor->offset = min(file_descriptor->offset + num_bytes, file_descriptor->inode->size);
    
    __asm__ __volatile__("sti");
    return r;
}

int rd_write(int fd, char *address, int num_bytes){
    __asm__ __volatile__("cli");
    if(fd < 0){
        __asm__ __volatile__("sti");
        return -1;
    }
    tprintf("n bytes: %d\n", num_bytes);
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) return -1;
    int r = inode_write_bytes(file_descriptor->inode, file_descriptor->offset, address, num_bytes);
    if(r == -1){
        __asm__ __volatile__("sti");
        return -1;
    }
    file_descriptor->offset += num_bytes;
    return r;
}
int rd_lseek(int fd, int offset){
    __asm__ __volatile__("cli");
    if(fd < 0){
        __asm__ __volatile__("sti");
        return -1;
    }
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) {
        __asm__ __volatile__("sti");
        return -1;
    }
    if(offset < 0 || offset > file_descriptor->inode->size) {
        __asm__ __volatile__("sti");
        return -1;
    }
    file_descriptor->offset = offset;
    __asm__ __volatile__("sti");
    return offset;
}
int rd_readdir(int fd, char *address){
    __asm__ __volatile__("cli");
    if(fd < 0) {
        __asm__ __volatile__("sti");
        return -1;
    }
    thread_ctl_blk_t* cur_tcb = get_current_tcb(TRUE);
    file_descriptor_t* file_descriptor = &cur_tcb->fds[fd];
    if(file_descriptor->in_use == FALSE) {
        tprintf("rd_readdir: file_descriptor->in_use == FALSE\n");
        
        __asm__ __volatile__("sti");
        return -1;
    }
    // return 1 on ok, 0 on eof, -1 on error
    if(file_descriptor->offset >= file_descriptor->inode->size) {
        
        __asm__ __volatile__("sti");
        return 0;
    }
    int r = inode_read_bytes(file_descriptor->inode, file_descriptor->offset, address, sizeof(dir_entry_t));
    if(r == -1) {
        __asm__ __volatile__("sti");
        return -1;
    }
    file_descriptor->offset += sizeof(dir_entry_t);
    __asm__ __volatile__("sti");
    return 1;
}
